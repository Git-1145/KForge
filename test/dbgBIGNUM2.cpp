#include "../base/KF.hpp"
using namespace KBIGNUM;
using namespace KCLI;
using namespace KSON;
using namespace KTIMER;
int main()
{
    auto file = ReadKsonFile("cfg.kson");
    KBegin(file);
    auto root1 = file["bignum"]["normalize"];
    for(size_t i = 0 ;i<root1.size();i++)
    {
        std::string str = root1[i].Str();
        AddTimer("tobignum",TimeUnit::us);
        BigNum big(str);
        kout << str << " -> \n";
        kout << "isneg = " << big.isneg << "\n";
        kout << "decimal length = " << big.scale << "\n";
        kout << "limbs = ";
        for(auto &x : big.limbs)
            kout << x << " ";
        PrintTimer("Normalize");
        DeleteTimer("Normalize");
    }
    KEnd();
}