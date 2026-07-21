#include<iostream>
#include<chrono>
#include<windows.h>
#include<vector>
#include<string>
#include<fstream>
#include<unordered_map>
#include<string_view>
#include<cmath>
#include<algorithm>
#include<array>
namespace kf
{
    class Info
    {
        public:
            bool load(const std::string& filename)
            {
                std::ifstream file(filename);
                if (!file)
                    return false;
                std::string line;
                std::string key;
                while (std::getline(file, line))
                {
                    if (line.empty())
                        continue;
                    auto pos = line.find('=');
                    if (pos != std::string::npos)
                    {
                        key = line.substr(0, pos);
                        data_[key] = line.substr(pos + 1);
                    }
                    else if (!key.empty())
                    {
                        data_[key] += '\n';
                        data_[key] += line;
                    }
                }
                return true;
            }

    const std::string& operator[](const std::string& key) const
    {
        return data_.at(key);
    }
private:
    std::unordered_map<std::string, std::string> data_;
};

    class IO
    {
        public:
            template<typename T>
            IO& operator>>(T&value)
            {
                std::cout << "> ";
                std::cin >> value;
                return *this;
            }
            template<typename T>
            IO& operator>>(std::vector<T>& vec)
            {
                std::cout << "> ";
                for(auto& value : vec)
                    std::cin >> value;
                return *this;
            }
            template<typename T>
            IO& operator<<(const T& output)
            {
                if(line_begin)
                {
                    std::cout << "- ";
                    line_begin=false;
                }
                std::cout << output;
                return *this;
            }
            IO& operator<<(std::ostream& (*manip)(std::ostream&))
            {
                manip(std::cout);
                if(manip == &std::endl<char, std::char_traits<char>>)
                    line_begin = true;
                return *this;
            }
            private:
                bool line_begin=true;
    };
    inline IO in;
    inline IO out;
    void pause(){std::cout << std::endl;system("pause");}
    class timer
    {
        private:
            using Clock = std::chrono::high_resolution_clock;
            std::string name_;
            std::chrono::time_point<Clock> begin_;
        public:
            explicit timer(std::string_view name="") : name_(name),begin_(Clock::now()){}
            void printTimer(std::string_view name="") const
            {
                auto end = Clock::now();
                std::cout <<std::endl<<'['<< name_ <<"] " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin_).count() << "ms" << std::endl;
            }
    };
    namespace cli
    {
        void clearScreen(){system("cls");}

        void begin(std::string_view title=""){
            std::cout << "======================================================\n                       " << title << "\n======================================================\n";
            system(("title "+std::string(title)).c_str());}
        void end()
        {
            std::cout << "======================================================";
        }

        template<size_t N>
        size_t option(std::string_view title,const std::array<std::string_view,N>& options)
        {
            std::cout << title << '\n';
            for(size_t i=0;i<N;i++)
                std::cout << "  " << i+1 << ". " << options[i] << '\n';
            long long choice;
            while(true)
            {
                kf::in >> choice;
                if(choice >=1 && choice<=static_cast<long long>(N))
                    return choice;
                std::cout << "invalid!\n";
            }
        }
    }

}
#define kin kf::in
#define kout kf::out