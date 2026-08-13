#pragma once
#include<vector>
#include<cmath>
#include<string>
#include<random>
#include<cstdlib>
#include<stdexcept>
#include<windows.h>
#include<functional>
#include<iomanip>
#include<memory>
#include<sstream>
#include<limits>
#include<cerrno>
#include<chrono>
#include<fstream>
#include<iostream>
#include<unordered_map>
#include<variant>
#include<cstring>
#include<type_traits>
using Code = uint32_t;
/**
 * @file KF.hpp
 * @brief KForge 所有基础模块的声明文件
 * @version 1.0.0
 * @date 2026-08-13
 * @author Git-1145
 * @usage #include "KF.hpp"
 * @usage using namespace xxx; // xxx 为模块名
**/


#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
// 日志宏：自动捕获调用位置（文件名:行号:函数名）
// 用法：KLOG_ERROR(code, "extra")  替代  KLOG::Error(code, "extra")
/// @attention C++17 无法用默认参数捕获 __FILE__/__LINE__，必须用宏

#define KLOG_ERROR(code, extra)     ::KF::KLOGGER::Error(code, extra, __FILE__, __LINE__, __FUNCTION__)
#define KLOG_WARNING(code, extra)   ::KF::KLOGGER::Warning(code, extra, __FILE__, __LINE__, __FUNCTION__)
#define KLOG_INFO(code, extra)      ::KF::KLOGGER::Info(code, extra, __FILE__, __LINE__, __FUNCTION__)
#define KLOG_FATAL(code, extra)     ::KF::KLOGGER::Fatal(code, extra, __FILE__, __LINE__, __FUNCTION__)
namespace KF
{
    using limb = uint32_t; // 基础分块
    using dlimb = uint64_t; // double limb
    using slimb = int32_t; // signed limb
    using sdlimb = int64_t; // signed double limb

    constexpr const char* FILE_META = "meta";
    constexpr const char* FILE_CMDTITLE = "cmdtitle";
    constexpr const char* FILE_TITLE = "title";
    constexpr const char* FILE_DESC = "description";
    constexpr const char* FILE_AUTHOR = "author";
    constexpr const char* FILE_OPTION = "options";
    constexpr const char* FILE_OPTNAME = "name";
    constexpr const char* KBEGIN_UNKNOWN = "Unknown"; // KBegin 读取配置缺键时的默认补充值
    namespace KLOGGER
    {
        /// @brief 输出 [ERROR] 级别日志（程序继续运行）
        /// @param code  错误码
        /// @param extra 附加信息
        /// @param file  源文件名（由宏 __FILE__ 自动捕获）
        /// @param line  源行号（由宏 __LINE__ 自动捕获）
        /// @param func  函数名（由宏 __FUNCTION__ 自动捕获）
        void Error(Code code, const std::string& extra, const char* file, int line, const char* func);
        void Warning(Code code, const std::string& extra, const char* file, int line, const char* func);
        void Info(Code code, const std::string& extra, const char* file, int line, const char* func);
        void Fatal(Code code, const std::string& extra, const char* file, int line, const char* func);
        extern std::unordered_map<Code, std::string_view> Table;// 码表

        /////////////////////////////////////////////////////////
        // 错误码编码系统
        /////////////////////////////////////////////////////////
        /// @brief 日志等级枚举，对应错误码中的 [b] 段（1 位 16 进制）
        /// @note  与 Log 的输出前缀一一对应：Info/Warning/Error/Fatal
        enum class LogLevel : uint32_t
        {
            Info    = 1, // 信息
            Warning = 2, // 警告
            Error   = 3, // 错误
            Fatal   = 4, // 严重错误（会终止程序）
        };

        /// @brief 模块号常量，对应错误码中的 [aa] 段（2 位 16 进制）
        namespace Module
        {
            constexpr uint32_t Unknown = 0x00; // 未知模块
            constexpr uint32_t Common  = 0x01; // 通用模块（测试用）
            constexpr uint32_t KFIO    = 0x02; // KFIO 文件读写模块
            constexpr uint32_t KSON    = 0x03; // KSON 解析模块
            constexpr uint32_t KTIMER  = 0x04; // KTIMER 计时模块
            constexpr uint32_t KCLI    = 0x05; // KCLI 命令行交互模块
            constexpr uint32_t KBIGNUM = 0x02; // 大数模块
        }

        /// @param module 模块号 [aa]，2 位 16 进制（bit24-31）
        /// @param level  等级   [b]， 1 位 16 进制（bit20-23）
        /// @param type   类型   [cc]，2 位 16 进制（bit12-19）
        /// @param id     序号   [ddd]，3 位 16 进制（bit0-11）
        /// @return 组合后的 32 位错误码，格式 0x[aa][b][cc][ddd]
        /// @attention constexpr 使 KLOGGER.cpp 中的错误码定义可在编译期求值
        constexpr Code MakeCode(uint32_t module, LogLevel level, uint32_t type, uint32_t id) noexcept
        {
            return ((module & 0xFF) << 24)
                 | ((static_cast<uint32_t>(level) & 0xF) << 20)
                 | ((type & 0xFF) << 12)
                 | (id & 0xFFF);
        }

        /////////////////////////////////////////////////////////
        // 错误码声明（定义在 KLOGGER.cpp，通过 MakeCode 组装）
        // 头文件只放 extern 声明，改码时只需重编译 KLOGGER.cpp
        /////////////////////////////////////////////////////////

        // 通用 / 测试模块 (01)
        extern const Code TEST_INFO;   // 测试-信息
        extern const Code TEST_WARN;   // 测试-警告
        extern const Code TEST_ERROR;  // 测试-错误
        extern const Code TEST_FATAL;  // 测试-严重错误

        // KFIO 模块 (02)
        extern const Code KFIO_FILE_OPEN_FAIL; // KFIO 文件打开失败 FATAL
        extern const Code KFIO_FILE_READ_FAIL; // KFIO 文件读取失败 FATAL

        // KCLI 模块 (05)
        extern const Code KCLI_INPUT_INVALID;   // KCLI 输入解析失败（类型不匹配）

        // KTIMER 模块 (04)
        extern const Code KTIMER_NOT_FOUND;       // KTIMER 计时器不存在 Warning
        extern const Code KTIMER_ALREADY_EXISTS;  // KTIMER 计时器已存在（将被覆盖） Warning
        extern const Code KTIMER_STATE_ERROR;     // KTIMER 计时器状态不允许此操作 Warning

        // KSON 模块 (03)
        extern const Code KSON_PARSE_STRE;             // 解析错误 不以双引号开头(多半是BUG)
        extern const Code KSON_PARSE_STR_NOEND;        // 解析错误 没有双引号匹配
        extern const Code KSON_PARSE_MULPOINT;         // 解析警告 多余的小数点.
        extern const Code KSON_PARSE_NUM_UE;           // 解析警告 数字中有不支持的非阿拉伯数字
        extern const Code KSON_PARSE_NUMOR;            // 解析错误 数字超出类型范围
        extern const Code KSON_PARSE_NUM_USTYPE;       // 解析错误 返回暂不支持的数字类型
        extern const Code KSON_PARSE_ESCAPE_SPECIAL;   // 解析警告 转义字符后接非特殊字符
        extern const Code KSON_PARSE_UNFINISHED_ESCAPE;// 解析错误 解析转义时越界了
        extern const Code KSON_PARSE_BIG_EXP;          // 解析错误 科学计数法指数过大
        extern const Code KSON_PARSE_VAL_END;          // 解析错误 读到文件末尾仍缺值
        extern const Code KSON_PARSE_VAL_ERROR;        // 解析错误 值类型无法识别
        extern const Code KSON_PARSE_ARR_BEGIN;        // 解析错误 期望数组起始 [
        extern const Code KSON_PARSE_ARRUE;            // 解析错误 数组中出现非预期字符
        extern const Code KSON_PARSE_OBJ_BEGIN;        // 解析错误 期望对象起始 {
        extern const Code KSON_PARSE_OBJ_KEY_QUOTE;    // 解析错误 对象键未用双引号包裹
        extern const Code KSON_PARSE_OBJ_SEPERATOR;    // 解析错误 键后缺少冒号分隔符 :
        extern const Code KSON_PARSE_OBJUE;            // 解析错误 对象中出现非预期字符
        extern const Code KSON_PARSE_TRAIL;            // 解析警告 结尾有多余字符
        extern const Code KSON_TYPE_MISMATCH;          // AsSth 类型不匹配 FATAL
        //KBIGNUM 模块 (06)
        extern const Code KBIGNUM_MULPOINT;            // 解析警告 多余的小数点.
        extern const Code KBIGNUM_INVALIDCHAR;        // 解析警告 数字中有不支持的非阿拉伯数字
        // 未知模块 (00)
        extern const Code UNKNOWN;

        /////////////////////////////////////////////////////////
        // VT100 颜色常量（KBegin / Log 自动启用 VT100 处理）
        /////////////////////////////////////////////////////////
        namespace Color
        {
            constexpr const char* Reset   = "\033[0m";
            constexpr const char* Red     = "\033[31m";
            constexpr const char* Green   = "\033[32m";
            constexpr const char* Yellow  = "\033[33m";
            constexpr const char* Blue    = "\033[34m";
            constexpr const char* Magenta = "\033[35m";
            constexpr const char* Cyan    = "\033[36m";
            constexpr const char* LightYellow = "\033[93m"; // 亮黄色
            constexpr const char* Orange  = "\033[38;5;208m"; // 256 色橙
            constexpr const char* SkyBlue = "\033[38;5;75m";  // 256 色天蓝
            constexpr const char* Bold    = "\033[1m";
        }
    }
    /// @brief 大数运算库
    namespace KBIGNUM
    {
        class BigNum; // 前向声明，供自由函数签名使用

        /// @brief 面向内部的运算（自由函数）
        slimb  AbsCmp(const BigNum& a, const BigNum& b); // 1 a>b ; 0 a=b; -1 a<b
        BigNum AbsAdd(const BigNum& a, const BigNum& b);
        BigNum AbsSub(const BigNum& a, const BigNum& b);
        BigNum AbsMul(const BigNum& a, const BigNum& b);
        BigNum AbsDiv(const BigNum& a, const BigNum& b);
        BigNum AbsMod(const BigNum& a, const BigNum& b);
        BigNum AbsPow(const BigNum& a, const BigNum& b);

        BigNum AbsMulSchool(const BigNum& a, const BigNum& b); // 乘法（朴素算法）
        BigNum AbsMulKaratsuba(const BigNum& a, const BigNum& b); // 乘法（Karatsuba 算法）
        BigNum AbsMulToomCook3(const BigNum& a, const BigNum& b); // 乘法（Toom-Cook 3 算法）
        BigNum AbsMulNTT(const BigNum& a, const BigNum& b); // 乘法（NTT 算法）

        BigNum DivSchool(const BigNum& a, const BigNum& b); // 除法（朴素算法）
        BigNum DivNewton(const BigNum& a, const BigNum& b); // 除法（牛顿迭代法）
        BigNum DivNewtonMod(const BigNum& a, const BigNum& b); // 除法（牛顿迭代法）取模
        class BigNum
        {
            public:
                std::vector<limb> limbs = {0}; // 存储 (无小数点 无符号)
                /// @attention base = 10 ^ 9 ,之所以不用 2^32 是因为这 ToStr 太麻烦且太慢
                bool isneg = false; // 是否为负数
                size_t scale = 0; // 小数位数
                
                static BigNum ToBig(const std::string& str); // 字符串转大数
                std::string   ToStr() const; // 大数转字符串

                /// @brief 构造 支持空 字符串 数字
                BigNum() = default;
                BigNum(const std::string& str);// 用字符串构造
                BigNum(const dlimb& num);// 用数字构造

                /// @brief 面向用户的运算
                BigNum operator+(const BigNum& b) const;
                BigNum operator-(const BigNum& b) const;
                BigNum operator*(const BigNum& b) const;
                BigNum operator/(const BigNum& b) const;
                BigNum operator%(const BigNum& b) const;
                BigNum Pow(const BigNum& b) const;
                /// @brief 比较 (由于这是c++ 17 所以不使用三目运算符)
                bool operator==(const BigNum& b) const{return !(*this < b) && !(b < *this);}
                bool operator!=(const BigNum& b) const{return (*this < b) || (b < *this);}
                bool operator<(const BigNum& b) const {if(isneg && !b.isneg) return true; else if(!isneg && b.isneg) return false; else return AbsCmp(*this,b) < 0; }
                bool operator<=(const BigNum& b) const {return (*this < b) || (*this == b);}
                bool operator>(const BigNum& b) const {return !(*this <= b);}
                bool operator>=(const BigNum& b) const{return !(*this < b);}

                /// @brief 输出
                friend std::ostream& operator<<(std::ostream& os, const BigNum& b)
                {
                    os << b.ToStr();
                    return os;
                }
        };
        std::string Normalize(const std::string& str); //合法化 包括但不限于去小数点 去前后导0
        BigNum RandBigNum(bool isneg=false,size_t IntSize=0ULL,size_t DecSize=0ULL); // 生成随机大数(整数位数 前面几个正负号 小数位数)
    }
    namespace KSON
    {
        class NodePtr;
        enum class NodeType // 节点
        {
            kInt, // Integer 整数
            kDec, // Decimal 浮点数
            kBig, // BigNum 大数
            kStr, // String 字符串
            kBool, // Boolean 布尔值
            kArr, // Array 数组
            kObj, // Object 对象
            kNull, // Null 空值
        };
        class Node
        {
            public:
                // 构造
                Node() noexcept;
                explicit Node(bool val) noexcept;
                explicit Node(long long val) noexcept;
                explicit Node(double val) noexcept;
                explicit Node(KBIGNUM::BigNum val) noexcept;
                explicit Node(std::string val) noexcept;
                explicit Node(std::vector<Node> val);
                explicit Node(std::vector<std::pair<std::string,Node>> val);

                // 类型
                NodeType type()  const noexcept;
                bool IsNull()    const noexcept;
                bool IsBool()    const noexcept;
                bool IsInt()     const noexcept;
                bool IsDec()     const noexcept;
                bool IsBig()     const noexcept;
                bool IsNumber()  const noexcept;  // int 或 dec 或 big
                bool IsString()  const noexcept;
                bool IsArray()   const noexcept;
                bool IsObject()  const noexcept;

                // 取值
                bool             AsBool()   const;
                long long        AsInt()    const;
                double           AsDec() const;
                const KBIGNUM::BigNum& AsBig() const;
                std::string_view AsStr() const;
                const std::vector<Node>&     AsArr()  const;
                const std::vector<std::pair<std::string,Node>>&     AsObj() const;

                // 大小
                std::size_t size() const;
                
                // 查找（返回指针）
                const Node* find(std::string_view key) const; // 根据 键 查找对象中的键值对
                const Node* at(std::size_t index)      const; // 根据 下标 查找数组中的元素

                private:
                    // StorageType 节点存储类型 
                    /// @attention 与 NodeType 的区别是 : NodeType 是对外的

                    using arr_t = std::vector<Node>;
                    using obj_t = std::vector<std::pair<std::string,Node>>;
                    using storage_t = std::variant< 
                        std::monostate,
                        bool,std::string,double,long long,
                        KBIGNUM::BigNum,
                        arr_t,obj_t>;

                    storage_t Data;
        };
        struct PathSeg
        /// @attention 与 NodePtr 的区别是 : PathSeg 只记录一个位置片段，NodePtr 是一个完整的路径
        {
            std::string key;
            std::size_t index;
            PathSeg(std::string Key);
            PathSeg(std::size_t Index);
        };
        /// @brief KSON 文档的根容器，持有整棵解析后的数据树，并提供解析方法
        /// @attention 继承 std::enable_shared_from_this，允许成员函数内部调用 shared_from_this()
        /// @note 当前 Document 的成员函数（parse/parse_file/root/ResolvePath/构造）尚未在
        ///        KSON.cpp 中实现，属预留接口。现行解析路径走 NodePtr::Parse / read()，请勿调用。
        class Document : public std::enable_shared_from_this<Document>
        {
            public:
                static NodePtr parse(std::string_view text);//从 Document 中解析出一颗 KSON 树
                static NodePtr parse_file(std::string_view filepath); // 从文件读取，后面和 parse() 一样
                
                Document();
                NodePtr root();
                const Node* ResolvePath(const std::vector<PathSeg>& path) const;
                
                Node RootNode; // 根节点
                
            private:
                friend class NodePtr;
        };
        /// @brief KSON 树的指针，持有路径，提供访问方法
        class NodePtr
        {
            public:
                NodePtr() noexcept;
                explicit NodePtr(std::shared_ptr<Node> root) noexcept;
                NodePtr(std::shared_ptr<Node> root, std::vector<PathSeg> path) noexcept;
                
                static NodePtr Parse(std::string_view text);
                static NodePtr ParseFile(std::string_view filepath);
                
                NodePtr operator[](std::string_view key) const;
                NodePtr operator[](std::size_t index) const;
                NodePtr operator[](const char* key) const;
                
                const Node* TryResolve() const;
                const Node* Resolve() const;
                
                std::string Str() const;
                long long Int() const;
                double Dec() const;
                KBIGNUM::BigNum Big() const;
                bool Bool() const;
                std::size_t Size() const;
                std::size_t size() const;  // 小写别名，等价于 Size()，方便 arr.size() 风格
                bool Exists() const;

                /// @brief 自动分析值的类型，返回可打印的字符串表示
                /// @return 根据节点类型自动转换：
                ///         null→"null"  bool→"true"/"false"  int→数字串
                ///         dec→浮点串   str→字符串原文
                ///         arr→[e1, e2, ...]  obj→{"k": v, ...}
                /// @note  路径未找到时返回 "null"，不会 Fatal
                std::string Auto() const;
                
            private:
                std::shared_ptr<Node> root_;
                std::vector<PathSeg> path_;
                mutable const Node* cached_ = nullptr;
                
                const Node* ResolvePath(const std::vector<PathSeg>& path) const;
        };
        using kson = NodePtr;

        /////////////////////////////////////////////////////////

        ///@brief 读取文件并解析
        std::string Preprocess(std::string raw); // 预处理，将注释删除，将转义字符替换，去掉空格 换行等
        kson read(std::string_view processed);
        kson ReadKsonFile(std::string_view filename);
    }
    namespace KFIO
    {
        std::string ReadFileRaw(std::string_view filepath);// 读取文件(粗文本 没有任何处理)
    }
    namespace KCLI
    {
        // Color 常量统一定义在 KF::KLOGGER::Color，此处创建别名以便 KCLI 内直接使用 Color::xxx
        namespace Color = KF::KLOGGER::Color;

        /// @brief 链式输出流，自带默认颜色
        /// @details 每次 << 自动套用默认色；遇到 std::endl 等 manipulator 时
        ///          先输出 Color::Reset 再输出 manipulator，防止颜色泄漏。
        ///          如需临时换色：koutE << Color::Red << "严重" << std::endl;
        /// @code
        /// kout  << "普通信息" << 42 << std::endl;  // 天蓝色
        /// koutW << "警告信息" << std::endl;         // 淡黄色
        /// koutE << "错误信息" << std::endl;         // 橙色
        /// koutF << "致命信息" << std::endl;         // 红色
        /// @endcode
        class Kout
        {
            const char* color_;
        public:
            explicit constexpr Kout(const char* c) noexcept : color_(c) {}

            template<typename T>
            Kout& operator<<(const T& val)
            {
                std::cout << color_ << val;
                return *this;
            }

            /// manipulator（std::endl / std::flush）：先 Reset 再输出
            Kout& operator<<(std::ostream& (*manip)(std::ostream&))
            {
                std::cout << Color::Reset << manip;
                return *this;
            }
        };

        /// @brief 链式输入流，operator>> 根据目标变量类型自动推导
        /// @code int x; double y; std::string s; kin >> x >> y >> s; @endcode
        /// @note  每个 >> 读取一行（getline），按目标类型转换，转换失败置 0 并告警
        class Kin
        {
        public:
            template<typename T>
            Kin& operator>>(T& val)
            {
                std::string line;
                std::getline(std::cin, line);
                if constexpr (std::is_same_v<T, std::string>)
                    val = line;
                else if constexpr (std::is_same_v<T, bool>)
                    val = (line == "1" || line == "true" || line == "yes");
                else if constexpr (std::is_integral_v<T>)
                {
                    try { val = static_cast<T>(std::stoll(line)); }
                    catch (const std::exception&) { val = 0; KLOG_WARNING(::KF::KLOGGER::KCLI_INPUT_INVALID, line); }
                }
                else if constexpr (std::is_floating_point_v<T>)
                {
                    try { val = static_cast<T>(std::stod(line)); }
                    catch (const std::exception&) { val = 0; KLOG_WARNING(::KF::KLOGGER::KCLI_INPUT_INVALID, line); }
                }
                return *this;
            }
        };

        inline Kout kout(Color::Bold);      ///< 默认天蓝色（普通输出）
        inline Kout koutW(Color::LightYellow); ///< 默认淡黄色（Warning）
        inline Kout koutE(Color::Orange);  ///< 默认橙色（Error）
        inline Kout koutF(Color::Red);     ///< 默认红色（Fatal）
        inline Kin kin;                    ///< 全局输入流对象（链式 >> ，自动推导类型）

        /////////////////////////////////////////////////////////
        // CLI 功能函数
        /////////////////////////////////////////////////////////

        /// @brief 初始化 CLI：启用 VT100 颜色，设置控制台标题（支持中文），
        ///        打印标题框和描述
        /// 字符串版本（可变参数，定义见下方模板）：
        ///   KBegin(title, desc)                  // 2 参
        ///   KBegin(title, desc, author)          // 3 参
        ///   KBegin(cmdtitle, title, desc, author)// 4 参（cmdtitle 仅作窗口标题）
        /// @param a 首参（2/3 参时为标题，4 参时为窗口标题）
        /// @param b 次参（2/3 参时为描述，4 参时为框标题）
        /// @param rest 剩余参数
        void KBeginImpl(const std::vector<std::string>& args);// 内部实现：args[0]=窗口标题，其余为框内容

        template<typename... Args>
        void KBegin(const std::string& a, const std::string& b, Args... rest)
        {
            std::vector<std::string> args{a, b, rest...};
            KBeginImpl(args);
        }

        void KBegin(const KSON::kson file);//从文件中读取
        /// @brief 显示选项菜单，循环等待用户输入合法选项
        /// @param menu KSON 节点，需含 "title" 和 "options"（字符串数组）
        /// @return 选中项索引（0-based），输入非法时循环提示
        std::size_t KOptions(const KSON::kson& menu);

        /// @brief 暂停：显示"按任意键继续..."并等待按键
        void kpause();

        /// @brief 结束：暂停后退出程序（exit(0)）
        void KEnd();
    }
    namespace KTIMER
    {
        // Color 常量统一定义在 KF::KLOGGER::Color，此处创建别名以便 KTIMER 内直接使用 Color::xxx
        namespace Color = KF::KLOGGER::Color;

        /// @brief 时间单位枚举
        enum class TimeUnit
        {
            ns,  // 纳秒
            us,  // 微秒
            ms,  // 毫秒
            s,   // 秒
        };

        /// @brief 计时器状态枚举
        enum class TimerState
        {
            Running,  // 运行中
            Paused,   // 已暂停
        };

        /// @brief 新建计时器（指定名字和单位），创建后立即开始计时
        /// @param name  计时器名称（唯一标识）
        /// @param unit  时间单位（ns/us/ms/s）
        /// @return true=新建成功，false=同名已存在（已覆盖，发出警告）
        bool AddTimer(const std::string& name, TimeUnit unit);

        /// @brief 暂停计时器（累计已运行时间）
        /// @return true=暂停成功，false=不存在或未在运行
        bool PauseTimer(const std::string& name);

        /// @brief 恢复已暂停的计时器
        /// @return true=恢复成功，false=不存在或未暂停
        bool StartTimer(const std::string& name);

        /// @brief 删除计时器
        /// @return true=删除成功，false=不存在
        bool DeleteTimer(const std::string& name);

        /// @brief 获取计时器当前累计时间（按计时器单位）
        /// @return >=0 累计时间，-1.0 表示不存在
        double GetTimer(const std::string& name);

        /// @brief 打印单个计时器信息（格式化框）
        void PrintTimer(const std::string& name);

        /// @brief 打印所有计时器信息（格式化表格，按名称排序）
        void PrintAllTimers();
    }
    /// @brief 实用库
    namespace KUTIL
    {
        sdlimb RandInt(sdlimb min, sdlimb max); ///< 生成 [min, max] 范围内的随机整数
        sdlimb Pow10(sdlimb n); // 10的n次方
        std::string MaxLenStr3(std::string a, std::string b, std::string c);
        std::string MaxLenStr4(std::string a, std::string b, std::string c, std::string d);
    }
}
namespace KSON = KF::KSON;
namespace KLOG = KF::KLOGGER;
namespace KFIO = KF::KFIO;
namespace KUTIL = KF::KUTIL;
namespace KCLI = KF::KCLI;
namespace KTIMER = KF::KTIMER;
namespace KBIGNUM = KF::KBIGNUM;
constexpr size_t DEFAULT_RESIZE_STR_LEN = 64; // 默认KSON中字符串的分配长度 (超过这个长度会再次扩容)
using namespace KF::KLOGGER;