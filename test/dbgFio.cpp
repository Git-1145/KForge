#include "../base/KF.hpp"
int main() {
    try {
        fio::open("D:/kForge/test/cfg.txt");
        auto File = KF::FIO::read("kf");
        std::cout << "kf is_object=" << File.is_object() << " is_array=" << File.is_array() << "\n";
        auto bob = File.get("bob");
        std::cout << "alice is_object=" << bob.is_object() << " is_array=" <<bob.is_array() << "\n";
        auto msg = bob.get("msg");
        std::cout << "msg is_object=" << msg.is_object() << " is_array=" << msg.is_array() << "\n";
        std::cout << File.value("[alice][msg]").size() << '\n';
        auto el = msg[0].str();
        std::cout << "msg[0]=" << el<<"\n";
    } catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    system("pause");
    return 0;
}
