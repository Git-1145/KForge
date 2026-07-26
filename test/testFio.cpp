#include "../utility/kf.hpp"
int main()
{
    try
    {
        KF::UTI::FIO::open("D:/kForge/test/cfg.txt");

        KF::UTI::FIO::nodeView File = KF::UTI::FIO::read("kf");

        // 数组下标路径访问
        std::string a = File.value("[alice][msg][1]").str();
        std::string b = File.value("[sort][title]").str();
        std::cout << "alice msg[1] = " << a << "\n";
        std::cout << "sort name = " << b << "\n";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    system("pause");
    return 0;
}
