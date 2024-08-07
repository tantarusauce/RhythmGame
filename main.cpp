/*-------------------------+
| ���񂽂郊�Y�����ǂׂ񂿂�`�@|
|                          |
|      �@2024 ���񂽂邻�[���@|
|�R���p�C���ɂ�DXLib��        |
|                   �K�v�ł�|
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
    //�m�[�g�K�C�h�\��
    for (int i = 50; i <= 350; i += 100)DrawRotaGraph(i, 0, 0.4, 0, Handle, TRUE);
    //�R�����g�\��
    DrawFormatString(500, 50, GetColor(0, 128, 255), comment[stricy].c_str());
    DrawFormatString(500, 90, GetColor(128, 64, 255), ("     Miss:" + std::to_string(noteTiming[0] + noteTiming[1])).c_str());
    DrawFormatString(500, 110, GetColor(128, 64, 255), ("     Nice:" + std::to_string(noteTiming[2])).c_str());
    DrawFormatString(500, 130, GetColor(128, 64, 255), ("Wonderful:" + std::to_string(noteTiming[3])).c_str());
    if (!playing)DrawFormatString(450, 200, GetColor(255, 255, 255), "Pless Space to Play");
    //�m�[�g�̕\��
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
        //�^�C�g�����
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

        //�Q�[�����[�f�B���O
        // 
        //�ϐ��E�萔�̐ݒ�
        bool autoPlay = true;
        int stricy1 = 60, stricy2 = 40, stricy3 = 10, stricy = 0, divide = 4500, volume = 50, sumx = 0;
        int pauseSelect = 0, MusicTime = 0;
        int Handle, Note, Music, Press, Quit, Continue;//�f�[�^�n���h���i�[�p�ϐ�
        bool releaseKey1 = true, releaseKey2 = true, releaseKey3 = true, releaseKey4 = true, pause = false;
        bool playingGame = true;
        double startTime = 0, secTime = 0, pauseStartTime = 0, pauseEndTime = 0;
        short notes[4096]{};
        float notesSpeed[4096]{}, notesBPM[4096]{};
        std::string comment[] = { "", "Miss", "Nice", "Wonderful" };
        Handle = LoadGraph("�摜/guide.png", TRUE);
        Note = LoadGraph("�摜/Note.png", TRUE);
        Quit = LoadGraph("�摜/�Q�[������߂�.png");
        Continue = LoadGraph("�摜/�Q�[���𑱂���.png");
        Press = LoadSoundMem("�T�E���h/�^�b�v���ʉ�.wav");
        double OFFSET = 0;
        int MusicOFFSET = 0;
        float speed, BPM;
        bool MusicPlay = false, playing = false;
        int noteTiming[4] = { 0,0,0,0 };
        std::string LINE = "";

        //�ǂݍ��ރt�@�C����
        std::ifstream file("Scores/��̐��.score");

        //�t�@�C���ǂݍ���
        while (std::getline(file, LINE)) {
            if (LINE.find("Music:") != std::string::npos) {
                //�L�[���[�h�uMusic:�v��̃L�[���[�h�̓ǂݎ��
                size_t position = LINE.find("Music:");
                std::string MusicFile = "�T�E���h/Music/" + LINE.substr(position + 6);
                Music = LoadSoundMem(MusicFile.c_str());
                continue;
            }
            else if (LINE.find("BPM:") != std::string::npos) {
                //�L�[���[�h�uBPM:�v��̐��l�̓ǂݎ��
                size_t position = LINE.find("BPM:");
                std::string sBPM = LINE.substr(position + 4);
                BPM = stof(sBPM);
                continue;
            }
            else if (LINE.find("Speed:") != std::string::npos) {
                //�L�[���[�h�uSpeed:�v��̐��l�̓ǂݎ��
                size_t position = LINE.find("Speed:");
                std::string Speed = LINE.substr(position + 6);
                speed = stof(Speed);
                continue;
            }
            else if (LINE.find("Offset:") != std::string::npos) {
                //�L�[���[�h�uOffset:�v��̐��l�̓ǂݎ��
                size_t position = LINE.find("Offset:");
                std::string Offset = LINE.substr(position + 7);
                OFFSET = stod(Offset);
                continue;
            }
            else if (LINE.find("Musicoffset:") != std::string::npos) {
                //�L�[���[�h�uMusicoffset:�v��̐��l�̓ǂݎ��
                size_t position = LINE.find("Musicoffset:");
                std::string MusicOffset = LINE.substr(position + 12);
                MusicOFFSET = stoi(MusicOffset);
                continue;
            }
            else {
                //���l�̓ǂݎ��
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

        //�Q�[�����[�v�֐�
        while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && playingGame) {


            if (pause == false) {
                if (ProcessMessage() != 0)break;//�G���[���ɏI��
                double time = GetNowCount();//���Ԃ��v��
                if (playing)secTime = (time - startTime) / 1000;//���݂̃Q�[���o�ߎ��Ԃ��v�Z

                // ���ʂ̐ݒ�
                ChangeVolumeSoundMem(255 * volume / 100, Music);
                ChangeVolumeSoundMem(255 * volume / 100, Press);
                //�T�E���h�̍Đ�
                if (secTime > MusicOFFSET && !MusicPlay) {
                    MusicPlay = true;
                    PlaySoundMem(Music, DX_PLAYTYPE_BACK);
                }
                else if (secTime > MusicOFFSET) {
                    //�Đ����I���ƃ��[�v�𔲂���
                    if (CheckSoundMem(Music) == 0) {
                        playingGame = false;

                    }
                }
                DrawFormatString(500, 90, GetColor(128, 64, 255), ("     Miss:" + std::to_string(noteTiming[0] + noteTiming[1])).c_str());
                //�X�y�[�X�L�[�ŊJ�n
                if (CheckHitKey(KEY_INPUT_SPACE) && !MusicPlay) { startTime = GetNowCount(); playing = true; }
                //�`�����
                ClearDrawScreen();
                //�`�揈��
                Graph(Handle, comment, stricy, noteTiming, sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, Note, playing);
                if (autoPlay) {
                    //�I�[�g�v���C�I�����̏���
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
                    //�I�[�g�v���C�I�t���̏���
                    //D�L�[�������ꂽ���̔���
                    if (CheckHitKey(KEY_INPUT_D) != 0 && releaseKey1) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 1);
                        releaseKey1 = false;
                    }
                    //F�L�[�������ꂽ���̔���
                    if (CheckHitKey(KEY_INPUT_F) != 0 && releaseKey2) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 2);
                        releaseKey2 = false;
                    }
                    //J�L�[�������ꂽ���̔���
                    if (CheckHitKey(KEY_INPUT_J) != 0 && releaseKey3) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 3);
                        releaseKey3 = false;
                    }
                    //K�L�[�������ꂽ���̔���
                    if (CheckHitKey(KEY_INPUT_K) != 0 && releaseKey4) {
                        stricy = judgeAll(sumx, secTime, OFFSET, divide, notesBPM, notesSpeed, notes, stricy1, stricy2, stricy3, Press, noteTiming, 4);
                        releaseKey4 = false;
                    }
                    releaseKey1 = (CheckHitKey(KEY_INPUT_D) == 0);//D�L�[�������ꂽ���̃t���O
                    releaseKey2 = (CheckHitKey(KEY_INPUT_F) == 0);//F�L�[�������ꂽ���̃t���O
                    releaseKey3 = (CheckHitKey(KEY_INPUT_J) == 0);//J�L�[�������ꂽ���̃t���O
                    releaseKey4 = (CheckHitKey(KEY_INPUT_K) == 0);//K�L�[�������ꂽ���̃t���O
                }

                /*
                if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
                    //ESC�Œ��f����
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
                //�|�[�Y������
                if (CheckHitKey(KEY_INPUT_SPACE) != 0) {
                    if (pauseSelect == 0) {
                        //�|�[�Y�Z���N�g�O�ԁi�Q�[���ɖ߂�j���Z���N�g���ꂽ��
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

//���C���֐�
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