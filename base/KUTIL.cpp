#include "KF.hpp"
namespace KF 
{
    namespace KUTIL
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        sdlimb RandInt(sdlimb min, sdlimb max)
        {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(gen);
        }
        sdlimb Pow10(sdlimb n)
        {
            sdlimb res = 1;
            for (sdlimb i = 0; i < n; i++)
                res *= 10;
            return res;
        }
    }
}