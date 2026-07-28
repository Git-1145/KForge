#include<utility/KF.hpp>
using namespace std;
int main(){    // 主函数定义
    kbegin(L"测试","this is a test","test");
    kopen("cfg.txt");
    auto file=kread("bob");
    auto res=koption(file.value("[sort][title]").str(),knvtos(file.value("[sort][options]").arr()));
    kout << res << kend;
    kpause();
    auto file2=kread("[bob][bob]");
    kout << file2.value("msg[0]").str() << kend;
    //kout << res << kend;
    kprogramend();
    return 0;
}