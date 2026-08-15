#include "base/KF.hpp"
#include <sstream>
using namespace KBIGNUM;
using namespace KCLI;
using namespace KSON;
using namespace KTIMER;

/// =====================================================================
/// 配置驱动型 BigNum 测试框架
/// 检测单元从 config/test/cfg.kson 的 dbgKBIGNUM.bignum 读取，
/// 通过 {green}[OK]{/} / {red}[FAIL]{/} 颜色直观区分结果。
/// =====================================================================
static int g_ok   = 0;   // 通过计数
static int g_fail = 0;   // 失败计数

/// 通用断言宏：expr 的 BigNum 结果 == expect 期望串
#define CHECK_CFG(expr, exprStr, expectStr, section, idx)                                        \
    do {                                                                                          \
        BigNum _got = (expr);                                                                     \
        BigNum _exp(expectStr);                                                                   \
        bool _bothNan = _got.IsNan() && _exp.IsNan();                                             \
        if(_got == _exp || _bothNan) {                                                            \
            kout << "  " << section << "[" << idx << "] {green}[OK]{/}  " << exprStr << " = " << _got << "\n"; \
            ++g_ok;                                                                               \
        } else {                                                                                  \
            kout << "  " << section << "[" << idx << "] {red}[FAIL]{/}  " << exprStr << " = " << _got \
                 << " , expect " << _exp << "\n";                                                 \
            ++g_fail;                                                                             \
        }                                                                                         \
    } while(0)

/// 字符串断言宏：expr 的 ToStr == 期望串
#define CHECK_CFG_STR(expr, exprStr, expectStr, section, idx)                                     \
    do {                                                                                          \
        std::string _got = (expr);                                                                \
        std::string _exp(expectStr);                                                              \
        if(_got == _exp) {                                                                        \
            kout << "  " << section << "[" << idx << "] {green}[OK]{/}  " << exprStr << " = \"" << _got << "\"\n"; \
            ++g_ok;                                                                               \
        } else {                                                                                  \
            kout << "  " << section << "[" << idx << "] {red}[FAIL]{/}  " << exprStr << " = \"" << _got \
                 << "\" , expect \"" << _exp << "\"\n";                                            \
            ++g_fail;                                                                             \
        }                                                                                         \
    } while(0)

/// 比较断言宏：A 与 B 的关系符号 == expectSign(1/-1/0)
#define CHECK_CFG_CMP(aStr, bStr, expectSign, idx)                                                \
    do {                                                                                          \
        BigNum _a(aStr); BigNum _b(bStr);                                                         \
        int _got = (_a < _b) ? -1 : ((_a > _b) ? 1 : 0);                                          \
        if(_got == (expectSign)) {                                                                \
            kout << "  cmp[" << idx << "] {green}[OK]{/}  (" << aStr << ") vs (" << bStr << ") = " << _got << "\n"; \
            ++g_ok;                                                                               \
        } else {                                                                                  \
            kout << "  cmp[" << idx << "] {red}[FAIL]{/}  (" << aStr << ") vs (" << bStr << ") = " << _got \
                 << " , expect " << (expectSign) << "\n";                                          \
            ++g_fail;                                                                             \
        }                                                                                         \
    } while(0)

/// 根据配置组运行二元运算测试（add/sub/mul/div/mod/pow）
template <typename G>
static void RunBinary(const G& g, const char* section)
{
    size_t n = g["A"].size();
    for(size_t i = 0; i < n; i++)
    {
        std::string a = g["A"][i].Auto();
        std::string b = g["B"][i].Auto();
        std::string e = g["E"][i].Auto();
        BigNum A(a), B(b);
        if(std::string(section) == "add")
            CHECK_CFG(A + B, a + " + " + b, e, section, i);
        else if(std::string(section) == "sub")
            CHECK_CFG(A - B, a + " - " + b, e, section, i);
        else if(std::string(section) == "mul")
            CHECK_CFG(A * B, a + " * " + b, e, section, i);
        else if(std::string(section) == "div")
            CHECK_CFG(A / B, a + " / " + b, e, section, i);
        else if(std::string(section) == "mod")
            CHECK_CFG(A % B, a + " % " + b, e, section, i);
        else if(std::string(section) == "pow")
            CHECK_CFG(Pow(A, B), a + " ^ " + b, e, section, i);
    }
}

/// 缺失键守卫：输出 FAIL 并返回是否继续
#define GUARD_SECTION(node, key, label) \
    if(!(node)[key].Exists()) { \
        kout << "  " << label << " {red}[FAIL]{/}  缺少 " << key << " 键，跳过\n"; \
        ++g_fail; \
    } else

int main()
{
    auto doc = ReadKsonFile("config/test/cfg.kson");
    auto file = doc["dbgKBIGNUM"];
    KBegin(file);
    auto bn = file["bignum"];

    if(!bn.Exists())
    {
        kout << "  bignum {red}[FAIL]{/}  缺少 bignum 键\n";
        ++g_fail;
        KEnd();
        return g_fail > 0 ? 1 : 0;
    }

    // ==================== 0. normalize（展示） ====================
    kout << "== 0. normalize(展示) ==\n";
    GUARD_SECTION(bn, "normalize", "normalize")
    {
        auto root = bn["normalize"];
        for(size_t i = 0; i < root.size(); i++)
        {
            std::string s = root[i].Str();
            kout << "  " << s << " -> " << Normalize(s) << "\n";
        }
    }

    // ==================== 1. 加法（配置驱动） ====================
    kout << "\n== 1. 加法 ==\n";
    GUARD_SECTION(bn, "add", "add")
    {
        RunBinary(bn["add"], "add");
    }

    // ==================== 2. 减法（配置驱动） ====================
    kout << "\n== 2. 减法 ==\n";
    GUARD_SECTION(bn, "sub", "sub")
    {
        RunBinary(bn["sub"], "sub");
    }

    // ==================== 3. 乘法（配置驱动） ====================
    kout << "\n== 3. 乘法 ==\n";
    GUARD_SECTION(bn, "mul", "mul")
    {
        RunBinary(bn["mul"], "mul");
    }

    // ==================== 4. 比较（配置驱动） ====================
    kout << "\n== 4. 比较 ==\n";
    GUARD_SECTION(bn, "cmp", "cmp")
    {
        auto g = bn["cmp"];
        size_t n = g["A"].size();
        for(size_t i = 0; i < n; i++)
            CHECK_CFG_CMP(g["A"][i].Str(), g["B"][i].Str(), g["E"][i].Int(), i);
    }

    // ==================== 5. ToStr（配置驱动） ====================
    kout << "\n== 5. ToStr ==\n";
    GUARD_SECTION(bn, "tostr", "tostr")
    {
        auto g = bn["tostr"];
        size_t n = g["in"].size();
        for(size_t i = 0; i < n; i++)
        {
            std::string in = g["in"][i].Str();
            CHECK_CFG_STR(BigNum(in).ToStr(), in + ".ToStr()", g["E"][i].Str(), "tostr", i);
        }
    }

    // ==================== 6. ScaleTo（配置驱动） ====================
    kout << "\n== 6. ScaleTo ==\n";
    GUARD_SECTION(bn, "scaleto", "scaleto")
    {
        auto g = bn["scaleto"];
        size_t n = g["in"].size();
        for(size_t i = 0; i < n; i++)
        {
            std::string in = g["in"][i].Str();
            size_t sc = (size_t)g["scale"][i].Int();
            std::ostringstream lbl;
            lbl << "ScaleTo(" << in << "," << sc << ")";
            CHECK_CFG_STR(ScaleTo(BigNum(in), sc).ToStr(), lbl.str(), g["E"][i].Str(), "scaleto", i);
        }
    }

    // ==================== 7. 代数恒等式（内部校验） ====================
    kout << "\n== 7. 代数恒等式 ==\n";
    {
        struct Case { const char* expr; const char* expect; };
        const Case cases[] = {
            { "7.5+2.5-3",             "7" },
            { "(2+3)*4",               "20" },
            { "2*3*4",                 "24" },
            { "10-10",                 "0" },
            { "2.5*2*2",               "10" },
            { "0.5+0.5+0.5",           "1.5" },
            { "-2*-3",                 "6" },
            { "1000000000*1000000000", "1000000000000000000" },
        };
        for(size_t i = 0; i < (sizeof(cases)/sizeof(cases[0])); i++)
        {
            // 用 BigNum 求值：按 cases[i].expr 中的表达式手写（仅覆盖已实现运算）
            BigNum r;
            if(std::string(cases[i].expr) == std::string("7.5+2.5-3"))
                r = BigNum("7.5") + BigNum("2.5") - BigNum("3");
            else if(std::string(cases[i].expr) == std::string("(2+3)*4"))
                r = (BigNum("2") + BigNum("3")) * BigNum("4");
            else if(std::string(cases[i].expr) == std::string("2*3*4"))
                r = BigNum("2") * BigNum("3") * BigNum("4");
            else if(std::string(cases[i].expr) == std::string("10-10"))
                r = BigNum("10") - BigNum("10");
            else if(std::string(cases[i].expr) == std::string("2.5*2*2"))
                r = BigNum("2.5") * BigNum("2") * BigNum("2");
            else if(std::string(cases[i].expr) == std::string("0.5+0.5+0.5"))
                r = BigNum("0.5") + BigNum("0.5") + BigNum("0.5");
            else if(std::string(cases[i].expr) == std::string("-2*-3"))
                r = BigNum("-2") * BigNum("-3");
            else if(std::string(cases[i].expr) == std::string("1000000000*1000000000"))
                r = BigNum("1000000000") * BigNum("1000000000");
            CHECK_CFG(r, cases[i].expr, cases[i].expect, "ident", i);
        }
    }

    // ==================== 8. RandBigNum 随机数（往返 + 范围 + 符号） ====================
    kout << "\n== 8. RandBigNum（往返）==\n";
    for(size_t i = 0; i < 10; i++)
    {
        size_t iMin = i % 5, iMax = iMin + 1 + (i * 7) % 30;
        size_t dMin = i % 3, dMax = dMin + (i % 4);
        int sign = (int)(i % 3); // 0随机 / 1全正 / 2全负
        BigNum r = RandBigNum({iMin,iMax},{dMin,dMax},sign);
        std::string s = r.ToStr();
        // 符号校验
        bool signOk;
        if(sign == 1)      signOk = (r >= BigNum("0"));                                   // 全正: 0或正
        else if(sign == 2) signOk = (r == BigNum("0")) || (r < BigNum("0"));              // 全负: 0或负
        else               signOk = true;                                                  // 随机不校验符号
        bool ok = (BigNum(s) == r) && signOk;
        if(!ok) { kout << "  RandBigNum[" << i << "] {red}[FAIL]{/}  " << s << "\n"; ++g_fail; }
        else    { kout << "  RandBigNum[" << i << "] {green}[OK]{/}  " << s << "\n"; ++g_ok; }
    }
    // 全负数校验（sign=2 时若非0必须为负）
    {
        bool allNeg = true;
        for(size_t i = 0; i < 20; i++)
        {
            BigNum r = RandBigNum({1,5},{0,3},2);
            if(r != BigNum("0") && !(r < BigNum("0"))) allNeg = false;
        }
        if(allNeg) { kout << "  RandBigNum sign=2 全负 {green}[OK]{/}\n"; ++g_ok; }
        else       { kout << "  RandBigNum sign=2 全负 {red}[FAIL]{/}\n"; ++g_fail; }
    }
    {
        BigNum z = RandBigNum({0,0},{0,0},0);
        if(z == BigNum("0")) { kout << "  RandBigNum(0,0) {green}[OK]{/}\n"; ++g_ok; }
        else                 { kout << "  RandBigNum(0,0) {red}[FAIL]{/}  got " << z << "\n"; ++g_fail; }
    }

    // ==================== 9. istream/ostream ====================
    kout << "\n== 9. operator>> / operator<< ==\n";
    {
        const char* inputs[] = {"123", "-456.789", "0", "1e3",
                                "999999999999999999999999", "-0.000000001"};
        for(auto& s : inputs)
        {
            std::istringstream iss(s);
            BigNum v;
            iss >> v;
            if(v == BigNum(s)) { kout << "  \"" << s << "\" {green}[OK]{/}  -> " << v << "\n"; ++g_ok; }
            else               { kout << "  \"" << s << "\" {red}[FAIL]{/}  -> " << v << "\n"; ++g_fail; }
        }
        std::ostringstream oss;
        oss << (BigNum("-1.5") * BigNum("2")) << " " << (BigNum("1.5") + BigNum("1.50"))
            << " " << (BigNum("0") * BigNum("5")) << " " << BigNum("1000000000")
            << " " << (BigNum("0.001") * BigNum("0.001"));
        std::string s = oss.str();
        if(s == "-3 3 0 1000000000 0.000001") { kout << "  ostream 输出 {green}[OK]{/}\n"; ++g_ok; }
        else                                  { kout << "  ostream 输出 {red}[FAIL]{/}  got \"" << s << "\"\n"; ++g_fail; }
    }

    // ==================== 10. Abs 系列独立函数（含大数） ====================
    kout << "\n== 10. AbsAdd / AbsSub / AbsMul ==\n";
    CHECK_CFG(AbsAdd(BigNum("-123456789012345678901.5"), BigNum("0.5")),
              "AbsAdd(-123456789012345678901.5, 0.5)", "123456789012345678902", "absadd", 0);
    CHECK_CFG(AbsSub(BigNum("1000000000"), BigNum("0.000000001")),
              "AbsSub(1000000000, 0.000000001)", "999999999.999999999", "abssub", 0);
    CHECK_CFG(AbsMul(BigNum("999999999.5"), BigNum("999999999.5")),
              "AbsMul(999999999.5, 999999999.5)", "999999999000000000.25", "absmul", 0);

    // ==================== 11. inf/nan 特殊状态（配置驱动） ====================
    kout << "\n== 11. inf/nan 特殊状态 ==\n";
    GUARD_SECTION(bn, "inf_nan", "inf_nan")
    {
        auto g = bn["inf_nan"];

        // 11.1 构造 / ToStr（大小写不敏感，KSON 关键字自动解析为 BigNum 状态）
        kout << "  11.1 构造/ToStr\n";
        if(g["tostr"].Exists())
        {
            auto T = g["tostr"];
            size_t n = T["in"].size();
            for(size_t i = 0; i < n; i++)
            {
                BigNum v = BigNum(T["in"][i].Auto());
                std::string got = v.ToStr();
                std::string exp = BigNum(T["E"][i].Auto()).ToStr();
                if(got == exp) { kout << "    tostr[" << i << "] {green}[OK]{/}  " << got << "\n"; ++g_ok; }
                else { kout << "    tostr[" << i << "] {red}[FAIL]{/}  got \"" << got << "\" , expect \"" << exp << "\"\n"; ++g_fail; }
            }
        }

        // 11.2 字符串构造（inf/-inf/nan 大小写不敏感 + 首尾空白容忍）
        kout << "  11.2 字符串构造\n";
        {
            struct Case { const char* in; const char* exp; };
            const Case cases[] = {
                { "inf",       "inf"   },
                { "Inf",       "inf"   },
                { "INF",       "inf"   },
                { "+inf",      "inf"   },
                { "-inf",      "-inf"  },
                { "-INF",      "-inf"  },
                { "nan",       "nan"   },
                { "NaN",       "nan"   },
                { "NAN",       "nan"   },
                { "  -inf  ",  "-inf"  },
                { "   NAN  ",  "nan"   },
                { "1.5",       "1.5"   }, // 普通数字不受影响
            };
            for(size_t i = 0; i < (sizeof(cases)/sizeof(cases[0])); i++)
            {
                std::string got = BigNum(cases[i].in).ToStr();
                if(got == cases[i].exp) { kout << "    \"" << cases[i].in << "\" -> {green}[OK]{/}  " << got << "\n"; ++g_ok; }
                else { kout << "    \"" << cases[i].in << "\" -> {red}[FAIL]{/}  got \"" << got << "\" , expect \"" << cases[i].exp << "\"\n"; ++g_fail; }
            }
        }

        // 11.3 比较（IEEE-754：NaN 参与比较恒为 0，+inf > 一切 > -inf）
        kout << "  11.3 比较\n";
        if(g["cmp"].Exists())
        {
            auto C = g["cmp"];
            size_t n = C["A"].size();
            for(size_t i = 0; i < n; i++)
            {
                BigNum a = BigNum(C["A"][i].Auto());
                BigNum b = BigNum(C["B"][i].Auto());
                int got = (a < b) ? -1 : ((a > b) ? 1 : 0);
                long long exp = C["E"][i].Int();
                if(got == exp) { kout << "    cmp[" << i << "] {green}[OK]{/}  " << a << " vs " << b << " = " << got << "\n"; ++g_ok; }
                else { kout << "    cmp[" << i << "] {red}[FAIL]{/}  " << a << " vs " << b << " = " << got << " , expect " << exp << "\n"; ++g_fail; }
            }
        }

        // 11.4 运算传播（加 / 减 / 乘）
        kout << "  11.4 运算传播\n";
        const char* ops[] = { "add", "sub", "mul" };
        const char sym[]  = { '+', '-', '*' };
        for(size_t o = 0; o < 3; o++)
        {
            const char* sec = ops[o];
            if(!g[sec].Exists()) continue;
            auto S = g[sec];
            size_t n = S["A"].size();
            for(size_t i = 0; i < n; i++)
            {
                BigNum a = BigNum(S["A"][i].Auto());
                BigNum b = BigNum(S["B"][i].Auto());
                BigNum r = (o == 0) ? (a + b) : ((o == 1) ? (a - b) : (a * b));
                std::string got = r.ToStr();
                std::string exp = BigNum(S["E"][i].Auto()).ToStr();
                if(got == exp) { kout << "    " << sec << "[" << i << "] {green}[OK]{/}  " << a << " " << sym[o] << " " << b << " = " << got << "\n"; ++g_ok; }
                else { kout << "    " << sec << "[" << i << "] {red}[FAIL]{/}  " << a << " " << sym[o] << " " << b << " = " << got << " , expect " << exp << "\n"; ++g_fail; }
            }
        }
    }

    // ==================== 12. 除法/取模/幂运算（配置驱动） ====================
    kout << "\n== 12. 除法/取模/幂运算 ==\n";
    GUARD_SECTION(bn, "div", "div") { RunBinary(bn["div"], "div"); }
    GUARD_SECTION(bn, "mod", "mod") { RunBinary(bn["mod"], "mod"); }
    GUARD_SECTION(bn, "pow", "pow") { RunBinary(bn["pow"], "pow"); }

    // ==================== 13. type() 类型描述（配置驱动） ====================
    kout << "\n== 13. type() ==\n";
    GUARD_SECTION(bn, "type", "type")
    {
        auto g = bn["type"];
        size_t n = g["in"].size();
        for(size_t i = 0; i < n; i++)
        {
            // 用 Auto()：引号字符串 "inf"/"-inf"/"nan" 会被 KSON 自动转为 BigNum 节点，
            // Str() 会触发类型不匹配崩溃，Auto() 对字符串与 BigNum 节点均返回文本
            std::string in = g["in"][i].Auto();
            CHECK_CFG_STR(BigNum(in).type(), "BigNum(" + in + ").type()", g["E"][i].Auto(), "type", i);
        }
    }

    // ==================== 14. Root 开根（配置驱动） ====================
    kout << "\n== 14. Root 开根 ==\n";
    GUARD_SECTION(bn, "root", "root")
    {
        auto g = bn["root"];
        size_t n = g["A"].size();
        for(size_t i = 0; i < n; i++)
        {
            std::string a = g["A"][i].Auto();
            std::string b = g["B"][i].Auto();
            std::string e = g["E"][i].Auto();
            CHECK_CFG(Root(BigNum(a), BigNum(b)), "Root(" + a + ", " + b + ")", e, "root", i);
        }
    }

    // ==================== 15. 与其他算术类型运算（重载） ====================
    kout << "\n== 15. 与其他算术类型运算 ==\n";
    // BigNum op 算术类型（右值）
    CHECK_CFG(BigNum("5") + 3,      "5 + 3(int)",        "8",   "arith", 0);
    CHECK_CFG(BigNum("5.5") - 2,    "5.5 - 2(int)",      "3.5", "arith", 1);
    CHECK_CFG(BigNum("5") * 2.5,    "5 * 2.5(double)",   "12.5","arith", 2);
    CHECK_CFG(BigNum("10") / 4,     "10 / 4(int)",       "2.5", "arith", 3);
    CHECK_CFG(BigNum("-5") + 2,     "-5 + 2(int)",       "-3",  "arith", 4);
    // 算术类型 op BigNum（右值反向）
    CHECK_CFG(3 + BigNum("5"),      "3(int) + 5",        "8",   "arith", 5);
    CHECK_CFG(10 - BigNum("3.5"),   "10(int) - 3.5",     "6.5", "arith", 6);
    CHECK_CFG(2.5 * BigNum("4"),    "2.5(double) * 4",   "10",  "arith", 7);
    CHECK_CFG(7 / BigNum("2"),      "7(int) / 2",        "3.5", "arith", 8);
    // 变量形式
    {
        int i = 2; double d = 1.5; long L = 100; float f = 0.5f;
        CHECK_CFG(BigNum("3") + i,   "3 + i(int=2)",      "5",   "arith", 9);
        CHECK_CFG(d + BigNum("2.5"), "d(double=1.5)+2.5", "4",   "arith", 10);
        CHECK_CFG(L - BigNum("99"),  "L(long=100)-99",    "1",   "arith", 11);
        CHECK_CFG(BigNum("10") * f,  "10 * f(float=0.5)", "5",   "arith", 12);
        CHECK_CFG(BigNum("100") / i, "100 / i(int=2)",    "50",  "arith", 13);
    }

    // ==================== 结论 ====================
    kout << "\n----------------------------------------\n";
    kout << "  {green}[OK]{/} " << g_ok << " 项通过\n";
    if(g_fail == 0)
        kout << "\n{green}[ALL PASS] BigNum 配置驱动测试通过{/}\n";
    else
        kout << "\n{red}[" << g_fail << " FAIL] BigNum 配置驱动测试有失败项{/}\n";
    KEnd();
    return g_fail > 0 ? 1 : 0;
}