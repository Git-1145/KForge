#include "base/KF.hpp"
using namespace std;
using namespace KBIGNUM;
using namespace KCLI;
using namespace KSON;
using namespace KTIMER;
size_t n;
vector<BigNum> arr;
void bubble_sort(vector<BigNum>& a, size_t rule)
{
    AddTimer("BUBBLE SORT",TimeUnit::us);
    for(size_t i = 0; i < a.size() - 1; i++)
    {
        for(size_t j = 0; j < a.size() - i - 1; j++)
        {
            BigNum tmp;
            if((rule == 1 && a[j] > a[j + 1]) || (rule == 2 && a[j] < a[j + 1])){
                tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }
    PrintTimer("BUBBLE SORT");
    kout << "result :\n";
    for(auto x : a)
        koutW << x << "\n";
    kout << endl;
}
int main()
{
    kson file = ReadKsonFile("config/algorithm/cfg.kson");
    kson main = file["Algorithm"]["Sorting"]["BubbleSort"];
    KBegin(main);
    size_t rule = KOptions(main["sort_method"]);
    bool isgen=false;
    kout << "size of array: ";
    kin >> n;
    kout << "Automate generate BIGNUM? {yellow}(Bool):{/}";
    kin >> isgen;
    if(isgen)
    {
        size_t len;
        kout << "length of random number: ";
        kin >> len;
        for(size_t i = 0; i < n; i++)
            arr.push_back(RandBigNum(false,len,10));
    }
    else
    {
        kout << "input array: \n";
        for(size_t i = 0; i < n; i++)
        {
            BigNum x;
            kin >> x;
            arr.push_back(x);
        }
    }
    bubble_sort(arr, rule);
    KEnd();
}