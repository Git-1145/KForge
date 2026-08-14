#include "base/KF.hpp"
using namespace std;
using namespace KBIGNUM;
using namespace KCLI;
using namespace KUTIL;
using namespace KSON;
using namespace KTIMER;
using NUM = uint32_t;
NUM n;
vector<NUM> arr;
void BucketSort(vector<NUM>& a, size_t rule)
{
    if (a.empty()) return;
    const size_t n = a.size();
    NUM maxnum = *max_element(a.begin(), a.end());
    vector<size_t> tmp(maxnum + 1, 0);
    vector<NUM> res;
    res.reserve(n);
    AddTimer("BUCKET SORT",TimeUnit::us);
    // 计数
    for(size_t i = 0; i < n; i++)
        tmp[a[i]]++;
    if(rule == 1) // ascending
    {
        for(size_t i = 0; i < tmp.size(); i++)
            if(tmp[i] != 0)
                for(size_t j = 0; j < tmp[i]; j++)
                    res.push_back(static_cast<NUM>(i));
    }
    else // descending
    {
        for(size_t i = tmp.size()-1; i > 0; i--)
            if(tmp[i] != 0)
                for(size_t j = 0; j < tmp[i]; j++)
                    res.push_back(static_cast<NUM>(i));
    }
    PrintTimer("BUCKET SORT");
    bool out = true;
    kout << "Output result?: ";
    kin >> out;
    if(out)
    {
        kout << "result :\n";
        for(size_t i = 0; i < res.size(); i++)
            kout << res[i] << endl;
        kout << endl;
    }
}
int main()
{
    kson file = ReadKsonFile("config/algorithm/cfg.kson");
    kson main = file["Algorithm"]["Sorting"]["BucketSort"];
    KBegin(main);
    size_t rule = KOptions(main["sort_method"]);
    bool isgen=false;
    kout << "size of array: ";
    kin >> n;
    arr.clear(); // 清空，push_back 会从零开始追加
    try
    {
        arr.reserve(n); // 预分配容量，避免多次扩容
    } catch (const std::bad_alloc& e) {
        Fatal(SYSTEM_OOM, "can not reserve the vector",__FILE__,__LINE__,__FUNCTION__);
    }
    kout << "Automate generate INTEGER? {yellow}(Bool):{/}";
    kin >> isgen;
    if(isgen)
    {
        NUM iMin, iMax;
        kout << "integer digit range (min max): ";
        kin >> iMin >> iMax;
        for(size_t i = 0; i < n; i++)
            arr.push_back(RandInt(iMin, iMax));
    }
    else
    {
        kout << "input array: \n";
        for(size_t i = 0; i < n; i++)
        {
            NUM x;
            kin >> x;
            arr.push_back(x);
        }
    }
    BucketSort(arr, rule);
    kout << "Time Complexity: " << main["time_complexity"].Str() << "\n\n";
    KEnd();
}