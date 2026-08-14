#include "base/KF.hpp"
using namespace std;
using namespace KBIGNUM;
using namespace KCLI;
using namespace KSON;
using namespace KTIMER;
size_t n;
vector<BigNum> arr;
void BubbleSort(vector<BigNum>& a, size_t rule)
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
    bool out = true;
    kout << "Output result?: ";
    kin >> out;
    if(out)
    {
        kout << "result :\n";
        for(auto x : a)
            koutW << x << "\n";
        kout << endl;
    }
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
        size_t iMin, iMax, dMin, dMax;
        int sign;
        kout << "integer digit range (min max): ";
        kin >> iMin >> iMax;
        kout << "decimal digit range (min max): ";
        kin >> dMin >> dMax;
        kout << "sign (0=random 1=positive 2=negative): ";
        kin >> sign;
        for(size_t i = 0; i < n; i++)
            arr.push_back(RandBigNum({iMin,iMax},{dMin,dMax},sign));
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
    BubbleSort(arr, rule);
    kout << "Time Compelximity" << main["time_compelxity"];
    KEnd();
}