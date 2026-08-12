/// @attention This File has been deprecated.Don't try to use it.
int main()
{
    
}


/*
/*
/**
 * @file    dbgKFIO.cpp
 * @brief   KFIO 文件读写模块全功能测试
 *
 * 测试内容:
 *   1. ReadFileRaw - 读取存在的文件
 *   2. ReadFileRaw - 内容验证
 *   3. ReadFileRaw - 读取二进制/源码文件
 *   4. 与 KSON 集成 (ReadKsonFile / 手动流程)
 *   5. Fatal 测试 - 读取不存在的文件 (最后执行, 会终止程序)

#include "base/KF.hpp"
#include <cstdio>
using namespace KFIO;
using namespace KSON;
using namespace KLOG;
using namespace KCLI;

// ==================== 测试辅助宏 ====================
#define SECTION(name) kout << Color::Bold << "\n--- " << name << " ---" << Color::Reset << std::endl
#define CHECK(cond, desc) do { \
    if (cond) kout << "  [PASS] " << desc << std::endl; \
    else koutE << "  [FAIL] " << desc << std::endl; \
} while(0)
#define SHOW(label, value) kout << "  " << label << ": " << value << std::endl

int main()
{
    KBegin("dbgKFIO 模块测试","KFIO 文件读写模块全功能测试");

    // ==================== 1. ReadFileRaw - 读取存在的文件 ====================
    SECTION("1. ReadFileRaw - 读取存在的文件");
    {
        std::string content = ReadFileRaw("config/test/cfg.kson");
        CHECK(!content.empty(), "文件内容非空");
        SHOW("文件大小", content.size());
        kout << "  前 80 字符:" << std::endl;
        kout << "    " << content.substr(0, 80) << "..." << std::endl;
    }

    // ==================== 2. ReadFileRaw - 内容验证 ====================
    SECTION("2. ReadFileRaw - 内容验证");
    {
        std::string content = ReadFileRaw("config/test/cfg.kson");

        // ReadFileRaw 不做任何处理, 注释和空白都保留
        CHECK(content.find("KSON") != std::string::npos, "内容包含 'KSON'");
        CHECK(content.find("#") != std::string::npos,    "注释保留 (含 #)");
        CHECK(content.find("intro") != std::string::npos, "内容包含 'intro'");
        CHECK(content.find("types") != std::string::npos, "内容包含 'types'");
        CHECK(content.find("\n") != std::string::npos,    "换行符保留");
    }

    // ==================== 3. ReadFileRaw - 读取其他文件 ====================
    SECTION("3. ReadFileRaw - 读取源码文件");
    {
        std::string self = ReadFileRaw("dbgKFIO.cpp");
        CHECK(!self.empty(), "读取 dbgKFIO.cpp 成功");
        CHECK(self.find("#include") != std::string::npos, "内容包含 #include");
        SHOW("dbgKFIO.cpp 大小", self.size());

        // 读取 cfg.kson 并验证 cli_test 段
        kson cli_doc = ReadKsonFile("cfg/cfg.kson");
        CHECK(cli_doc["cli_test"].Exists(), "cfg.kson contains cli_test section");
        CHECK(cli_doc["cli_test"]["title"].Exists(), "cli_test has title");
    }

    // ==================== 4. 与 KSON 集成 ====================
    SECTION("4. 与 KSON 集成");
    {
        // 方式一: ReadKsonFile 一站式 (读取 + 预处理 + 解析)
        kson doc1 = ReadKsonFile("cfg/cfg.kson");
        CHECK(doc1.Exists(), "ReadKsonFile 解析成功");
        SHOW("  intro.name", doc1["intro"]["name"].Auto());

        // 方式二: 手动流程 (ReadFileRaw + Preprocess + read)
        std::string raw = ReadFileRaw("config/test/cfg.kson");
        std::string processed = Preprocess(raw);
        kson doc2 = read(processed);
        CHECK(doc2.Exists(), "手动流程解析成功");
        SHOW("  intro.name", doc2["intro"]["name"].Auto());

        // 方式三: NodePtr::ParseFile
        kson doc3 = NodePtr::ParseFile("config/test/cfg.kson");
        CHECK(doc3.Exists(), "NodePtr::ParseFile 解析成功");

        // 验证三种方式结果一致
        CHECK(doc1["intro"]["name"].Auto() == doc2["intro"]["name"].Auto(),
              "ReadKsonFile 与手动流程结果一致");

        // 验证 Preprocess 确实移除了注释
        CHECK(raw.find('#') != std::string::npos,   "原始文件含注释");
        CHECK(processed.find('#') == std::string::npos, "预处理后注释已移除");
    }

    // ==================== 5. ReadFileRaw - 空文件测试 ====================
    SECTION("5. ReadFileRaw - 空文件测试");
    {
        // 创建一个空文件
        std::ofstream out("empty_test.tmp");
        out.close();

        std::string content = ReadFileRaw("empty_test.tmp");
        CHECK(content.empty(), "空文件内容为空");
        SHOW("空文件大小", content.size());

        // 清理
        std::remove("empty_test.tmp");
    }

    // ==================== 完成 ====================
    kout << Color::Bold << "\n=== dbgKFIO 所有测试完成 ===" << Color::Reset << std::endl;

    // ==================== 6. Fatal 测试 (最后执行, 终止程序) ====================
    SECTION("6. Fatal 测试 - 读取不存在的文件");
    {
        koutW << "  警告: 以下操作将触发 KLOG_FATAL(KFIO_FILE_OPEN_FAIL) 并终止程序" << std::endl;
        kout << "  >> ReadFileRaw(\"nonexistent_file.xyz\")" << std::endl;

        // 这会触发 Fatal: 绝对路径 + errno 会输出到 stderr
        std::string bad = ReadFileRaw("nonexistent_file.xyz");

        // 不会执行到这里
    }

    return 0;
}
*/