#include "KF.hpp"
namespace KF::UTI
{
    namespace KMATH
    {
        bool isPosInt(std::string str)
        {
            for (char c : str)
                if(!isdigit(c) && c!='\0')
                    return false;
            return true;
        }
        
    }
}