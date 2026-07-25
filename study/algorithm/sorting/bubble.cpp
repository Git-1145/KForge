#include"definitions.hpp"
using namespace std;
int main()
{
    kf::cli::begin("bubble");
    int n,method;
    //method == 1 for descending, else for ascending
    kin >> n;
    vector<long long> arr(n);
    kin >> arr;
    kin >> method;
    kf::timer timer("bubble sort");
    for(int i=0;i<n-1;i++)
    {
        bool isSorted = false;
        for(int j=0;j<n-i-1;j++)
        {
            int a=arr[j],b=arr[j+1];
            if((a>b && method==1) || (a<b && method!=1))
            {
                a=a^b;
                b=a^b;
                a=a^b;
                arr[j]=a;
                arr[j+1]=b;
                isSorted = true;
            }
        }
        if(!isSorted)
            break;
    }
    kout<< "result: ";
    for(int i=0;i<n;i++)
        kout << arr[i] << " ";
    timer.printTimer("bubble sort");
    kf::cli::pause();
    return 0;
}