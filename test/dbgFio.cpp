#include <iostream>
#include "../utility/kf.hpp"
int main() {
    try {
        KF::UTI::FIO::open("D:/kForge/test/cfg.txt");
        auto File = KF::UTI::FIO::read("kf");
        std::cout << "kf is_object=" << File.is_object() << " is_array=" << File.is_array() << "\n";
        auto bob = File.get("bob");
        std::cout << "alice is_object=" << bob.is_object() << " is_array=" <<bob.is_array() << "\n";
        auto msg = bob.get("msg");
        std::cout << "msg is_object=" << msg.is_object() << " is_array=" << msg.is_array() << "\n";
        auto el = msg[0];
        std::cout << "msg[0] ok is_object=" << el.is_object() << " is_array=" << el.is_array() << "\n";
    } catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    system("pause");
    return 0;
}
