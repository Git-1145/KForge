#include "KF.hpp"
namespace KF::UTI
{
    namespace CLI
    {
        size_t OPTION(std::string prompt,std::vector<std::string> options)
        {
            while (true)
            {
                KOUT << prompt;
                for (size_t i = 0; i < options.size(); ++i)
                    KOUT << std::endl << i+1 << ". " << options[i];
                KOUT << std::endl << KEND;
                std::string tmp;
                KIN >> tmp;
                if(!KMATH::isPosInt(tmp))
                {
                    KOUT << tmp << KEND;
                    KOUT << std::endl << "This is not a positive integer" << std::endl << KEND;
                    continue;
                }
                else
                {
                    size_t ans = std::stoul(tmp);
                    if (ans <= options.size())
                        return ans;
                    KOUT << std::endl << "This is an invalid option" << std::endl << "It must be in the range [1," <<
                    options.size() << ']' << std::endl << KEND;
                }
            }
        }
        void CLEAR(){system("cls");}
        void PAUSE(){KOUT << std::endl << "Press ENTER to continue..." << KEND;getchar();getchar();}
        void END(){std::cout << std::endl << CLI_SEPERATOR << std::endl << "Program ended" << std::endl;PAUSE();}
        void BEGIN(std::wstring ConsoleTitle,std::string title ,std::string description)
        {
            SetConsoleTitleW(ConsoleTitle.c_str());
            std::cout << CLI_SEPERATOR << std::endl;
            std::cout << title << std::endl << std::endl << description <<std::endl<<CLI_SEPERATOR << std::endl << std::endl;
        }
    }
}