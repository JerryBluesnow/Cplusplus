
// MyTimer.h//
#include <windows.h>
#include <iostream>

using namespace std;

class MyTimer
{
   private:
    int           _freq;
    LARGE_INTEGER _begin;
    LARGE_INTEGER _end;

   public:
    long costTime;  // 花费的时间(精确到微秒)

   public:
    MyTimer()
    {
        LARGE_INTEGER tmp;
        QueryPerformanceFrequency(
            &tmp);  // QueryPerformanceFrequency()作用：返回硬件支持的高精度计数器的频率。

        _freq    = tmp.QuadPart;
        costTime = 0;
    }

    void
    Start()  // 开始计时
    {
        QueryPerformanceCounter(&_begin);  //获得初始值
    }

    void
    End()  // 结束计时
    {
        QueryPerformanceCounter(&_end);  //获得终止值
        costTime = (long)((_end.QuadPart - _begin.QuadPart) * 1000000 / _freq);
        std::cout << "timer spend: "<< costTime << endl;
    }

    void
    Reset()  // 计时清0
    {
        costTime = 0;
    }


};

MyTimer little_timer;

// main.cpp
// #include "MyTimer.h"
// #include <iostream>

// int
// main()
// {
//     MyTimer      timer;
//     unsigned int t = 0;
//     timer.Start();
//     while (t++ < 10e+5)
//         ;
//     timer.End();
//     //std::cout << "耗时为：" << timer.costTime << "us";
//     return 0;
// }