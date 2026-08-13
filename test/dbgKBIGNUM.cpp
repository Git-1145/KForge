#include "base/KF.hpp"
using namespace KBIGNUM;
using namespace KCLI;
using namespace KSON;
using namespace KTIMER;
int main()
{
    auto doc = ReadKsonFile("config/test/cfg.kson");
    auto file = doc["dbgKBIGNUM"];
    auto ksonDoc = doc["dbgKSON"];
    KBegin(file);

    /// @brief 测试 Normalize 函数
    auto root1 = file["bignum"]["normalize"];
    for(size_t i = 0; i < root1.size(); i++)
    {
        std::string str = root1[i].Str();
        AddTimer("Normalize", TimeUnit::us);
        kout << str << " -> " << Normalize(str) << "\n";
        PrintTimer("Normalize");
        DeleteTimer("Normalize");
    }

    kout << "\n\n\n\n";

    /// @brief 测试 ToBig + ToStr 往返
    auto root2 = file["bignum"]["to_big"];
    for(size_t i = 0; i < root2.size(); i++)
    {
        std::string str = root2[i].Str();
        std::string normalized = Normalize(str);
        // Normalize 给非零正数加了 '+' 前缀，ToStr 不带 '+'，比较时去掉
        if(!normalized.empty() && normalized[0] == '+')
            normalized = normalized.substr(1);

        AddTimer("tobignum", TimeUnit::us);
        BigNum big(str);
        PrintTimer("tobignum");
        DeleteTimer("tobignum");

        std::string tostr = big.ToStr();
        kout << str << " -> \n";
        kout << "  normalized = " << normalized << "\n";
        kout << "  isneg      = " << big.isneg << "\n";
        kout << "  scale      = " << big.scale << "\n";
        kout << "  limbs      = ";
        for(auto &x : big.limbs)
            kout << x << " ";
        kout << "\n";
        kout << "  ToStr      = " << tostr << "\n";
        if(tostr == normalized)
            kout << "  [OK] round-trip\n\n";
        else
            kout << "  [FAIL] round-trip: expected " << normalized << "\n\n";
    }

    /// @brief 测试 AbsAdd AbsSub AbsCmp 函数
    auto root3 = file["bignum"]["absCaculate"];
    for(size_t i = 0; i < root3["A"].size(); i++)
    {
        AddTimer("absCaculate", TimeUnit::us);
        BigNum A(root3["A"][i].Auto());
        BigNum B(root3["B"][i].Auto());
        kout << '|'<< A << " + " << B << '|' << " -> " << AbsAdd(A, B) << "\n";
        kout << '|'<< A << " - " << B << '|' <<" -> " << AbsSub(A, B) << "\n";
        kout << '|'<< A << " * " << B << '|' << " -> " << AbsMul(A, B) << "\n";
        kout << A << " + " << B << " -> " << A + B << "\n";
        kout << A << " - " << B << " -> " << A - B << "\n";
        kout << A << " * " << B << " -> " << A * B << "\n";
        auto cmp = AbsCmp(A, B);
        kout << A << " cmp " << B << " -> " << cmp;
        kout << " (" << (cmp > 0 ? "A>B" : (cmp == 0 ? "A=B" : "A<B")) << ")\n";
        PrintTimer("absCaculate");
        DeleteTimer("absCaculate");
    }

    /// @brief 测试 error_cases.normalize_errors (错误字符串)
    auto root5 = ksonDoc["error_cases"]["normalize_errors"];
    for(size_t i = 0; i < root5.size(); i++)
    {
        std::string str = root5[i].Str();
        AddTimer("Normalize", TimeUnit::us);
        kout << str << " -> " << Normalize(str) << "\n";
        PrintTimer("Normalize");
        DeleteTimer("Normalize");
    }

    /// @brief 测试科学计数法 和 大数B
    auto root4 = ksonDoc["kson_bignum"];
    for(size_t i = 0; i < root4.size(); i++)
    {
        auto a = root4[i].Auto();
        kout << a << "\n";
    }
    /// @brief 测试随机数
    for(size_t i = 0; i < 10; i++)
    {
        kout << RandBigNum(true,100,2) << "\n";
    }
    KEnd();
}
