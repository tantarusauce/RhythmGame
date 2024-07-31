#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include "DxLib.h"
#include "FPS.cpp"

volatile int EndFlag;

int Judge(int notePosition, int notesi, int x, int stricy1, int stricy2, int stricy3) {
    int stricy = 0;
    if (notePosition > 422 - stricy3 && notePosition < 422 + stricy3) {
        if (notesi == x) {
            notesi = 0;
            stricy = 3;
        }
    }
    else if (notePosition > 422 - stricy2 && notePosition < 422 + stricy2) {
        if (notesi == x) {
            notesi = 0;
            stricy = 2;
        }
    }
    else if (notePosition > 422 - stricy1 && notePosition < 422 + stricy1) {
        if (notesi == x) {
            notesi = 0;
            stricy = 1;
        }
    }
    return stricy;
}

DWORD WINAPI MainThread(LPVOID)
{
    int stricy1 = 60, stricy2 = 40, stricy3 = 10, stricy = 0, divide = 4500, volume = 50;
    int Handle, Note, Music, Press;//データハンドル格納用変数
    bool releaseKey1 = true, releaseKey2 = true, releaseKey3 = true, releaseKey4 = true;
    double startTime = GetNowCount(), secTime = 0;;
    short notes[4096]{};
    float notesSpeed[4096]{}, notesBPM[4096]{};
    std::string comment[] = { "", "Miss", "Nice", "Wonderful" };

    Handle = LoadGraph("画像/guide.png", TRUE);
    Note = LoadGraph("画像/Note.png", TRUE);
    Press = LoadSoundMem("サウンド/タップ効果音.wav");

    Fps fps;
    double OFFSET = 0;
    int MusicOFFSET = 0;
    float speed, BPM;
    bool autoPlay = true;
    bool MusicPlay = false;

    int noteTiming[4] = { 0,0,0,0 };
    std::string LINE = "";
    int sumx = 0;
    std::ifstream file("Scores/夜の先へ.score");
    while (std::getline(file, LINE)) {
        if (LINE.find("Music:") != std::string::npos) {
            size_t position = LINE.find("Music:");
            std::string MusicFile = "サウンド/" + LINE.substr(position + 6);
            Music = LoadSoundMem(MusicFile.c_str());
            continue;
        }
        else if (LINE.find("BPM:") != std::string::npos) {
            size_t position = LINE.find("BPM:");
            std::string sBPM = LINE.substr(position + 4);
            BPM = stof(sBPM);
            continue;
        }
        else if (LINE.find("Speed:") != std::string::npos) {
            size_t position = LINE.find("Speed:");
            std::string Speed = LINE.substr(position + 6);
            speed = stof(Speed);
            continue;
        }
        else if (LINE.find("Offset:") != std::string::npos) {
            size_t position = LINE.find("Offset:");
            std::string Offset = LINE.substr(position + 7);
            OFFSET = stod(Offset);
            continue;
        }
        else if (LINE.find("Musicoffset:") != std::string::npos) {
            size_t position = LINE.find("Musicoffset:");
            std::string MusicOffset = LINE.substr(position + 12);
            MusicOFFSET = stoi(MusicOffset);
            continue;
        }
        else {
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

    while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 ) {
        ClearDrawScreen();
        if (ProcessMessage() != 0)break;
        double time = GetNowCount();
        secTime = (time - startTime) / 1000;
        fps.Update();
        fps.Draw();
        // 音量の設定
        ChangeVolumeSoundMem(255 * volume / 100, Music);
        if (secTime > MusicOFFSET && !MusicPlay) {
            MusicPlay = true;
            PlaySoundMem(Music, DX_PLAYTYPE_BACK);
        }
        if (MusicPlay) {
            if (CheckSoundFile() != 0) {
                break;
            }
        }
        DrawRotaGraph(50, 0, 0.4, 0, Handle, TRUE);
        DrawRotaGraph(150, 0, 0.4, 0, Handle, TRUE);
        DrawRotaGraph(250, 0, 0.4, 0, Handle, TRUE);
        DrawRotaGraph(350, 0, 0.4, 0, Handle, TRUE);
        DrawFormatString(500, 60, GetColor(0, 128, 255), comment[stricy].c_str(), secTime);
        
        for (int i = sumx; i > 0 ; i--) {
            double notePosition = (secTime + OFFSET) * 300;
            for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
            notePosition = (notePosition - 422) * notesSpeed[i] + 422;
                if (notePosition < 600 && notePosition > -30)DrawRotaGraph(50 + (notes[i] - 1) * 100, (int)notePosition, 0.35, 0, Note, TRUE);
        }

        if (autoPlay) {
            for (int i = 0; i < sumx; i++) {
                double notePosition = (secTime + OFFSET) * 300;
                for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                if ((notePosition - 422) * notesSpeed[i] > 0) {
                    if (notes[i] != 0) {
                        notes[i] = 0;
                        PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    }
                }
            }
        }
        else {
            if (CheckHitKey(KEY_INPUT_D) != 0 && releaseKey1) {
                for (int i = 0; i < sumx; i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    stricy = Judge((int)((notePosition - 422) * notesSpeed[i] + 422), notes[i], 1, stricy1, stricy2, stricy3);
                    if (stricy != 0) {
                        notes[i] = 0;
                        break;
                    }
                }
                PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                releaseKey1 = false;
            }
            else {
                if (CheckHitKey(KEY_INPUT_D) == 0)releaseKey1 = true;
            }
            if (CheckHitKey(KEY_INPUT_F) != 0 && releaseKey2) {
                for (int i = 0; i < sumx; i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    stricy = Judge((int)((notePosition - 422) * notesSpeed[i] + 422), notes[i], 2, stricy1, stricy2, stricy3);
                    if (stricy != 0) {
                        notes[i] = 0;
                        break;
                    }
                }
                PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                releaseKey2 = false;
            }
            else {
                if (CheckHitKey(KEY_INPUT_F) == 0)releaseKey2 = true;
            }
            if (CheckHitKey(KEY_INPUT_J) != 0 && releaseKey3) {
                for (int i = 0; i < sumx; i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    stricy = Judge((int)((notePosition - 422) * notesSpeed[i] + 422), notes[i], 3, stricy1, stricy2, stricy3);
                    if (stricy != 0) {
                        notes[i] = 0;
                        break;
                    }
                }
                PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                releaseKey3 = false;
            }
            else {
                if (CheckHitKey(KEY_INPUT_J) == 0)releaseKey3 = true;
            }
            if (CheckHitKey(KEY_INPUT_K) != 0 && releaseKey4) {
                for (int i = 0; i < sumx; i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    stricy = Judge((int)((notePosition - 422) * notesSpeed[i] + 422), notes[i], 4, stricy1, stricy2, stricy3);
                    if (stricy != 0) {
                        notes[i] = 0;
                        break;
                    }
                }
                PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                releaseKey4 = false;
            }
            else {
                if(CheckHitKey(KEY_INPUT_K) == 0)releaseKey4 = true;
            }
        }
        fps.Wait();
        //while (1);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
    ChangeWindowMode(TRUE);
    SetAlwaysRunFlag(true);
    DxLib_Init();
    SetDrawScreen(DX_SCREEN_BACK);
    HANDLE hand;
    DWORD id;
    hand = CreateThread(NULL, 0, MainThread, 0, 0, &id);
    while (ProcessMessage() == 0)
    {
        Sleep(10);
    }
    EndFlag = 1;
    while (EndFlag == 0)
    {
        Sleep(10);
    }
    DxLib_End();
    return 0;
}