#pragma once
#include<vector>
#include<cmath>
#include<random>
#include<windows.h>
#include<functional>
#include<iomanip>
#include<memory>
#include<chrono>
#include<fstream>
#include<iostream>
#include<unordered_map>
#include<variant>
using Code = uint32_t;
namespace KF
{
    namespace KLOGGER
    {
        void Error(Code code, const std::string& extra);
        void Warning(Code code, const std::string& extra);
        void Info(Code code, const std::string& extra);
        void Fatal(Code code, const std::string& extra);
        extern std::unordered_map<Code, std::string_view> Table;// 码表
    }
    namespace KSON
    {
        class NodePtr;
        enum class NodeType // 节点
        {
            kInt, // Integer 整数
            kDec, // Decimal 浮点数
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
                explicit Node(std::string val) noexcept;
                explicit Node(std::vector<Node> val);
                explicit Node(std::vector<std::pair<std::string,Node>> val);

                // 类型
                NodeType type()  const noexcept;
                bool IsNull()    const noexcept;
                bool IsBool()    const noexcept;
                bool IsInt()     const noexcept;
                bool IsDec()     const noexcept;
                bool IsNumber()  const noexcept;  // int 或 dec
                bool IsString()  const noexcept;
                bool IsArray()   const noexcept;
                bool IsObject()  const noexcept;

                // 取值
                bool             AsBool()   const;
                long long        AsInt()    const;
                double           AsDec() const;
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
        class NodeProxy
        {
            public:
                NodeProxy(std::shared_ptr<Document> doc, std::vector<PathSeg> path) noexcept;
                
                ///@brief 路径继承
                NodeProxy operator[](std::string key) const;  // 根据 键 继续向下查找
                NodeProxy operator[](std::size_t index) const;// 根据 下标 继续向下查找
                
                // 隐式转换：自动解析并返回 Node 引用
                /// @attention 让 NodeProxy 用起来像 const Node&
                operator const Node&() const;
                
                bool             AsBool()   const { return Resolve().AsBool(); }
                long long        AsInt()    const { return Resolve().AsInt(); }
                double           AsDec()    const { return Resolve().AsDec(); }
                std::string      AsStr()    const { return std::string(Resolve().AsStr()); }
                std::size_t      size()     const { return Resolve().size(); }
                
                bool IsNull()   const { return Resolve().IsNull(); }
                bool IsBool()   const { return Resolve().IsBool(); }
                bool IsInt()    const { return Resolve().IsInt(); }
                bool IsDec()    const { return Resolve().IsDec(); }
                bool IsNumber() const { return Resolve().IsNumber(); }
                bool IsString() const { return Resolve().IsString(); }
                bool IsArray()  const { return Resolve().IsArray(); }
                bool IsObject() const { return Resolve().IsObject(); }
                
                bool exists() const;
                
            private:
                const Node& Resolve() const;  // 严格解析，失败抛异常
                
                std::shared_ptr<Document> doc_;
                std::vector<PathSeg> path_;
                mutable const Node* cached_ = nullptr;
        };
        class NodePtr
        {
            public:
                //NodePtr() noexcept;
                //NodePtr(std::shared_ptr<Document> doc,std::vector<PathSeg> path) noexcept;
        };
        using kson = NodePtr;

        /////////////////////////////////////////////////////////

        ///@brief 读取文件并解析
        std::string Preprocess(std::string raw); // 预处理，将注释删除，将转义字符替换，去掉空格 换行等
    }
    namespace KFIO
    {
        std::string ReadFileRaw(std::string_view filepath);// 读取文件(粗文本 没有任何处理)
    }
}
namespace KSON = KF::KSON;
namespace KLOG = KF::KLOGGER;
namespace KFIO = KF::KFIO;

constexpr char CHAR_COMMENT1 = '#';
constexpr char CHAR_QUOTE1  = '\"';
constexpr char CHAR_ESCAPE1  = '\\';
/* @brief 报错码 KLOGGER.cpp
错误码格式：0x[aa][b][cc][ddd]，其中：
        - [aa]：模块号（2位16进制）
        - [b]：等级（1位16进制）
        - [cc]: 类型（2位16进制）
        - [ddd]：具体错误码（3位16进制）
*/
constexpr Code TEST_INFO = 0x01101001;
constexpr Code TEST_WARN = 0x01201002;
constexpr Code TEST_ERROR = 0x01301003;
constexpr Code TEST_FATAL = 0x01401004;

constexpr Code KSON_TYPE_MISMATCH  = 0x02301001; // KSON AsSth 类型不匹配
constexpr Code KFIO_FILE_OPEN_FAIL = 0x02401001; // KFIO 文件打开失败
constexpr Code KFIO_FILE_READ_FAIL = 0x02401002; // KFIO 文件读取失败