#include<iostream>
#include<study/definitions.hpp>
using namespace std;
int main()
{
    kf::Info info;
    if(!info.load("fuck.txt"))
        cout << "fuck!";
    cout << info["title"] << endl;
    cout << info["description"] << endl;
    kf::pause();
    return 0;
}