#include "KF.hpp"
namespace KF
{
    namespace CLI
    {
        size_t option(std::string path)
        {
            std::vector<std::string> options = fio::nVectoStrVec(fio::read(path+"[options]").arr());
            std::string prompt=fio::read(path+"[prompt]").str();
            while (true)
            {
                KOUT << prompt;
                for (size_t i = 0; i < options.size(); ++i)
                    KOUT << std::endl << i+1 << ". " << options[i];
                KOUT << std::endl << KEND;
                std::string tmp;
                KIN >> tmp;
                if(!uti::isPosInt(tmp) && tmp.size()>=11)
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
        void clear(){system("cls");}
        void pause(){KOUT << std::endl << "Press ENTER to continue..." << KEND;getchar();getchar();}
        void programEnd(){std::cout << std::endl << CLI_SEPERATOR << std::endl << "Program ended" << std::endl;cli::pause();}
        void programBegin(std::wstring ConsoleTitle,std::string file,std::string path)
        {
            SetConsoleTitleW(ConsoleTitle.c_str());
            std::cout << CLI_SEPERATOR << std::endl;
            fio::open(file);
            /// @attention var PATH is the filepath thst the program will read ,like [kf][algo][bubble],its a object
            /// @attention 
            std::string title=fio::read(path+"[info][title]").str(),description=fio::read(path+"[info][description]").str(),
            ver=fio::read(path+"[info][version]").str(),create_time =fio::read(path+"[info][create_time]").str();
            std::cout << title << std::endl << std::endl << description <<std::endl<<std::endl
            <<"VERSION: "<< ver << std::endl << "CREATE TIME: " << create_time << std::endl << CLI_SEPERATOR << std::endl << std::endl;
        }
    }
}