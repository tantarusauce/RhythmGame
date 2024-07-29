#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include "DxLib.h"

class Fps {
    int mStartTime;//測定開始時刻
    int mCount;//カウンタ
    float mFps;//fps
    static const int N = 60;//平均をとるサンプル数
    static const int FPS = 60;//設定したFPS

public:
    Fps() {
        mStartTime = 0;
        mCount = 0;
        mFps = 0;
    }

    bool Update() {
        if (mCount == 0) {
            mStartTime = GetNowCount();
        }
        if (mCount == N) {
            int t = GetNowCount();
            mFps = 1000.f / ((t - mStartTime) / (float)N);
            mCount = 0;
            mStartTime = t;
        }
        mCount++;
        return true;
    }

    void Draw() {
        DrawFormatString(600, 0, GetColor(0, 128, 255), "%.1f", mFps);
    }

    void Wait() {
        int tookTime = GetNowCount() - mStartTime;//かかった時間
        int waitTime = mCount * 1000 / FPS - tookTime;
        if (waitTime > 0) {
            Sleep(waitTime);//待機
        }
    }
};

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
    ChangeWindowMode(TRUE), DxLib_Init(), SetDrawScreen(DX_SCREEN_BACK);
    int stricy = 0;
    int divide = 4495;
    double secTime = 0;
    int stricy1 = 60;
    int stricy2 = 40;
    int stricy3 = 10;
    double startTime = GetNowCount();
    int notes[32768]{};
    int Handle, Note, Music, Press;//データハンドル格納用変数
    Handle = LoadGraph("画像/guide.png", TRUE);//画像をロード
    Note = LoadGraph("画像/Note.png", TRUE);//画像をロード
    
    Press = LoadSoundMem("サウンド/タップ効果音.wav");

    Fps fps;
    int OFFSET;
    int MusicOFFSET = 0;
    int speed;
    int BPM;
    bool autoPlay = true;
    bool MusicPlay = false;
    
    int noteTiming[4] = {0,0,0,0};
    std::string LINE = "";
    int sumx = 0;
    std::ifstream file("Scores/夜の先へ.score");
    while (std::getline(file, LINE)){
        if (LINE.find("Music:") != std::string::npos) {
            int position = LINE.find("Music:");
            std::string MusicFile = "サウンド/" + LINE.substr(position + 6);
            Music = LoadSoundMem(MusicFile.c_str());
            continue;
        }else if (LINE.find("BPM:") != std::string::npos) {
            int position = LINE.find("BPM:");
            std::string sBPM = LINE.substr(position + 4);
            BPM = stoi(sBPM);
            continue;
        }else if (LINE.find("Speed:") != std::string::npos) {
            int position = LINE.find("Speed:");
            std::string Speed = LINE.substr(position + 6);
            speed = stoi(Speed);
            continue;
        }else if (LINE.find("Offset:") != std::string::npos) {
            int position = LINE.find("Offset:");
            std::string Offset = LINE.substr(position + 7);
            OFFSET = stoi(Offset);
            continue;
        }
        else if (LINE.find("Musicoffset:") != std::string::npos) {
            int position = LINE.find("Musicoffset:");
            std::string MusicOffset = LINE.substr(position + 12);
            MusicOFFSET = stoi(MusicOffset);
            continue;
        }else{
            //譜面データ読み込み
            int x = 0;
            while (LINE[x] != '\0') {
                if (LINE[x] == '1') {
                    notes[sumx] = 1;
                }
                else if (LINE[x] == '2') {
                    notes[sumx] = 2;
                }
                else if (LINE[x] == '3') {
                    notes[sumx] = 3;
                }
                else if (LINE[x] == '4') {
                    notes[sumx] = 4;
                }
                sumx++;
                x++;
            }
        }
    }

    while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0) {
        ClearDrawScreen();
        if (ProcessMessage() != 0)break;
        double time = GetNowCount();
        secTime = (time - startTime)/1000;
        DrawFormatString(500, 30, GetColor(0, 128, 255), "%lf", secTime);
        fps.Update();//更新
        fps.Draw();//描画

        if (secTime > MusicOFFSET && !MusicPlay) {
            MusicPlay = true;
            PlaySoundMem(Music, DX_PLAYTYPE_BACK);
        }

        DrawRotaGraph(50, 0, 0.4, 0, Handle, TRUE);
        DrawRotaGraph(150, 0, 0.4, 0, Handle, TRUE);
        DrawRotaGraph(250, 0, 0.4, 0, Handle, TRUE);
        DrawRotaGraph(350, 0, 0.4, 0, Handle, TRUE);
        switch(stricy){
            case 3:
                DrawFormatString(500, 60, GetColor(0, 128, 255), "JUST", secTime);
                break;
            case 2:
                DrawFormatString(500, 60, GetColor(0, 128, 255), "GOOD", secTime);
                break;
            case 1:
                DrawFormatString(500, 60, GetColor(0, 128, 255), "REGRETTABLE", secTime);
                break;
        }
        

        //Y=422でJUST判定
        for (int i = 0; i < sizeof(notes)/sizeof(int); i++) {
            int notePosition = secTime * speed - i * ((double)divide / (double)BPM) + OFFSET;
            if (notes[i] == 1) {
                DrawRotaGraph(50, notePosition, 0.35, 0, Note, TRUE);
            }
            if (notes[i] == 2) {
                DrawRotaGraph(150, notePosition, 0.35, 0, Note, TRUE);
            }
            if (notes[i] == 3) {
                DrawRotaGraph(250, notePosition, 0.35, 0, Note, TRUE);
            }
            if (notes[i] == 4) {
                DrawRotaGraph(350, notePosition, 0.35, 0, Note, TRUE);
            }
        }

        if (autoPlay) {
            for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                int notePosition = secTime * speed - i * (double)divide / (double)BPM + OFFSET;
                if (notePosition > 422) {
                    if (notes[i] != 0) {
                        notes[i] = 0;
                        PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    }
                }
            }
        }
        else {
            if (CheckHitKey(KEY_INPUT_D) != 0) {
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    int notePosition = secTime * speed - i * (double)divide / (double)BPM + OFFSET;
                    if (notePosition > 422 - stricy3 && notePosition < 422 + stricy3) {
                        if (notes[i] == 1) {
                            notes[i] = 0;
                            stricy = 3;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy2 && notePosition < 422 + stricy2) {
                        if (notes[i] == 1) {
                            notes[i] = 0;
                            stricy = 2;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy1 && notePosition < 422 + stricy1) {
                        if (notes[i] == 1) {
                            notes[i] = 0;
                            stricy = 1;
                        }
                    }
                }
            }
            if (CheckHitKey(KEY_INPUT_F) != 0) {
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    int notePosition = secTime * speed - i * ((double)divide / (double)BPM) + OFFSET;
                    if (notePosition > 422 - stricy3 && notePosition < 422 + stricy3) {
                        if (notes[i] == 2) {
                            notes[i] = 0;
                            stricy = 3;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy2 && notePosition < 422 + stricy2) {
                        if (notes[i] == 2) {
                            notes[i] = 0;
                            stricy = 2;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy1 && notePosition < 422 + stricy1) {
                        if (notes[i] == 2) {
                            notes[i] = 0;
                            stricy = 1;
                        }
                    }
                }
            }
            if (CheckHitKey(KEY_INPUT_J) != 0) {
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    int notePosition = secTime * speed - i * ((double)divide / (double)BPM) + OFFSET;
                    if (notePosition > 422 - stricy3 && notePosition < 422 + stricy3) {
                        if (notes[i] == 3) {
                            notes[i] = 0;
                            stricy = 3;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy2 && notePosition < 422 + stricy2) {
                        if (notes[i] == 3) {
                            notes[i] = 0;
                            stricy = 2;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy1 && notePosition < 422 + stricy1) {
                        if (notes[i] == 3) {
                            notes[i] = 0;
                            stricy = 1;
                        }
                    }
                }
            }
            if (CheckHitKey(KEY_INPUT_K) != 0) {
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    int notePosition = secTime * speed - i * ((double)divide / (double)BPM) + OFFSET;

                    if (notePosition > 422 - stricy3 && notePosition < 422 + stricy3) {
                        if (notes[i] == 4) {
                            notes[i] = 0;
                            stricy = 3;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy2 && notePosition < 422 + stricy2) {
                        if (notes[i] == 4) {
                            notes[i] = 0;
                            stricy = 2;
                            PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                        }
                    }
                    else if (notePosition > 422 - stricy1 && notePosition < 422 + stricy1) {
                        if (notes[i] == 4) {
                            notes[i] = 0;
                            stricy = 1;
                        }
                    }
                }
            }
        }
        fps.Wait();
        //while (1);
        
    }
    DxLib_End();    // DXライブラリ終了処理
    return 0;
}