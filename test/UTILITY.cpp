#include "../base/KF.hpp"
int main()
{
    auto file = KSON::ReadKsonFile("cfg.kson");
    KCLI::KBegin(file);
    for(size_t i = 0 ;i<100;i++)
        KCLI::kout << KUTIL::RandInt(0, 100000000) << "\n";
    KCLI::KEnd();
}
