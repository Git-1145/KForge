/**
 * @file    dbgKLOGGER.cpp
 * @brief   KLOGGER 日志模块全功能测试
 *
 * 测试内容:
 *   1. KLOG_INFO / KLOG_WARNING / KLOG_ERROR 宏
 *   2. 各模块错误码触发
 *   3. MakeCode 错误码组装验证
 *   4. Table 码表查询
 *   5. LogLevel / Module 枚举值
 *   6. Color 颜色常量展示
 *   7. KLOG_FATAL 终止测试 (最后执行, 会终止程序)
 */

#include "../base/KF.hpp"
using namespace KFIO;
using namespace KSON;
using namespace KLOG;
using namespace KCLI;

// ==================== 测试辅助宏 ====================
#define SECTION(name) kout << Color::Bold << "\n--- " << name << " ---" << Color::Reset << std::endl

// 辅助函数: 将 Code 转为 hex 字符串 (避免直接操作 cout 格式状态)
static std::string HexCode(Code c)
{
    std::ostringstream oss;
    oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << c;
    return oss.str();
}

int main()
{
    KBegin(read(Preprocess(
        "\"title\": \"dbgKLOGGER 模块测试\","
        "\"description\": \"KLOGGER 日志模块全功能测试\""
    )));

    // ==================== 1. 日志宏 ====================
    SECTION("1. KLOG_INFO / WARNING / ERROR");
    {
        kout << "  >> KLOG_INFO(TEST_INFO, \"这是一条信息日志\")" << std::endl;
        KLOG_INFO(TEST_INFO, "这是一条信息日志");

        kout << "  >> KLOG_WARNING(TEST_WARN, \"这是一条警告日志\")" << std::endl;
        KLOG_WARNING(TEST_WARN, "这是一条警告日志");

        kout << "  >> KLOG_ERROR(TEST_ERROR, \"这是一条错误日志\")" << std::endl;
        KLOG_ERROR(TEST_ERROR, "这是一条错误日志");

        kout << "  >> KLOG_INFO(TEST_INFO, \"\")  (空 extra)" << std::endl;
        KLOG_INFO(TEST_INFO, "");
    }

    // ==================== 2. 各模块错误码 ====================
    SECTION("2. 各模块错误码触发");
    {
        // KSON 模块
        kout << "  >> KLOG_WARNING(KSON_PARSE_MULPOINT, \"multiple dots\")" << std::endl;
        KLOG_WARNING(KSON_PARSE_MULPOINT, "multiple dots in number");

        kout << "  >> KLOG_ERROR(KSON_PARSE_NUMOR, \"overflow\")" << std::endl;
        KLOG_ERROR(KSON_PARSE_NUMOR, "number overflow");

        kout << "  >> KLOG_WARNING(KSON_PARSE_ESCAPE_SPECIAL, \"bad escape\")" << std::endl;
        KLOG_WARNING(KSON_PARSE_ESCAPE_SPECIAL, "bad escape char");

        kout << "  >> KLOG_ERROR(KSON_PARSE_STR_NOEND, \"no closing quote\")" << std::endl;
        KLOG_ERROR(KSON_PARSE_STR_NOEND, "no closing quote");

        // KCLI 模块
        kout << "  >> KLOG_WARNING(KCLI_INPUT_INVALID, \"bad input\")" << std::endl;
        KLOG_WARNING(KCLI_INPUT_INVALID, "bad input");

        // KFIO 模块
        kout << "  >> KLOG_ERROR(KFIO_FILE_OPEN_FAIL, \"simulated\")" << std::endl;
        // 注: KFIO_FILE_OPEN_FAIL 是 Fatal 级别, 这里用 Error 级别的码演示
        KLOG_ERROR(KSON_PARSE_VAL_ERROR, "simulated KFIO error");
    }

    // ==================== 3. MakeCode 错误码组装 ====================
    SECTION("3. MakeCode 错误码组装");
    {
        // 测试模块
        kout << "  TEST_INFO  = " << HexCode(TEST_INFO)  << std::endl;
        kout << "  TEST_WARN  = " << HexCode(TEST_WARN)  << std::endl;
        kout << "  TEST_ERROR = " << HexCode(TEST_ERROR) << std::endl;
        kout << "  TEST_FATAL = " << HexCode(TEST_FATAL) << std::endl;

        // KFIO 模块
        kout << "  KFIO_FILE_OPEN_FAIL = " << HexCode(KFIO_FILE_OPEN_FAIL) << std::endl;
        kout << "  KFIO_FILE_READ_FAIL = " << HexCode(KFIO_FILE_READ_FAIL) << std::endl;

        // KSON 模块
        kout << "  KSON_PARSE_STRE      = " << HexCode(KSON_PARSE_STRE)      << std::endl;
        kout << "  KSON_PARSE_MULPOINT  = " << HexCode(KSON_PARSE_MULPOINT)  << std::endl;
        kout << "  KSON_TYPE_MISMATCH   = " << HexCode(KSON_TYPE_MISMATCH)   << std::endl;

        // KCLI 模块
        kout << "  KCLI_INPUT_INVALID   = " << HexCode(KCLI_INPUT_INVALID)   << std::endl;

        // UNKNOWN
        kout << "  UNKNOWN              = " << HexCode(UNKNOWN)              << std::endl;

        // 手动组装验证
        Code manual = MakeCode(Module::KSON, LogLevel::Error, 0x01, 0x001);
        kout << "  MakeCode(KSON, Error, 0x01, 0x001) = " << HexCode(manual) << std::endl;
        kout << "  == KSON_PARSE_STRE ? " << (manual == KSON_PARSE_STRE ? "YES" : "NO") << std::endl;
    }

    // ==================== 4. Table 码表查询 ====================
    SECTION("4. Table 码表查询");
    {
        auto show = [](const char* name, Code code) {
            auto it = Table.find(code);
            std::string desc = (it != Table.end()) ? std::string(it->second) : "(not in table)";
            kout << "  " << name << " -> " << desc << std::endl;
        };

        show("TEST_INFO",              TEST_INFO);
        show("TEST_WARN",              TEST_WARN);
        show("TEST_ERROR",             TEST_ERROR);
        show("KSON_PARSE_STRE",        KSON_PARSE_STRE);
        show("KSON_PARSE_MULPOINT",    KSON_PARSE_MULPOINT);
        show("KFIO_FILE_OPEN_FAIL",    KFIO_FILE_OPEN_FAIL);
        show("KCLI_INPUT_INVALID",     KCLI_INPUT_INVALID);
        show("UNKNOWN",                UNKNOWN);

        // 不在码表中的码
        Code fake = MakeCode(Module::Common, LogLevel::Info, 0xFF, 0xFFF);
        auto it = Table.find(fake);
        kout << "  (fake code) -> " << (it != Table.end() ? std::string(it->second) : "(not in table)") << std::endl;
    }

    // ==================== 5. LogLevel / Module 枚举 ====================
    SECTION("5. LogLevel / Module 枚举");
    {
        kout << "  LogLevel::Info    = " << static_cast<uint32_t>(LogLevel::Info)    << std::endl;
        kout << "  LogLevel::Warning = " << static_cast<uint32_t>(LogLevel::Warning) << std::endl;
        kout << "  LogLevel::Error   = " << static_cast<uint32_t>(LogLevel::Error)   << std::endl;
        kout << "  LogLevel::Fatal   = " << static_cast<uint32_t>(LogLevel::Fatal)   << std::endl;

        kout << "  Module::Unknown = " << HexCode(Module::Unknown) << std::endl;
        kout << "  Module::Common  = " << HexCode(Module::Common)  << std::endl;
        kout << "  Module::KFIO    = " << HexCode(Module::KFIO)    << std::endl;
        kout << "  Module::KSON    = " << HexCode(Module::KSON)    << std::endl;
        kout << "  Module::KTIMER  = " << HexCode(Module::KTIMER)  << std::endl;
        kout << "  Module::KCLI    = " << HexCode(Module::KCLI)    << std::endl;
    }

    // ==================== 6. Color 颜色常量展示 ====================
    SECTION("6. Color 颜色常量展示");
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

        // 日志级别对应颜色
        kout << "  日志颜色映射:" << std::endl;
        kout << Color::Green       << "    [INFO] Info 级别 = Green"       << Color::Reset << std::endl;
        kout << Color::LightYellow << "    [WARNING] Warning 级别 = LightYellow" << Color::Reset << std::endl;
        kout << Color::Orange      << "    [ERROR] Error 级别 = Orange"     << Color::Reset << std::endl;
        kout << Color::Bold << Color::Red << "    [FATAL] Fatal 级别 = Bold+Red" << Color::Reset << std::endl;
    }

    // ==================== 完成 ====================
    kout << Color::Bold << "\n=== dbgKLOGGER 测试完成 (除 Fatal) ===" << Color::Reset << std::endl;

    // ==================== 7. KLOG_FATAL 终止测试 (最后执行) ====================
    SECTION("7. KLOG_FATAL - 终止测试");
    {
        koutW << "  警告: 以下操作将触发 KLOG_FATAL 并终止程序" << std::endl;
        kout << "  >> KLOG_FATAL(TEST_FATAL, \"程序将终止\")" << std::endl;
        KLOG_FATAL(TEST_FATAL, "程序将终止");

        // 不会执行到这里
    }

    return 0;
}
