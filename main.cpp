#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include "DxLib.h"

volatile int EndFlag;

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

// ProcessMessage 以外の処理を行うスレッド
DWORD WINAPI MainThread(LPVOID)
{
    int stricy = 0;
    int divide = 4500;
    double secTime = 0;
    int stricy1 = 60;
    int stricy2 = 40;
    int stricy3 = 10;
    bool releaseKey1 = true;
    bool releaseKey2 = true;
    bool releaseKey3 = true;
    bool releaseKey4 = true;
    double startTime = GetNowCount();
    int notes[1028]{};
    double notesSpeed[1028]{};
    int notesBPM[1028]{};
    int Handle, Note, Music, Press;//データハンドル格納用変数
    Handle = LoadGraph("画像/guide.png", TRUE);//画像をロード
    Note = LoadGraph("画像/Note.png", TRUE);//画像をロード
    Press = LoadSoundMem("サウンド/タップ効果音.wav");

    Fps fps;
    double OFFSET;
    int MusicOFFSET = 0;
    double speed;
    double BPM;
    bool autoPlay = false;
    bool MusicPlay = false;

    int noteTiming[4] = { 0,0,0,0 };
    std::string LINE = "";
    int sumx = 0;
    std::ifstream file("Scores/夜の先へ.score");
    while (std::getline(file, LINE)) {
        if (LINE.find("Music:") != std::string::npos) {
            int position = LINE.find("Music:");
            std::string MusicFile = "サウンド/" + LINE.substr(position + 6);
            Music = LoadSoundMem(MusicFile.c_str());
            continue;
        }
        else if (LINE.find("BPM:") != std::string::npos) {
            int position = LINE.find("BPM:");
            std::string sBPM = LINE.substr(position + 4);
            BPM = stod(sBPM);
            continue;
        }
        else if (LINE.find("Speed:") != std::string::npos) {
            int position = LINE.find("Speed:");
            std::string Speed = LINE.substr(position + 6);
            speed = stod(Speed);
            continue;
        }
        else if (LINE.find("Offset:") != std::string::npos) {
            int position = LINE.find("Offset:");
            std::string Offset = LINE.substr(position + 7);
            OFFSET = stod(Offset);
            continue;
        }
        else if (LINE.find("Musicoffset:") != std::string::npos) {
            int position = LINE.find("Musicoffset:");
            std::string MusicOffset = LINE.substr(position + 12);
            MusicOFFSET = stoi(MusicOffset);
            continue;
        }
        else {
            //譜面データ読み込み
            int x = 0;
            while (LINE[x] != '\0') {
                if (LINE[x] == '1') {
                    notes[sumx] = 1;
                    notesBPM[sumx] = BPM;
                    notesSpeed[sumx] = speed;
                }
                else if (LINE[x] == '2') {
                    notes[sumx] = 2;
                    notesBPM[sumx] = BPM;
                    notesSpeed[sumx] = speed;
                }
                else if (LINE[x] == '3') {
                    notes[sumx] = 3;
                    notesBPM[sumx] = BPM;
                    notesSpeed[sumx] = speed;
                }
                else if (LINE[x] == '4') {
                    notes[sumx] = 4;
                    notesBPM[sumx] = BPM;
                    notesSpeed[sumx] = speed;
                }
                else if (LINE[x] == '0') {
                    notes[sumx] = 0;
                    notesBPM[sumx] = BPM;
                    notesSpeed[sumx] = speed;
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
        secTime = (time - startTime) / 1000;
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
        switch (stricy) {
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
        for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
            double notePosition = (secTime + OFFSET) * 300;
            for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                notePosition -= 422;
                notePosition *= notesSpeed[i];
                notePosition += 422;
            
            switch (notes[i]) {
            case 1:
                DrawRotaGraph(50, notePosition, 0.35, 0, Note, TRUE);
                break;
            case 2:
                DrawRotaGraph(150, notePosition, 0.35, 0, Note, TRUE);
                break;
            case 3:
                DrawRotaGraph(250, notePosition, 0.35, 0, Note, TRUE);
                break;
            case 4:
                DrawRotaGraph(350, notePosition, 0.35, 0, Note, TRUE);
                break;
            }
        }

        if (autoPlay) {
            for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                double notePosition = (secTime + OFFSET) * 300;
                for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                notePosition -= 422;
                notePosition *= notesSpeed[i];
                notePosition += 422;
                if (notePosition > 422) {
                    if (notes[i] != 0) {
                        notes[i] = 0;
                        PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    }
                }
            }
        }
        else {
            if (CheckHitKey(KEY_INPUT_D) != 0 && releaseKey1) {
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    notePosition -= 422;
                    notePosition *= notesSpeed[i];
                    notePosition += 422;
                    
                    stricy = Judge(notePosition, notes[i], 1, stricy1, stricy2, stricy3);
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
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    notePosition -= 422;
                    notePosition *= notesSpeed[i];
                    notePosition += 422;
                    stricy = Judge(notePosition, notes[i], 2, stricy1, stricy2, stricy3);
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
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    notePosition -= 422;
                    notePosition *= notesSpeed[i];
                    notePosition += 422;
                    PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    stricy = Judge(notePosition, notes[i], 3, stricy1, stricy2, stricy3);
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
                for (int i = 0; i < sizeof(notes) / sizeof(int); i++) {
                    double notePosition = (secTime + OFFSET) * 300;
                    for (int j = 0; j < i; j++)notePosition -= (double)divide / (double)notesBPM[j];
                    notePosition -= 422;
                    notePosition *= notesSpeed[i];
                    notePosition += 422;
                    PlaySoundMem(Press, DX_PLAYTYPE_BACK);
                    stricy = Judge(notePosition, notes[i], 4, stricy1, stricy2, stricy3);
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