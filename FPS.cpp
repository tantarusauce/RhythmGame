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