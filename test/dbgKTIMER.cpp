/**
 * @file    dbgKTIMER.cpp
 * @brief   KTIMER 计时器模块全功能测试
 *
 * 测试内容:
 *   1. AddTimer - 新建计时器（不同名字和单位）
 *   2. AddTimer - 重名覆盖（警告）
 *   3. 计时精度 - sleep 后验证时间
 *   4. PauseTimer - 暂停计时器
 *   5. PauseTimer - 暂停不存在的计时器（警告）
 *   6. PauseTimer - 暂停已暂停的计时器（警告）
 *   7. StartTimer - 恢复计时器
 *   8. StartTimer - 恢复运行中的计时器（警告）
 *   9. GetTimer - 获取计时时间
 *  10. PrintTimer - 打印单个计时器
 *  11. PrintAllTimers - 打印所有计时器
 *  12. DeleteTimer - 删除计时器
 *  13. DeleteTimer - 删除不存在的计时器（警告）
 *  14. 综合工作流
 *  15. 空表打印
 *  16. 边界情况 (空名字 / 已删除 / 不存在)
 */

#include "../base/KF.hpp"
#include <thread>
using namespace KFIO;
using namespace KSON;
using namespace KLOG;
using namespace KCLI;
using namespace KTIMER;

// ==================== 测试辅助宏 ====================
#define SECTION(name) kout << Color::Bold << "\n--- " << name << " ---" << Color::Reset << std::endl

int main()
{
    KBegin(read(Preprocess(
        "\"title\": \"dbgKTIMER 计时器模块测试\","
        "\"description\": \"KTIMER 计时器模块全功能测试\""
    )));

    // ==================== 1. AddTimer 新建计时器 ====================
    SECTION("1. AddTimer 新建计时器");
    {
        AddTimer("render", TimeUnit::ms);
        AddTimer("load",   TimeUnit::us);
        AddTimer("init",   TimeUnit::ns);
        AddTimer("total",  TimeUnit::s);
        kout << "  已创建 4 个计时器: render(ms), load(us), init(ns), total(s)" << std::endl;
    }

    // ==================== 2. AddTimer 重名覆盖 ====================
    SECTION("2. AddTimer 重名覆盖");
    {
        kout << "  >> AddTimer(\"render\", TimeUnit::ms)  (重名, 应触发警告)" << std::endl;
        AddTimer("render", TimeUnit::ms);
        kout << "  render 已被重置" << std::endl;
    }

    // ==================== 3. 计时精度 ====================
    SECTION("3. 计时精度验证");
    {
        AddTimer("sleep100", TimeUnit::ms);
        kout << "  sleep 100ms ..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        PauseTimer("sleep100");
        double elapsed = GetTimer("sleep100");
        kout << "  实测: " << elapsed << " ms (期望 >= 100)" << std::endl;
    }

    // ==================== 4. PauseTimer 暂停 ====================
    SECTION("4. PauseTimer 暂停");
    {
        AddTimer("work", TimeUnit::ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        PauseTimer("work");
        double t1 = GetTimer("work");
        kout << "  暂停后时间: " << t1 << " ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double t2 = GetTimer("work");
        kout << "  暂停期间时间: " << t2 << " ms (应与上次相近)" << std::endl;
    }

    // ==================== 5. PauseTimer 不存在 ====================
    SECTION("5. PauseTimer 不存在 (警告)");
    {
        kout << "  >> PauseTimer(\"nonexistent\")" << std::endl;
        PauseTimer("nonexistent");
    }

    // ==================== 6. PauseTimer 已暂停 ====================
    SECTION("6. PauseTimer 已暂停 (警告)");
    {
        kout << "  >> PauseTimer(\"work\")  (已暂停)" << std::endl;
        PauseTimer("work");
    }

    // ==================== 7. StartTimer 恢复 ====================
    SECTION("7. StartTimer 恢复");
    {
        StartTimer("work");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        PauseTimer("work");
        double t3 = GetTimer("work");
        kout << "  恢复后时间: " << t3 << " ms (应比暂停时增加 ~50ms)" << std::endl;
    }

    // ==================== 8. StartTimer 运行中 ====================
    SECTION("8. StartTimer 运行中 (警告)");
    {
        AddTimer("running", TimeUnit::ms);
        kout << "  >> StartTimer(\"running\")  (运行中)" << std::endl;
        StartTimer("running");
        PauseTimer("running");
    }

    // ==================== 9. GetTimer ====================
    SECTION("9. GetTimer 获取时间");
    {
        double r = GetTimer("render");
        double l = GetTimer("load");
        double i = GetTimer("init");
        double t = GetTimer("total");
        kout << "  render: " << r << " ms" << std::endl;
        kout << "  load:   " << l << " us" << std::endl;
        kout << "  init:   " << i << " ns" << std::endl;
        kout << "  total:  " << t << " s"  << std::endl;

        // 暂停 vs 运行中的 GetTimer 差异
        kout << "  -- 暂停 vs 运行中 对比 --" << std::endl;
        AddTimer("cmp", TimeUnit::ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        PauseTimer("cmp");
        double paused1 = GetTimer("cmp");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double paused2 = GetTimer("cmp");
        kout << "  暂停后  GetTimer: " << paused1 << " ms" << std::endl;
        kout << "  再等50ms GetTimer: " << paused2 << " ms (暂停期间应保持不变)" << std::endl;
        StartTimer("cmp");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double running = GetTimer("cmp");
        kout << "  恢复运行 GetTimer: " << running << " ms (运行中应持续增长)" << std::endl;
        PauseTimer("cmp");

        kout << "  >> GetTimer(\"nonexistent\")" << std::endl;
        double bad = GetTimer("nonexistent");
        kout << "  返回值: " << bad << " (期望 -1)" << std::endl;
    }

    // ==================== 10. PrintTimer ====================
    SECTION("10. PrintTimer 打印单个");
    {
        PrintTimer("render");
        PrintTimer("work");
    }

    // ==================== 11. PrintAllTimers ====================
    SECTION("11. PrintAllTimers 打印全部");
    {
        PrintAllTimers();
    }

    // ==================== 12. DeleteTimer ====================
    SECTION("12. DeleteTimer 删除");
    {
        DeleteTimer("init");
        DeleteTimer("sleep100");
        DeleteTimer("running");
        kout << "  已删除 init, sleep100, running" << std::endl;
        PrintAllTimers();
    }

    // ==================== 13. DeleteTimer 不存在 ====================
    SECTION("13. DeleteTimer 不存在 (警告)");
    {
        kout << "  >> DeleteTimer(\"nonexistent\")" << std::endl;
        DeleteTimer("nonexistent");
    }

    // ==================== 14. 综合工作流 ====================
    SECTION("14. 综合工作流");
    {
        kout << "  模拟: 加载 -> 处理 -> 保存" << std::endl;

        AddTimer("wf_load",   TimeUnit::ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        PauseTimer("wf_load");

        AddTimer("wf_process", TimeUnit::ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        PauseTimer("wf_process");

        AddTimer("wf_save",   TimeUnit::ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        PauseTimer("wf_save");

        PrintAllTimers();

        kout << "  清理工作流计时器..." << std::endl;
        DeleteTimer("wf_load");
        DeleteTimer("wf_process");
        DeleteTimer("wf_save");
    }

    // ==================== 15. 空表打印 ====================
    SECTION("15. 空表打印");
    {
        // 删除所有剩余计时器
        DeleteTimer("render");
        DeleteTimer("load");
        DeleteTimer("total");
        DeleteTimer("work");
        DeleteTimer("cmp");
        kout << "  所有计时器已删除, 打印空表:" << std::endl;
        PrintAllTimers();
    }

    // ==================== 16. 边界情况 ====================
    SECTION("16. 边界情况");
    {
        // (a) AddTimer 空字符串名字
        kout << "  >> AddTimer(\"\", TimeUnit::ms)  (空名字)" << std::endl;
        AddTimer("", TimeUnit::ms);

        // (b) GetTimer 已删除的计时器 (应返回 -1)
        AddTimer("tmp", TimeUnit::ms);
        DeleteTimer("tmp");
        kout << "  >> GetTimer(\"tmp\")  (已删除)" << std::endl;
        double gone = GetTimer("tmp");
        kout << "  返回值: " << gone << " (期望 -1)" << std::endl;

        // (c) PrintTimer 不存在的计时器 (不应崩溃)
        kout << "  >> PrintTimer(\"ghost\")  (不存在, 不应崩溃)" << std::endl;
        PrintTimer("ghost");

        // 清理空名字计时器
        DeleteTimer("");
    }

    // ==================== 完成 ====================
    kout << Color::Bold << "\n=== dbgKTIMER 所有测试完成 ===" << Color::Reset << std::endl;

    KEnd();
}
