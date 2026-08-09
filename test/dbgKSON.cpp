/**
 * @file    dbgKSON.cpp
 * @brief   KSON 解析模块全功能测试
 *
 * 测试内容:
 *   1.  字符串解析 (隐式对象 / 显式对象 / 数组 / 单值)
 *   2.  文件解析 (ReadKsonFile / ParseFile)
 *   3.  节点类型判断 (IsNull / IsBool / IsInt / IsDec / IsString / IsArray / IsObject)
 *   4.  节点取值 (AsBool / AsInt / AsDec / AsStr / AsArr / AsObj)
 *   5.  NodePtr 路径访问 (operator[] / TryResolve / Resolve)
 *   6.  find / at 查找
 *   7.  size 大小
 *   8.  Auto() 类型自动转换
 *   9.  多维数组遍历 (2D / 3D)
 *  10.  字符串转义
 *  11.  注释 / 尾随逗号
 *  12.  重复键覆盖
 *  13.  空数组 / 空对象
 *  14.  NodePtr 取值方法 (Str / Int / Dec / Bool / Size / Exists)
 *  15.  Preprocess 预处理
 *  16.  错误条件 (警告级)
 */

#include "../base/KF.hpp"
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
    KBegin(read(Preprocess(
        "\"title\": \"dbgKSON 模块测试\","
        "\"description\": \"KSON 解析模块全功能测试\""
    )));

    // ==================== 1. 字符串解析 ====================
    SECTION("1. 字符串解析");

    // 隐式对象 (无外层 {})
    kson implicit_obj = read(Preprocess(
        "\"name\": \"Alice\","
        "\"age\": 25,"
        "\"score\": 95.5"
    ));
    CHECK(implicit_obj.Exists(), "隐式对象解析");
    SHOW("name",  implicit_obj["name"].Auto());
    SHOW("age",   implicit_obj["age"].Auto());
    SHOW("score", implicit_obj["score"].Auto());

    // 显式对象
    kson explicit_obj = read(Preprocess(
        "{\"key\": \"value\", \"num\": 42}"
    ));
    CHECK(explicit_obj["key"].Exists(), "显式对象解析");
    SHOW("key", explicit_obj["key"].Auto());
    SHOW("num", explicit_obj["num"].Auto());

    // 数组
    kson arr = read(Preprocess("[1, 2, 3, 4, 5]"));
    CHECK(arr.size() == 5, "数组解析 (size=5)");
    std::size_t idx0 = 0, idx4 = 4;
    SHOW("arr[0]", arr[idx0].Auto());
    SHOW("arr[4]", arr[idx4].Auto());

    // 单值
    SHOW("单值 int",  read(Preprocess("42")).Auto());
    SHOW("单值 dec",  read(Preprocess("3.14")).Auto());
    SHOW("单值 str",  read(Preprocess("\"hello\"")).Auto());
    SHOW("单值 bool", read(Preprocess("true")).Auto());
    SHOW("单值 null", read(Preprocess("null")).Auto());

    // ==================== 2. 文件解析 ====================
    SECTION("2. 文件解析");
    {
        // ReadKsonFile: 一站式 读取 + 预处理 + 解析
        kson doc = ReadKsonFile("cfg.kson");
        CHECK(doc.Exists(), "ReadKsonFile 读取 cfg.kson");
        SHOW("intro.name",    doc["intro"]["name"].Auto());
        SHOW("intro.version", doc["intro"]["version"].Auto());

        // NodePtr::ParseFile 静态工厂
        kson doc2 = NodePtr::ParseFile("cfg.kson");
        CHECK(doc2.Exists(), "NodePtr::ParseFile 读取 cfg.kson");

        // 手动流程: ReadFileRaw + Preprocess + read
        kson doc3 = read(Preprocess(ReadFileRaw("cfg.kson")));
        CHECK(doc3.Exists(), "手动流程读取 cfg.kson");
    }

    // 后续测试统一使用 ReadKsonFile 读取的 doc
    kson doc = ReadKsonFile("cfg.kson");

    // ==================== 3. 节点类型判断 ====================
    SECTION("3. 节点类型判断");
    {
        const Node* n;

        n = doc["types"]["integer"].TryResolve();
        CHECK(n && n->IsInt(),    "IsInt (42)");
        CHECK(n && n->IsNumber(), "IsNumber (42)");

        n = doc["types"]["decimal"].TryResolve();
        CHECK(n && n->IsDec(),    "IsDec (3.14)");
        CHECK(n && n->IsNumber(), "IsNumber (3.14)");

        n = doc["types"]["string"].TryResolve();
        CHECK(n && n->IsString(), "IsString");

        n = doc["types"]["bool_true"].TryResolve();
        CHECK(n && n->IsBool(),   "IsBool (true)");

        n = doc["types"]["null_value"].TryResolve();
        CHECK(n && n->IsNull(),   "IsNull");

        n = doc["types"].TryResolve();
        CHECK(n && n->IsObject(), "IsObject (types)");

        n = doc["arrays"]["int_array"].TryResolve();
        CHECK(n && n->IsArray(),  "IsArray (int_array)");
    }

    // ==================== 4. 节点取值 ====================
    SECTION("4. 节点取值");
    {
        const Node* n;

        n = doc["types"]["integer"].TryResolve();
        SHOW("AsInt (42)",       n->AsInt());
        SHOW("AsDec (from int)", n->AsDec());

        n = doc["types"]["negative_int"].TryResolve();
        SHOW("AsInt (-7)",       n->AsInt());

        n = doc["types"]["decimal"].TryResolve();
        SHOW("AsDec (3.14)",     n->AsDec());

        n = doc["types"]["string"].TryResolve();
        SHOW("AsStr",            n->AsStr());

        n = doc["types"]["bool_true"].TryResolve();
        SHOW("AsBool (true)",    n->AsBool());

        n = doc["types"]["bool_false"].TryResolve();
        SHOW("AsBool (false)",   n->AsBool());

        n = doc["arrays"]["int_array"].TryResolve();
        SHOW("AsArr().size()",   n->AsArr().size());
    }

    // ==================== 5. NodePtr 路径访问 ====================
    SECTION("5. NodePtr 路径访问");
    {
        // 链式对象访问
        SHOW("doc[intro][name]",
             doc["intro"]["name"].Auto());

        // 深层嵌套
        SHOW("doc[objects][nested][level1][level2][deep_value]",
             doc["objects"]["nested"]["level1"]["level2"]["deep_value"].Auto());

        // 数组下标访问
        std::size_t idx2 = 2;
        SHOW("doc[arrays][int_array][2]",
             doc["arrays"]["int_array"][idx2].Auto());

        // const char* 重载
        SHOW("operator[](const char*)",
             doc["types"]["string"].Auto());

        // 不存在的路径
        CHECK(!doc["nonexistent"].Exists(),             "不存在路径 Exists()=false");
        CHECK(doc["nonexistent"].Auto() == "null",      "不存在路径 Auto()=\"null\"");
        CHECK(doc["nonexistent"].TryResolve() == nullptr, "不存在路径 TryResolve()=nullptr");
    }

    // ==================== 6. find / at ====================
    SECTION("6. find / at");
    {
        const Node* types = doc["types"].TryResolve();
        if (types)
        {
            const Node* found = types->find("integer");
            CHECK(found != nullptr, "find(\"integer\") 找到");
            SHOW("  found value",   found->AsInt());

            const Node* not_found = types->find("nonexistent");
            CHECK(not_found == nullptr, "find(\"nonexistent\") 返回 nullptr");

            // find 区分大小写
            const Node* case_sensitive = types->find("Integer");
            CHECK(case_sensitive == nullptr, "find 区分大小写 (\"Integer\" != \"integer\")");
        }

        const Node* arr_node = doc["arrays"]["int_array"].TryResolve();
        if (arr_node)
        {
            const Node* elem = arr_node->at(2);
            CHECK(elem != nullptr,    "at(2) 找到");
            SHOW("  at(2) value",     elem->AsInt());

            const Node* oob = arr_node->at(999);
            CHECK(oob == nullptr,     "at(999) 越界返回 nullptr");
        }
    }

    // ==================== 7. size ====================
    SECTION("7. size");
    {
        const Node* arr_node = doc["arrays"]["int_array"].TryResolve();
        CHECK(arr_node->size() == 5, "int_array Node::size() = 5");

        const Node* types = doc["types"].TryResolve();
        SHOW("types Node::size()", types->size());

        const Node* empty_arr = doc["arrays"]["empty_array"].TryResolve();
        CHECK(empty_arr->size() == 0, "empty_array size = 0");

        // NodePtr::Size() 和 size() 别名
        SHOW("NodePtr::Size()",    doc["arrays"]["int_array"].Size());
        SHOW("NodePtr::size()",    doc["arrays"]["int_array"].size());

        // 标量 size() = 0
        const Node* scalar = doc["types"]["integer"].TryResolve();
        CHECK(scalar->size() == 0, "标量 size() = 0");
    }

    // ==================== 8. Auto() 类型自动转换 ====================
    SECTION("8. Auto() 类型自动转换");
    {
        SHOW("int",  doc["types"]["integer"].Auto());
        SHOW("dec",  doc["types"]["decimal"].Auto());
        SHOW("str",  doc["types"]["string"].Auto());
        SHOW("bool", doc["types"]["bool_true"].Auto());
        SHOW("null", doc["types"]["null_value"].Auto());
        SHOW("arr",  doc["arrays"]["int_array"].Auto());
        SHOW("obj",  doc["types"].Auto());
        SHOW("empty_arr", doc["arrays"]["empty_array"].Auto());
        SHOW("empty_obj", doc["objects"]["empty_obj"].Auto());
    }

    // ==================== 9. 多维数组遍历 ====================
    SECTION("9. 多维数组遍历");
    {
        // 2D 数组
        kout << "  2D 数组:" << std::endl;
        kson arr2d = doc["arrays"]["nested_2d"];
        for (size_t i = 0; i < arr2d.size(); i++)
        {
            kout << "    [";
            for (size_t j = 0; j < arr2d[i].size(); j++)
            {
                if (j) kout << ", ";
                kout << arr2d[i][j].Auto();
            }
            kout << "]" << std::endl;
        }

        // 3D 数组
        kout << "  3D 数组:" << std::endl;
        kson arr3d = doc["arrays"]["nested_3d"];
        for (size_t i = 0; i < arr3d.size(); i++)
        {
            kout << "    Layer " << i << ":" << std::endl;
            for (size_t j = 0; j < arr3d[i].size(); j++)
            {
                kout << "      [";
                for (size_t k = 0; k < arr3d[i][j].size(); k++)
                {
                    if (k) kout << ", ";
                    kout << arr3d[i][j][k].Auto();
                }
                kout << "]" << std::endl;
            }
        }
    }

    // ==================== 10. 字符串转义 ====================
    SECTION("10. 字符串转义");
    {
        SHOW("newline",      doc["escapes"]["newline"].Auto());
        SHOW("tab",          doc["escapes"]["tab"].Auto());
        SHOW("quote",        doc["escapes"]["quote"].Auto());
        SHOW("backslash",    doc["escapes"]["backslash"].Auto());
        SHOW("carriage_ret", doc["escapes"]["carriage_return"].Auto());
        SHOW("backspace",    doc["escapes"]["backspace"].Auto());
    }

    // ==================== 11. 注释 / 尾随逗号 ====================
    SECTION("11. 注释 / 尾随逗号");
    {
        // cfg.kson 本身包含注释和尾随逗号
        CHECK(doc.Exists(), "含注释的文件解析成功");

        // 尾随逗号 - 数组
        kson trail_arr = read(Preprocess("[1, 2, 3,]"));
        CHECK(trail_arr.size() == 3, "尾随逗号数组 (size=3)");

        // 尾随逗号 - 对象
        kson trail_obj = read(Preprocess("{\"a\": 1, \"b\": 2,}"));
        CHECK(trail_obj["a"].Exists(), "尾随逗号对象");

        // 注释行
        kson commented = read(Preprocess(
            "# 这是注释\n\"key\": \"value\" # 行尾注释\n"
        ));
        CHECK(commented["key"].Auto() == "value", "注释行正确跳过");
    }

    // ==================== 12. 重复键覆盖 ====================
    SECTION("12. 重复键覆盖");
    {
        SHOW("dup_key (应为 second)", doc["dup_key"].Auto());
        CHECK(doc["dup_key"].Auto() == "second", "重复键后覆盖前");
    }

    // ==================== 13. 空数组 / 空对象 ====================
    SECTION("13. 空数组 / 空对象");
    {
        const Node* ea = doc["arrays"]["empty_array"].TryResolve();
        CHECK(ea && ea->IsArray() && ea->size() == 0, "空数组 IsArray + size=0");

        const Node* eo = doc["objects"]["empty_obj"].TryResolve();
        CHECK(eo && eo->IsObject() && eo->size() == 0, "空对象 IsObject + size=0");

        SHOW("Auto() 空数组", doc["arrays"]["empty_array"].Auto());
        SHOW("Auto() 空对象", doc["objects"]["empty_obj"].Auto());
    }

    // ==================== 14. NodePtr 取值方法 ====================
    SECTION("14. NodePtr 取值方法");
    {
        kson s = doc["types"]["string"];
        SHOW("Str()",  s.Str());

        kson i = doc["types"]["integer"];
        SHOW("Int()",  i.Int());

        kson d = doc["types"]["decimal"];
        SHOW("Dec()",  d.Dec());

        kson b = doc["types"]["bool_true"];
        SHOW("Bool()", b.Bool());

        kson sz = doc["arrays"]["int_array"];
        SHOW("Size()", sz.Size());

        CHECK(doc["types"]["integer"].Exists(),     "Exists() = true");
        CHECK(!doc["nonexistent"].Exists(),          "Exists() = false (不存在)");
    }

    // ==================== 15. Preprocess 预处理 ====================
    SECTION("15. Preprocess 预处理");
    {
        std::string raw = "# 注释行\n\"key\": \"value\"  # 行尾注释\n";
        std::string processed = Preprocess(raw);
        SHOW("原始长度",   raw.size());
        SHOW("处理后长度", processed.size());

        CHECK(processed.find('#') == std::string::npos, "注释已移除");
        CHECK(processed.find(' ') == std::string::npos, "空白已移除");

        kson result = read(processed);
        CHECK(result["key"].Auto() == "value", "预处理后解析正确");

        // 验证字符串内的 # 不被误删
        std::string raw2 = "\"url\": \"http://example.com#frag\"";
        std::string proc2 = Preprocess(raw2);
        CHECK(proc2.find('#') != std::string::npos, "字符串内 # 保留");
    }

    // ==================== 16. 错误条件 (警告级) ====================
    SECTION("16. 错误条件 (警告级)");
    {
        koutW << "  以下测试会触发 KLOGGER 警告/错误输出到 stderr" << std::endl;

        // 多个小数点 (Warning)
        kout << "  >> 多个小数点 (KSON_PARSE_MULPOINT)" << std::endl;
        kson mulpoint = read(Preprocess("\"x\": 1.2.3"));

        // 尾随字符 (Warning)
        kout << "  >> 尾随字符 (KSON_PARSE_TRAIL)" << std::endl;
        kson trail = read(Preprocess("42 extra"));

        // 无效转义 (Warning)
        kout << "  >> 无效转义 (KSON_PARSE_ESCAPE_SPECIAL)" << std::endl;
        kson badesc = read(Preprocess("\"x\": \"bad\\xescape\""));
    }

    // ==================== 17. BigNum / 科学计数法 ====================
    SECTION("17. BigNum / 科学计数法");
    {
        // testu: 使用 cfg.kson 中的 kson_bignum 数据
        kson bn = doc["kson_bignum"];

        // int64 范围内 → 普通整数
        kout << "  >> int64_max (普通整数)" << std::endl;
        const Node* int64max = bn["int64_max"].TryResolve();
        CHECK(int64max && int64max->IsInt(), "int64_max 是 Int 类型");
        CHECK(int64max->AsInt() == 9223372036854775807LL, "int64_max 值正确");

        // 超出 int64 → 自动 BigNum
        kout << "  >> overflow_big (自动转 BigNum)" << std::endl;
        const Node* obig = bn["overflow_big"].TryResolve();
        CHECK(obig && obig->IsBig(), "overflow_big 是 BigNum 类型");
        CHECK(obig->AsBig().ToStr() == "123456789012345678901234567890", "overflow_big 值正确");

        // 负大数
        kout << "  >> negative_big (负大数)" << std::endl;
        const Node* nbig = bn["negative_big"].TryResolve();
        CHECK(nbig && nbig->IsBig(), "negative_big 是 BigNum 类型");
        CHECK(nbig->AsBig().isneg == true, "negative_big 为负数");
        SHOW("  negative_big = ", nbig->AsBig().ToStr());

        // 科学计数法 → 自动 BigNum
        kout << "  >> sci_big (科学计数法 1.23e50)" << std::endl;
        const Node* sci = bn["sci_big"].TryResolve();
        CHECK(sci && sci->IsBig(), "sci_big 是 BigNum 类型");
        SHOW("  sci_big = ", sci->AsBig().ToStr());

        // 小科学计数法
        kout << "  >> sci_small (科学计数法 1.23e-10)" << std::endl;
        const Node* sci_s = bn["sci_small"].TryResolve();
        CHECK(sci_s && sci_s->IsBig(), "sci_small 是 BigNum 类型");
        SHOW("  sci_small = ", sci_s->AsBig().ToStr());

        // 负科学计数法
        kout << "  >> sci_neg (科学计数法 -5e20)" << std::endl;
        const Node* sci_n = bn["sci_neg"].TryResolve();
        CHECK(sci_n && sci_n->IsBig(), "sci_neg 是 BigNum 类型");
        SHOW("  sci_neg = ", sci_n->AsBig().ToStr());

        // 'B' 后缀强制 BigNum
        kout << "  >> big_suffix (123456789B)" << std::endl;
        const Node* bs = bn["big_suffix"].TryResolve();
        CHECK(bs && bs->IsBig(), "big_suffix 是 BigNum 类型");
        SHOW("  big_suffix = ", bs->AsBig().ToStr());

        // 负 'B' 后缀
        kout << "  >> neg_big_suffix (-987654321B)" << std::endl;
        const Node* nbs = bn["neg_big_suffix"].TryResolve();
        CHECK(nbs && nbs->IsBig(), "neg_big_suffix 是 BigNum 类型");
        CHECK(nbs->AsBig().isneg == true, "neg_big_suffix 为负数");
        SHOW("  neg_big_suffix = ", nbs->AsBig().ToStr());

        // 科学计数法 3e25 → 自动 BigNum
        kout << "  >> sci_auto (3e25)" << std::endl;
        const Node* sa = bn["sci_auto"].TryResolve();
        CHECK(sa && sa->IsBig(), "sci_auto 是 BigNum 类型");
        SHOW("  sci_auto = ", sa->AsBig().ToStr());

        // NodePtr::AsBig() 方法
        kout << "  >> NodePtr::AsBig() 方法" << std::endl;
        KBIGNUM::BigNum bigval = bn["overflow_big"].AsBig();
        SHOW("  AsBig() = ", bigval.ToStr());
    }

    // ==================== 完成 ====================
    kout << Color::Bold << "\n=== dbgKSON 所有测试完成 ===" << Color::Reset << std::endl;
    KEnd();
}
