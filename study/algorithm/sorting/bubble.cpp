#include"../../../base/kf.hpp"
using namespace std;
size_t n;
vector<int> ret;
vector<int> bubble(vector<int> arr,bool sortmethod)
{
    vector<int> res = arr;
    for(size_t i=0;i<n;i++)
    {
        for(size_t j=0;j<n-i-1;j++)
        {
            if(sortmethod)
            {
                if(res[j]<res[j+1])
                    swap(res[j],res[j+1]);
            }
            else
                if(res[j]>res[j+1])
                    swap(res[j],res[j+1]);
        }
    }
    return res;
}
int main()
{
    cli::programBegin(L"冒泡排序","cfg.kf","[sorting][bubble]");
    auto res = cli::option("[sorting][bubble][opt]");
    kout << "input array size:" << kend;
    kin >> n;
    vector<int> arr(n);
    kout << "input array:" << kend;
    for(auto& i:arr) cin >> i;
    if(res==1)
        ret=bubble(arr,false);
    else 
        ret=bubble(arr,true);
    kout << endl << "the result is:" << kend;
    for(auto& i:ret)
    {
        kout << i << " ";
    }
    kout << kend;
    cli::programEnd();
    return 0;
}