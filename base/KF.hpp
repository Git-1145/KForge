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
inline constexpr std::string_view CLI_SEPERATOR = "####################################";//========== title ==========这种=的数量

inline constexpr char FIO_OBJECT_BEG = '{';
inline constexpr char FIO_ARRAY_BEG = '(';
inline constexpr char FIO_OBJECT_END = '}';
inline constexpr char FIO_ARRAY_END = ')';
inline constexpr char FIO_STRING = '"';
inline constexpr char FIO_KEY_BEG = '[';
inline constexpr char FIO_KEY_END = ']';
inline constexpr char FIO_SEPERATOR = '=';
inline constexpr char FIO_COMMA = ',';

namespace KF
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
            nodeView at(size_t idx) { return nodeView(&ptr->arr[idx]); }//返回数组中的节点
            nodeView operator[](size_t index);
            nodeView value(const std::string& path);//路径解析

            //面向对象的
            bool is_string() const { return ptr->is_string(); }
            bool is_ll() const { return ptr->is_ll(); }
            bool is_object() const { return ptr->is_object(); }
            bool is_array() const  { return ptr->is_array();  }
            std::vector<node>& arr() { return ptr->arr; }//返回引用，杜绝拷贝vector<node>
            ///@attention 如果不加&就会implicitly declared as deleted because 'KF::FIO::node' declares a move constructor or move assignment operator   
            std::string str()    { return std::get<std::string>(ptr->val);}//节点的值(string类型)
            long long i64()          { return std::get<long long>(ptr->val);}//节点的值(long long类型)
            double f64()          { return std::get<double>(ptr->val);}//节点的值(double类型)
            size_t size() const { return ptr->arr.size(); }//数组大小
        };
        std::vector<std::string> nVectoStrVec(std::vector<node>& vec);
        void open(const std::string& path);
        nodeView read(const std::string& topKey);
    }
    /// @brief console面板
    namespace CLI
    {
        /// @brief CLI 输入输出
        using OstreamManip = std::ostream& (*)(std::ostream&);
        using IstreamManip = std::istream& (*)(std::istream&);
        constexpr OstreamManip endl_cptr = std::endl<char, std::char_traits<char>>;
        struct EndTag{};
        inline constexpr EndTag KEND{};
        class KIO_OUT
        {
        private:
            bool new_group    = true;// 是否需要打印头部 "- "
            bool need_indent  = false;// 是否需要打印缩进 "  "
        public:
            template<typename T>
            KIO_OUT& operator<<(const T& in)
            {
                if (new_group){std::cout << CLI_KOUT_PREFIX << ' ';new_group = false;}
                else if (need_indent){std::cout << "  ";need_indent = false;}
                std::cout << in;return *this;
            }
            KIO_OUT& operator<<(OstreamManip manip)
            {
                manip(std::cout);
                if (manip == endl_cptr && !new_group)
                    need_indent = true;
                return *this;
            }
            KIO_OUT& operator<<(EndTag)
            {
                endl_cptr(std::cout);//自动换行
                new_group   = true;
                need_indent = false;
                return *this;
            }
        };
        class KIO_IN
        {
        private:
            bool needPrompt = true;

            struct Proxy
            {
                KIO_IN& parent;
                explicit Proxy(KIO_IN& p) : parent(p) {}

                ~Proxy(){parent.needPrompt = true;}
                Proxy(Proxy&&) noexcept = default;
                template<typename T>
                Proxy& operator>>(T& out)
                {
                    parent.input(out);
                    return *this;
                }
                Proxy& operator>>(IstreamManip manip)
                {
                    parent.handle_imanip(manip);
                    return *this;
                }
            };
            template<typename T>
            void input(T& out)
            {
                if (needPrompt)
                {
                    std::cout << CLI_KIN_PREFIX << ' ';
                    needPrompt = false;
                }
                std::cin >> out;
            }
            void handle_imanip(IstreamManip manip){manip(std::cin);}
        public:
            template<typename T>
            Proxy operator>>(T& out)
            {
                Proxy p(*this);
                p >> out;
                return p;
            }
            Proxy operator>>(IstreamManip manip)
            {
                Proxy p(*this);
                p >> manip;
                return p;
            }
        };
        // 全局实例
        inline KIO_OUT KOUT;
        inline KIO_IN  KIN;

        /// @brief 选项
        size_t option(std::string path);
        /// @brief 基础设施 如清屏 暂停 开始 结束
        void clear();
        void pause();
        void programEnd();
        void programBegin(std::wstring ConsoleTitle,std::string title ,std::string description);
    }
    namespace KTIMER
    {
        enum class KTimeUnit
        {
            us,
            ms,
            s
        };
        class KTimer
        {
        private:
            using Clock = std::chrono::steady_clock;
            using TimePoint = std::chrono::time_point<Clock>;
            std::string name_;
            KTimeUnit unit_;
            TimePoint start_point_;
            std::chrono::nanoseconds accumulated_{0};
            bool running_{false};
        public:
            explicit KTimer(std::string name = "Timer", KTimeUnit unit = KTimeUnit::ms);
            void start();
            void pause();
            void clear();
            void setUnit(KTimeUnit u);
            void setName(std::string n);
            double getTime() const;
            void print() const;
        private:
            std::string unitStr() const;
        };
        ///@brief 计时器管理器：管理多个命名计时器
        class KTimerManager
        {
        private:
            std::unordered_map<std::string, KTimer> timers_;
        public:
            void create(const std::string& name, KTimeUnit unit);
            // 获取计时器引用
            KTimer& get(const std::string& name);
            bool exists(const std::string& name);
            // 删除计时器
            void remove(const std::string& name);
        };


    }
    namespace UTILITY
    {
        /// @brief 数学工具
        bool isPosInt(std::string str);
    }
}
/// @brief 简写作用
namespace cli = KF::CLI;
namespace fio = KF::FIO;
namespace timer = KF::KTIMER;
namespace uti = KF::UTILITY;
inline auto& kout = KF::CLI::KOUT;
inline auto& kin = KF::CLI::KIN;
inline auto& kend = KF::CLI::KEND;