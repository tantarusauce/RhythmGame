/*-------------------------+
| たんたるリズムあどべんちゃ～　|
|                          |
|      　2024 たんたるそーす　|
|コンパイルにはDXLibが        |
|                   必要です|
+--------------------------+*/

#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include "DxLib.h"
volatile int EndFlag;

int Judge(int notePosition, int notesi, int x, int stricy1, int stricy2, int stricy3) {
    int stricy = 0;
    if (notePosition > 422 - stricy3 && notePosition < 422 + stricy3) {
        if (notesi == x)stricy = 3;
    }
    else if (notePosition > 422 - stricy2 && notePosition < 422 + stricy2) {
        if (notesi == x)stricy = 2;
    }
    else if (notePosition > 422 - stricy1 && notePosition < 422 + stricy1) {
        if (notesi == x)stricy = 1;
    }
    return stricy;
}

int judgeAll(int sumx, float secTime, float OFFSET, float divide, float notesBPM[], float notesSpeed[], short notes[], int stricy1, int stricy2, int stricy3, int Press, int noteTiming[], int key) {
    int stricy = 0;
    for (int i = 0; i < sumx; i++) {
        float notePosition = (secTime + OFFSET) * 300;
        for (int j = 0; j < i; j++)notePosition -= divide / notesBPM[j];
        stricy = Judge((int)((notePosition - 422) * notesSpeed[i] + 422), notes[i], key, stricy1, stricy2, stricy3);
        if (stricy != 0) {
            notes[i] = 0;
            noteTiming[stricy]++;
            break;
        }
    }
    PlaySoundMem(Press, DX_PLAYTYPE_BACK);
    return stricy;
}

void Graph(int Handle, std::string comment[], int stricy, int noteTiming[], int sumx, double secTime, double OFFSET, float divide, float notesBPM[], float notesSpeed[], short notes[], int Note, bool playing) {
    //ノートガイド表示
    for (int i = 50; i <= 350; i += 100)DrawRotaGraph(i, 0, 0.4, 0, Handle, TRUE);
    //コメント表示
    DrawFormatString(500, 50, GetColor(0, 128, 255), comment[stricy].c_str());
    DrawFormatString(500, 90, GetColor(128, 64, 255), ("     Miss:" + std::to_string(noteTiming[0] + noteTiming[1])).c_str());
    DrawFormatString(500, 110, GetColor(128, 64, 255), ("     Nice:" + std::to_string(noteTiming[2])).c_str());
    DrawFormatString(500, 130, GetColor(128, 64, 255), ("Wonderful:" + std::to_string(noteTiming[3])).c_str());
    if (!playing)DrawFormatString(450, 200, GetColor(255, 255, 255), "Pless Space to Play");
    //ノートの表示
    for (int i = sumx; i > 0; i--) {
        float notePosition = (secTime + OFFSET) * 300;
        for (int j = 0; j < i; j++)notePosition -= divide / notesBPM[j];
        notePosition = (notePosition - 422) * notesSpeed[i] + 422;
        if (notePosition < 600 && notePosition > -30)DrawRotaGraph(50 + (notes[i] - 1) * 100, (int)notePosition, 0.35, 0, Note, TRUE);
    }
}

void titleGraphic(bool a) {
    if(a)DrawFormatString(250, 400, GetColor(255, 255, 255), ("Pless Space to Start"));
}

/*
void pauseGraphic(int pauseSelect, int Continue, int Quit) {
    DrawRotaGraph(300, 150, 0.33, 0, Continue, TRUE);
    DrawRotaGraph(300, 250, 0.33, 0, Quit, TRUE);
    unsigned int Cr = GetColor(255, 255, 255);
    DrawBoxAA(50.0f, 120.0f, 580.0f, 190.0f, Cr, FALSE);  
}*/

DWORD WINAPI MainThread(LPVOID)
{
    while (true) {
        bool titleLoop = true;
        bool titleFlip = false;
        int titleint = 0;
        //タイトル画面
        while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && titleLoop) {
            //DrawFormatString(0, 0, GetColor(255, 255, 255), std::to_string(titleint).c_str());
            titleGraphic(titleFlip);
            (titleFlip) ? titleint++ : titleint--;
            if (titleFlip == false && titleint <= 0) {
                titleFlip = true;
            }
            if (titleFlip == true && titleint >= 50) {
                titleFlip = false;
            }
            if (CheckHitKey(KEY_INPUT_SPACE) != 0) {
                titleLoop = false;
            }
        }

        //ゲームローディング
        // 
        //変数・定数の設定
        bool autoPlay = true;
        int stricy1 = 60, stricy2 = 40, stricy3 = 10, stricy = 0, divide = 4500, volume = 50, sumx = 0;
        int pauseSelect = 0, MusicTime = 0;
        int Handle, Note, Music, Press, Quit, Continue;//データハンドル格納用変数
        bool releaseKey1 = true, releaseKey2 = true, releaseKey3 = true, releaseKey4 = true, pause = false;
        bool playingGame = true;
        double startTime = 0, secTime = 0, pauseStartTime = 0, pauseEndTime = 0;
        short notes[4096]{};
        float notesSpeed[4096]{}, notesBPM[4096]{};
        std::string comment[] = { "", "Miss", "Nice", "Wonderful" };
        Handle = LoadGraph("画像/guide.png", TRUE);
        Note = LoadGraph("画像/Note.png", TRUE);
        Quit = LoadGraph("画像/ゲームをやめる.png");
        Continue = LoadGraph("画像/ゲームを続ける.png");
        Press = LoadSoundMem("サウンド/タップ効果音.wav");
        double OFFSET = 0;
        int MusicOFFSET = 0;
        float speed, BPM;
        bool MusicPlay = false, playing = false;
        int noteTiming[4] = { 0,0,0,0 };
        std::string LINE = "";

        //読み込むファイル名
        std::ifstream file("Scores/夜の先へ.score");

        //ファイル読み込み
        while (std::getline(file, LINE)) {
            if (LINE.find("Music:") != std::string::npos) {
                //キーワード「Music:」後のキーワードの読み取り
                size_t position = LINE.find("Music:");
                std::string MusicFile = "サウンド/Music/" + LINE.substr(position + 6);
                Music = LoadSoundMem(MusicFile.c_str());
                continue;
            }
            else if (LINE.find("BPM:") != std::string::npos) {
                //キーワード「BPM:」後の数値の読み取り
                size_t position = LINE.find("BPM:");
                std::string sBPM = LINE.substr(position + 4);
                BPM = stof(sBPM);
                continue;
            }
            else if (LINE.find("Speed:") != std::string::npos) {
                //キーワード「Speed:」後の数値の読み取り
                size_t position = LINE.find("Speed:");
                std::string Speed = LINE.substr(position + 6);
                speed = stof(Speed);
                continue;
            }
            else if (LINE.find("Offset:") != std::string::npos) {
                //キーワード「Offset:」後の数値の読み取り
                size_t position = LINE.find("Offset:");
                std::string Offset = LINE.substr(position + 7);
                OFFSET = stod(Offset);
                continue;
            }
            else if (LINE.find("Musicoffset:") != std::string::npos) {
                //キーワード「Musicoffset:」後の数値の読み取り
                size_t position = LINE.find("Musicoffset:");
                std::string MusicOffset = LINE.substr(position + 12);
                MusicOFFSET = stoi(MusicOffset);
                continue;
            }
            else {
                //数値の読み取り
                int x = 0;
                while (LINE[x] != '\0') {
                    notes[sumx] = LINE[x] - '0';
                    notesBPM[sumx] = BPM;
                    notesSpeed[sumx] = speed;
                    sumx++;
                    x++;
                }
            }
        }

        //ゲームループ関数
        while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && playingGame) {


            if (pause == false) {
                if (ProcessMessage() != 0)break;//エラー時に終了
                double time = GetNowCount();//時間を計測
                if (playing)secTime = (time - startTime) / 1000;//現在のゲーム経過時間を計算

                // 音量の設定
                ChangeVolumeSoundMem(255 * volume / 100, Music);
                ChangeVolumeSoundMem(255 * volume / 100, Press);
                //サウンドの再生
                if (secTime > MusicOFFSET && !MusicPlay) {
                    MusicPlay = true;
                    PlaySoundMem(Music, DX_PLAYTYPE_BACK);
                }
                else if (secTime > MusicOFFSET) {
                    //再生が終わるとループを抜ける
                    if (CheckSoundMem(Music) == 0) {
                        playingGame = false;

                    }
                }
                DrawFormatString(500, 90, GetColor(128, 64, 255), ("     Miss:" + std::to_string(noteTiming[0] + noteTiming[1])).c_str());
                //スペースキーで開始
                if (CheckHitKey(KEY_INPUT_SPACE) && !MusicPlay) { startTime = GetNowCount(); playing = true; }
                //描画消去
                ClearDrawScreen();
                //描画処理
                Graph(Handle, comment, stricy, noteTiming, sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, Note, playing);
                if (autoPlay) {
                    //オートプレイオン時の処理
                    for (int i = 0; i < sumx; i++) {
                        float notePosition = (secTime + OFFSET) * 300;
                        for (int j = 0; j < i; j++)notePosition -= divide / notesBPM[j];
                        if ((notePosition - 422) * notesSpeed[i] > 0) {
                            if (notes[i] != 0) {
                                notes[i] = 0;
                                noteTiming[3]++;
                                PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                            }
                        }
                    }
                }
                else {
                    //オートプレイオフ時の処理
                    //Dキーが押された時の判定
                    if (CheckHitKey(KEY_INPUT_D) != 0 && releaseKey1) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 1);
                        releaseKey1 = false;
                    }
                    //Fキーが押された時の判定
                    if (CheckHitKey(KEY_INPUT_F) != 0 && releaseKey2) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 2);
                        releaseKey2 = false;
                    }
                    //Jキーが押された時の判定
                    if (CheckHitKey(KEY_INPUT_J) != 0 && releaseKey3) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 3);
                        releaseKey3 = false;
                    }
                    //Kキーが押された時の判定
                    if (CheckHitKey(KEY_INPUT_K) != 0 && releaseKey4) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 4);
                        releaseKey4 = false;
                    }
                    releaseKey1 = (CheckHitKey(KEY_INPUT_D) == 0);//Dキーが離された時のフラグ
                    releaseKey2 = (CheckHitKey(KEY_INPUT_F) == 0);//Fキーが離された時のフラグ
                    releaseKey3 = (CheckHitKey(KEY_INPUT_J) == 0);//Jキーが離された時のフラグ
                    releaseKey4 = (CheckHitKey(KEY_INPUT_K) == 0);//Kキーが離された時のフラグ
                }

                /*
                if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
                    //ESCで中断処理
                    pause = true;
                    MusicTime = GetCurrentPositionSoundMem(Music);
                    StopSoundMem(Music);
                    pauseStartTime = GetNowCount();
                }
                */
            }
            else {
                /*
                Graph(Handle, comment, stricy, noteTiming, sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, Note, playing);
                pauseGraphic(pauseSelect, Continue, Quit);
                //ポーズ中処理
                if (CheckHitKey(KEY_INPUT_SPACE) != 0) {
                    if (pauseSelect == 0) {
                        //ポーズセレクト０番（ゲームに戻る）がセレクトされた時
                        pause = false;
                        pauseEndTime = GetNowCount();
                        startTime += pauseEndTime - pauseStartTime + 21;
                        if (playing) {
                            SetCurrentPositionSoundMem(MusicTime, Music);
                            PlaySoundMem(Music, DX_PLAYTYPE_BACK, FALSE);
                        }
                    }
                }
            */
            }
        }
    }
    return 0;
}

//メイン関数
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
    ChangeWindowMode(TRUE), SetAlwaysRunFlag(TRUE), DxLib_Init(), SetDrawScreen(DX_SCREEN_BACK);
    HANDLE hand;
    DWORD id;
    hand = CreateThread(NULL, 0, MainThread, 0, 0, &id);
    while (ProcessMessage() == 0)Sleep(10);
    EndFlag = 1;
    while (EndFlag == 0)Sleep(10);
    ClearDrawScreen();
    DrawFormatString(500, 130, GetColor(128, 64, 255), ("Finish Game"));
    DxLib_End();
    return 0;
}