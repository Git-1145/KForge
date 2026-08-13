#include "base/KF.hpp"
int main()
{
    KCLI::KBegin("KForge Utility", "Random number generator","Git-1145","2026-08-13");
    for(size_t i = 0 ;i<100;i++)
        KCLI::kout << KUTIL::RandInt(0, 100000000) << "\n";
    KCLI::KEnd();
}
