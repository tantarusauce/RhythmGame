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

DWORD WINAPI MainThread(LPVOID)
{
    int stricy1 = 60, stricy2 = 40, stricy3 = 10, stricy = 0, divide = 4500, volume = 20, sumx = 0;
    int Handle, Note, Music, Press;//�f�[�^�n���h���i�[�p�ϐ�
    bool releaseKey1 = true, releaseKey2 = true, releaseKey3 = true, releaseKey4 = true;
    double startTime = GetNowCount(), secTime = 0;;
    short notes[4096]{};
    float notesSpeed[4096]{}, notesBPM[4096]{};
    std::string comment[] = { "", "Miss", "Nice", "Wonderful" };
    Handle = LoadGraph("�摜/guide.png", TRUE);
    Note = LoadGraph("�摜/Note.png", TRUE);
    Press = LoadSoundMem("�T�E���h/�^�b�v���ʉ�.wav");
    double OFFSET = 0;
    int MusicOFFSET = 0;
    float speed, BPM;
    bool autoPlay = true;
    bool MusicPlay = false;
    int noteTiming[4] = { 0,0,0,0 };
    std::string LINE = "";
    std::ifstream file("Scores/��̐��.score");
    while (std::getline(file, LINE)) {
        if (LINE.find("Music:") != std::string::npos) {
            size_t position = LINE.find("Music:");
            std::string MusicFile = "�T�E���h/" + LINE.substr(position + 6);
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
        // ���ʂ̐ݒ�
        ChangeVolumeSoundMem(255 * volume / 100, Music);
        ChangeVolumeSoundMem(255 * volume / 100, Press);
        if (secTime > MusicOFFSET && !MusicPlay) {
            MusicPlay = true;
            PlaySoundMem(Music, DX_PLAYTYPE_BACK);
        }
        if (MusicPlay)if (CheckSoundFile() != 0)break;//�~�߂悤�Ƃ��Ă���
        for (int i = 50; i <= 350; i += 100)DrawRotaGraph(i, 0, 0.4, 0, Handle, TRUE);
        DrawFormatString(500, 50, GetColor(0, 128, 255), comment[stricy].c_str());
        DrawFormatString(500, 90, GetColor(128, 64, 255), ( "     Miss:" + std::to_string(noteTiming[0]+noteTiming[1])).c_str());
        DrawFormatString(500, 110, GetColor(128, 64, 255), ("     Nice:" + std::to_string(noteTiming[2])).c_str());
        DrawFormatString(500, 130, GetColor(128, 64, 255), ("Wonderful:" + std::to_string(noteTiming[3])).c_str());
        for (int i = sumx; i > 0 ; i--) {
            double notePosition = (secTime + OFFSET) * 300;
            for (int j = 0; j < i; j++)notePosition -= divide / notesBPM[j];
            notePosition = (notePosition - 422) * notesSpeed[i] + 422;
                if (notePosition < 600 && notePosition > -30)DrawRotaGraph(50 + (notes[i] - 1) * 100, (int)notePosition, 0.35, 0, Note, TRUE);
        }
        if (autoPlay) {
            for (int i = 0; i < sumx; i++) {
                float notePosition = (secTime + OFFSET) * 300;
                for (int j = 0; j < i; j++)notePosition -= divide / notesBPM[j];
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
                stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 1);
                releaseKey1 = false;
            }
            if (CheckHitKey(KEY_INPUT_F) != 0 && releaseKey2) {
                stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 2);
                releaseKey2 = false;
            }
            if (CheckHitKey(KEY_INPUT_J) != 0 && releaseKey3) {
                stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 3);
                releaseKey3 = false;
            }
            if (CheckHitKey(KEY_INPUT_K) != 0 && releaseKey4) {
                stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 4);
                releaseKey4 = false;
            }
            releaseKey1 = (CheckHitKey(KEY_INPUT_D) == 0);
            releaseKey2 = (CheckHitKey(KEY_INPUT_F) == 0);
            releaseKey3 = (CheckHitKey(KEY_INPUT_J) == 0);
            releaseKey4 = (CheckHitKey(KEY_INPUT_K) == 0);
        }
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
    ChangeWindowMode(TRUE), SetAlwaysRunFlag(TRUE), DxLib_Init(), SetDrawScreen(DX_SCREEN_BACK);
    HANDLE hand;
    DWORD id;
    hand = CreateThread(NULL, 0, MainThread, 0, 0, &id);
    while (ProcessMessage() == 0)Sleep(10);
    EndFlag = 1;
    while (EndFlag == 0)Sleep(10);
    DxLib_End();
    return 0;
}