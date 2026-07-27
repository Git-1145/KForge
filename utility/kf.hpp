#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <variant>
#include <memory>
#include <windows.h>
#include <chrono>

inline constexpr char CLI_KOUT_PREFIX = '-';
inline constexpr char CLI_KIN_PREFIX = '>';
inline constexpr size_t CLI_BEGIN_LEN = 25;//========== title ==========这种=的数量

inline constexpr char FIO_OBJECT_BEG = '{';
inline constexpr char FIO_ARRAY_BEG = '(';
inline constexpr char FIO_OBJECT_END = '}';
inline constexpr char FIO_ARRAY_END = ')';
inline constexpr char FIO_STRING = '"';
inline constexpr char FIO_KEY_BEG = '[';
inline constexpr char FIO_KEY_END = ']';
inline constexpr char FIO_SEPERATOR = '=';
inline constexpr char FIO_COMMA = ',';

namespace KF::UTI
{
    namespace FIO
    {
        enum class valueType
        {
            Empty,
            Str,
            I64,
            F64
        };
        struct node;
        using nodePtr = std::unique_ptr<node>;
        using objMap = std::unordered_map<std::string, nodePtr>;
        struct node//节点
        {
            objMap obj;//{} 孩子有哪些
            std::vector<node> arr;//()数组 node退化成val 
            valueType type=valueType::Empty;
            std::variant<std::string, long long, double> val;//值
            //构造
            node()=default;
            node(node&&)=default;
            node& operator= (node&&)=default;
            //查询类型
            bool is_object() const { return !obj.empty(); }
            bool is_array()  const { return !arr.empty(); }
            bool is_string() const { return type == valueType::Str; }
            bool is_ll() const { return type == valueType::I64;}
            bool is_db() const { return type == valueType::F64;}
            
        };
        
        struct nodeView//查询节点
        {
            node* ptr;
            nodeView(node *p) : ptr(p) {}
            bool isExist(const std::string &key) const { return ptr->obj.count(key) != 0; }//查找是否有这个孩子节点
            nodeView get(const std::string& key);//返回孩子节点
            nodeView operator[](size_t index);
            nodeView value(const std::string& path);//路径解析

            //面向对象的
            bool is_string() const { return ptr->is_string(); }
            bool is_ll() const { return ptr->is_ll(); }
            bool is_object() const { return ptr->is_object(); }
            bool is_array() const  { return ptr->is_array();  }
            std::vector<node>& arr() { return ptr->arr; }//返回引用，杜绝拷贝vector<node>
            ///@attention 如果不加&就会implicitly declared as deleted because 'KF::UTI::FIO::node' declares a move constructor or move assignment operator   
            std::string str()    { return std::get<std::string>(ptr->val);}//节点的值(string类型)
            long long i64()          { return std::get<long long>(ptr->val);}//节点的值(long long类型)
            double f64()          { return std::get<double>(ptr->val);}//节点的值(double类型)
            size_t size() const { return ptr->arr.size(); }//数组大小
        };

        void open(const std::string& path);
        nodeView read(const std::string& topKey);
    }
    namespace CLI
    {
        void begin(std::string& title, std::string& description);
        void end();
        void pause();
        void clear();
        size_t option(std::string& title,std::vector<std::string> options);
        class KIO_OUT
        {
            private:
                bool needPrefix=true;
            public:
            template<typename T>
            KIO_OUT& operator<<(const T& in)
            {
                if(needPrefix)
                {
                    std::cout << CLI_KOUT_PREFIX << ' ';
                    needPrefix=false;
                }
                std::cout << in;
                return *this;
            }
            KIO_OUT& operator<<(std::ostream& (*manip)(std::ostream&))//用于支持标准输出流操纵符
            {
                manip(std::cout);
                if(manip==static_cast<std::ostream&(*)(std::ostream&)>(std::endl))
                    needPrefix=true;
                return *this;        
            }
        };
        class KIO_IN
        {
        private:
            bool needPrefix = true;
        public:
            template<typename T>
            KIO_IN& operator>>(T& out)
            {
            if (needPrefix)
            {
                std::cout << CLI_KIN_PREFIX << ' ';
                needPrefix = false;
            }
            std::cin >> out;
            needPrefix = true;
            return *this;
            }
            KIO_IN& operator>>(std::istream& (*manip)(std::istream&))
            {
                manip(std::cin);
                return *this;
            }
        };
        inline KIO_OUT KOUT; 
        inline KIO_IN KIN;
    }
}
/// @brief 简写作用
#define kout KF::UTI::CLI::KOUT
#define kin KF::UTI::CLI::KIN