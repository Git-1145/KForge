/**
 * @file    dbgKCLI.cpp
 * @brief   KCLI 命令行交互模块全功能测试
 *
 * 测试内容:
 *   1. kout 链式输出 (天蓝色, 多类型)
 *   2. koutW 链式输出 (淡黄色)
 *   3. koutE 链式输出 (橙色)
 *   4. koutF 链式输出 (红色)
 *   5. 临时换色
 *   6. Color 常量展示
 *   7. kin 链式输入 (int / double / string / bool)
 *   8. KOptions 菜单 (字符串配置)
 *   9. 从文件读取配置的 KOptions
 *  10. kpause / kend
 */

#include "../base/KF.hpp"
using namespace KFIO;
using namespace KSON;
using namespace KLOG;
using namespace KCLI;

// ==================== 测试辅助宏 ====================
#define SECTION(name) kout << Color::Bold << "\n--- " << name << " ---" << Color::Reset << std::endl

int main()
{
    auto root = ReadKsonFile("cfg.kson");
    // kbegin: 启用 VT100, 设置标题, 打印标题框
    kbegin(root);

    // ==================== 1. kout 链式输出 ====================
    SECTION("1. kout 链式输出");
    {
        kout << "  整数: " << 42 << std::endl;
        kout << "  浮点: " << 3.14 << std::endl;
        kout << "  字符串: " << "Hello" << std::endl;
        kout << "  字符: " << 'A' << std::endl;
        kout << "  布尔(1): " << true << std::endl;
        kout << "  混合: int=" << 10 << ", dec=" << 2.5 << ", str=" << "test" << std::endl;
    }

    // ==================== 2. koutW 链式输出 ====================
    SECTION("2. koutW 链式输出 (淡黄色)");
    {
        koutW << "  警告: 配置项缺失" << std::endl;
        koutW << "  警告: code=" << 100 << ", msg=timeout" << std::endl;
    }

    // ==================== 3. koutE 链式输出 ====================
    SECTION("3. koutE 链式输出 (橙色)");
    {
        koutE << "  错误: 文件未找到" << std::endl;
        koutE << "  错误: code=" << 404 << ", path=/config.kson" << std::endl;
    }

    // ==================== 4. koutF 链式输出 ====================
    SECTION("4. koutF 链式输出 (红色)");
    {
        koutF << "  致命: 内存耗尽" << std::endl;
        koutF << "  致命: code=" << 500 << ", 无法恢复" << std::endl;
    }

    // ==================== 5. 临时换色 ====================
    SECTION("5. 临时换色");
    {
        kout << "  默认天蓝色输出" << std::endl;
        kout << Color::Red << "  临时红色" << Color::Reset << std::endl;
        kout << "  恢复天蓝色" << std::endl;
        kout << Color::Green << "  临时绿色" << Color::Reset << std::endl;
        kout << Color::Bold << "  加粗文本" << Color::Reset << std::endl;
        kout << Color::Magenta << "  临时紫红" << Color::Reset << std::endl;
    }

    // ==================== 6. Color 常量展示 ====================
    SECTION("6. Color 常量展示");
    {
        kout << Color::Red         << "  Red"          << Color::Reset << std::endl;
        kout << Color::Green       << "  Green"        << Color::Reset << std::endl;
        kout << Color::Yellow      << "  Yellow"       << Color::Reset << std::endl;
        kout << Color::Blue        << "  Blue"         << Color::Reset << std::endl;
        kout << Color::Magenta     << "  Magenta"      << Color::Reset << std::endl;
        kout << Color::Cyan        << "  Cyan"         << Color::Reset << std::endl;
        kout << Color::LightYellow << "  LightYellow"  << Color::Reset << std::endl;
        kout << Color::Orange      << "  Orange"       << Color::Reset << std::endl;
        kout << Color::SkyBlue     << "  SkyBlue (kout 默认)" << Color::Reset << std::endl;
        kout << Color::Bold        << "  Bold"         << Color::Reset << std::endl;
    }

    // ==================== 7. kin 链式输入 ====================
    SECTION("7. kin 链式输入 (自动类型推导)");
    {
        kout << "  请依次输入 (每项一行):" << std::endl;
        kout << "  1. 整数: ";
        int i;
        kin >> i;

        kout << "  2. 浮点数: ";
        double d;
        kin >> d;

        kout << "  3. 字符串: ";
        std::string s;
        kin >> s;

        kout << "  4. 布尔值 (1/0): ";
        bool b;
        kin >> b;

        kout << Color::Bold << "  --- 输入结果 ---" << Color::Reset << std::endl;
        kout << "  int=" << i << ", dec=" << d << ", str=" << s << ", bool=" << b << std::endl;
    }

    // ==================== 8. KOptions 菜单 (字符串配置) ====================
    SECTION("8. KOptions 菜单 (字符串配置)");
    {

        kson menu = read(Preprocess(
            "\"title\": \"请选择操作\","
            "\"options\": [\"选项 A\", \"选项 B\", \"选项 C\", \"返回\"]"
        ));
        size_t choice = KOptions(menu);
        kout << "  你选择了: [" << choice << "] " << menu["options"][choice].Auto() << std::endl;
    }

    // ==================== 9. 从文件读取配置 ====================
    SECTION("9. 从文件读取配置的 KOptions");
    {
        kson cfg = ReadKsonFile("test_cli.kson");
        kout << "  从 test_cli.kson 读取菜单配置" << std::endl;
        kson menu = cfg["main_menu"];
        size_t choice = KOptions(menu);
        kout << "  你选择了: [" << choice << "] " << menu["options"][choice].Auto() << std::endl;
    }

    // ==================== 10. kpause ====================
    SECTION("10. kpause 暂停");
    {
        kout << "  测试 kpause() 暂停功能" << std::endl;
        kpause();
        kout << "  暂停结束, 继续执行" << std::endl;
    }

    // ==================== 完成 ====================
    kout << Color::Bold << "\n=== dbgKCLI 所有测试完成 ===" << Color::Reset << std::endl;

    // kend: 暂停后退出
    kend();
}
