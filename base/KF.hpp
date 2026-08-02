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

namespace KF
{
    namespace KLOGGER
    {
        
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
                NodeType type()   noexcept;
                bool IsNull()    const noexcept;
                bool IsBool()    const noexcept;
                bool IsInt()     const noexcept;
                bool IsDec()  const noexcept;
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

                    storage_t data_;
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
        class NodePtr
        {
            public:
                NodePtr() noexcept;
                NodePtr(std::shared_ptr<Document> doc,std::vector<PathSeg> path) noexcept;
        };
        using kson = NodePtr;
        /// @example kson a = KSON::parse("({"a":1,"b":2,"c":[1,2,3],"d":{"e":4,"f":5}})");
    }
}
namespace KSON = KF::KSON;
namespace KLOG = KF::KLOGGER;
#define LOG KF::KLOGGER::Logger::instance()