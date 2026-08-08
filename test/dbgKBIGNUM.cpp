#include "../base/KF.hpp"
using namespace KBIGNUM;
using namespace KCLI;
using namespace KSON;
using namespace KTIMER;
int main()
{
    auto file = ReadKsonFile("cfg.kson");
    KBegin(file);

    /// @brief 测试 Normalize 函数
    auto root1 = file["bignum"]["normalize"];
    for(size_t i = 0 ;i<root1.size();i++)
    {
        std::string str = root1[i].Str();
        AddTimer("Normalize",TimeUnit::us);
        kout << str << " -> " << Normalize(str) << "\n";
        PrintTimer("Normalize");
        DeleteTimer("Normalize");
    }
    auto root2 = file["bignum"]["to_big"];
    kout << "\n\n\n\n";
    /// @brief 测试 ToBig + ToStr 往返
    for(size_t i = 0 ;i<root2.size();i++)
    {
        std::string str = root2[i].Str();
        std::string normalized = Normalize(str);
        // Normalize 给非零正数加了 '+' 前缀，ToStr 不带 '+'，比较时去掉
        if(!normalized.empty() && normalized[0] == '+')
            normalized = normalized.substr(1);

        AddTimer("tobignum",TimeUnit::us);
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
    KEnd();
}
