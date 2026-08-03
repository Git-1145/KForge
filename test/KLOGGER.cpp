#include "../base/KF.hpp"
using namespace std;
int main()
{
    KF::KLOGGER::Error(TEST_ERROR, "作者有病");
    KF::KLOGGER::Warning(TEST_WARN, "作者可能有病");
    KF::KLOGGER::Info(TEST_INFO, "作者没病");
    KF::KLOGGER::Fatal(TEST_FATAL, "作者病入膏肓了");
    system("pause");
    return 0;
}