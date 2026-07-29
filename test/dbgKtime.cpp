#include "../base/KF.hpp"
using namespace std;
int main()
{
    timer::KTimerManager tm;
    tm.create("test", timer::KTimeUnit::ms);
    // 操作计时器
    tm.get("test").pause();
    tm.get("test").print();
    tm.get("test").start();

    Sleep(1010);
    tm.get("test").print();
    tm.get("test").clear();
    // 创建第二个计时器
    tm.create("loop_timer", timer::KTimeUnit::s);
    Sleep(3410);
    tm.get("loop_timer").print();
    cli::programEnd();
    return 0;
}