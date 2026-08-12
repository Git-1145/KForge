#include "base/KF.hpp"
int main()
{
    KCLI::KBegin("KForge Utility", "Random number generator");
    for(size_t i = 0 ;i<100;i++)
        KCLI::kout << KUTIL::RandInt(0, 100000000) << "\n";
    KCLI::KEnd();
}
