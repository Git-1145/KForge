#include "../base/KF.hpp"
int main()
{
    std::string raw = KFIO::ReadFileRaw("cfg.kson");
    std::cout << raw << "\n\n\n\n\n\n\n\n\nI'm here:\n";
    std::string Processed = KSON::Preprocess(raw);
    std::cout << Processed << std::endl;
    std::string exit = KFIO::ReadFileRaw("exit.txt");
    return 0;
}