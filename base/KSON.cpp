
#include "KF.hpp"
namespace KF
{
    namespace KSON
    {
        Node::Node() noexcept : Data(std::monostate{}) {}  // 默认构造：空节点，类型为 kNull
        Node::Node(bool val) noexcept : Data(val) {}        // 从 bool 构造，类型为 kBool
        Node::Node(long long val) noexcept : Data(val) {}   // 从 long long 构造，类型为 kInt
        Node::Node(double val) noexcept : Data(val) {}      // 从 double 构造，类型为 kDec
        Node::Node(std::string val) noexcept : Data(std::move(val)) {}  // 从 string 构造，类型为 kStr
        Node::Node(std::vector<Node> val) : Data(std::move(val)) {}     // 从数组构造，类型为 kArr
        Node::Node(std::vector<std::pair<std::string, Node>> val) : Data(std::move(val)) {}  // 从对象构造，类型为 kObj

        /// @brief 返回当前节点的类型
        /// @attention 使用 std::visit 遍历 variant，根据实际存储的类型返回对应的 NodeType
        NodeType Node::type() const noexcept
        {
            return std::visit([](auto&& arg) -> NodeType
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) return NodeType::kNull;
                else if constexpr (std::is_same_v<T, bool>) return NodeType::kBool;
                else if constexpr (std::is_same_v<T, long long>) return NodeType::kInt;
                else if constexpr (std::is_same_v<T, double>) return NodeType::kDec;
                else if constexpr (std::is_same_v<T, std::string>) return NodeType::kStr;
                else if constexpr (std::is_same_v<T, arr_t>) return NodeType::kArr;
                else if constexpr (std::is_same_v<T, obj_t>) return NodeType::kObj;
                return NodeType::kNull;
            }, Data);
        }
        /// @brief 判断是否为 null（空值）
        bool Node::IsNull()    const noexcept { return type() == NodeType::kNull; }
        /// @brief 判断是否为布尔值
        bool Node::IsBool()    const noexcept { return type() == NodeType::kBool; }
        /// @brief 判断是否为整数
        bool Node::IsInt()     const noexcept { return type() == NodeType::kInt; }
        /// @brief 判断是否为浮点数
        bool Node::IsDec()     const noexcept { return type() == NodeType::kDec; }
        /// @brief 判断是否为数字（整数或浮点数）
        bool Node::IsNumber()  const noexcept { return IsInt() || IsDec(); }
        /// @brief 判断是否为字符串
        bool Node::IsString()  const noexcept { return type() == NodeType::kStr; }
        /// @brief 判断是否为数组
        bool Node::IsArray()   const noexcept { return type() == NodeType::kArr; }
        /// @brief 判断是否为对象
        bool Node::IsObject()  const noexcept { return type() == NodeType::kObj; }


        /// @brief 取布尔值
        bool Node::AsBool() const {
            if (!IsBool()) KLOG::Error(KSON_TYPE_MISMATCH, "Node is not boolean");
            return std::get<bool>(Data);
        }
        /// @brief 取整数值
        long long Node::AsInt() const {
            if (!IsInt()) KLOG::Error(KSON_TYPE_MISMATCH, "Node is not integer");
            return std::get<long long>(Data);
        }
        /// @brief 取浮点数值
        /// @attention 如果底层存储的是整数，会自动转换为 double
        double Node::AsDec() const {
            if (IsInt()) return static_cast<double>(AsInt());  // 整数可隐式转为浮点
            if (!IsDec()) KLOG::Error(KSON_TYPE_MISMATCH, "Node is not decimal");
            return std::get<double>(Data);
        }
        /// @brief 取字符串
        std::string_view Node::AsStr() const {
            if (!IsString()) KLOG::Error(KSON_TYPE_MISMATCH, "Node is not string");
            return std::get<std::string>(Data);
        }
        /// @brief 取数组引用
        const std::vector<Node>& Node::AsArr() const {
            if (!IsArray()) KLOG::Error(KSON_TYPE_MISMATCH, "Node is not array");
            return std::get<arr_t>(Data);
        }
        /// @brief 取对象引用
        const std::vector<std::pair<std::string, Node>>& Node::AsObj() const {
            if (!IsObject()) KLOG::Error(KSON_TYPE_MISMATCH, "Node is not object");
            return std::get<obj_t>(Data);
        }
        /// @brief 返回容器大小
        std::size_t Node::size() const
        {
            if (IsArray()) return AsArr().size();
            if (IsObject()) return AsObj().size();
            return 0;
        }

        /// @brief 在对象中按键查找
        /// @param key 要查找的键名
        /// @return 指向找到的 Node 的指针，未找到返回 nullptr
        const Node* Node::find(std::string_view key) const
        {
            if (!IsObject()) return nullptr;
            for (const auto& [k, v] : AsObj())
                if (k == key) return &v;
            return nullptr;  // 遍历完未找到
        }

        /// @brief 在数组中按下标查找
        /// @param index 数组下标
        /// @return 指向找到的 Node 的指针，越界返回 nullptr
        const Node* Node::at(std::size_t index) const {
            if (!IsArray()) return nullptr;
            const auto& arr = AsArr();
            if (index >= arr.size()) return nullptr;
            return &arr[index];
        }

        /// @brief 构造一个键路径段（用于对象键访问）
        /// @param Key 键名字符串
        PathSeg::PathSeg(std::string Key) : key(std::move(Key)), index(0) {}

        /// @brief 构造一个索引路径段（用于数组下标访问）
        /// @param Index 数组下标
        PathSeg::PathSeg(std::size_t Index) : key(), index(Index) {}

        /// @brief 默认构造：创建一个空文档，RootNode 为 null
        Document::Document() = default;

        /// @brief 获取根节点的 NodePtr 视图
        // NodePtr Document::root() {
        //     return NodePtr(shared_from_this(), {});
        // }

        /// @brief 按路径解析，找到对应的 Node 指针
        /// @param path 路径段列表，每个段要么是键（访问对象）要么是索引（访问数组）
        /// @return 指向目标节点的指针，路径无效时返回 nullptr
        /// @attention 这是 NodePtr 懒解析的核心：NodePtr 只存路径，真正取值时才调用此函数
        const Node* Document::ResolvePath(const std::vector<PathSeg>& path) const {
            const Node* current = &RootNode;  // 从根节点开始遍历
            for (const auto& seg : path) {    // 逐个路径段向下解析
                if (!current) return nullptr;  // 上一步已经失效，路径中断
                
                if (!seg.key.empty()) {
                    // 键非空：按对象键访问
                    current = current->find(seg.key);
                } else {
                    // 键为空：按数组索引访问
                    current = current->at(seg.index);
                }
            }
            return current;  // 返回最终定位到的节点指针
        }
        /// @brief 文件读取并处理
        std::string Preprocess(std::string raw)
        {
            std::string res;
            res.resize(raw.size());
            size_t WriteIndex = 0, ReadIndex = 0;  // 读到哪 写到哪
            enum State { Normal, InString, InEscape} state = Normal;  // 状态机 同时只能处于一种状态
            for(;ReadIndex < raw.size();)
            {
                char c = raw[ReadIndex];
                switch (state)
                {
                    case Normal:
                        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                            ReadIndex++;//跳过空白符号
                        else if(c == CHAR_COMMENT1)//如果是注释行
                        {
                            auto end = raw.find('\n', ReadIndex);//跳过这行
                            ReadIndex = (end == std::string::npos) ? raw.size() : end;
                        }
                        else if(c == CHAR_QUOTE1)//如果是引号
                        {
                            state = InString;
                            res[WriteIndex++] = c;
                            ReadIndex++;
                        }
                        else
                        {
                            res[WriteIndex++] = c;
                            ReadIndex++;
                        }
                        break;
                    case InString:
                        if(c == CHAR_ESCAPE1) //如果有 转义字符
                        {
                            state = InEscape;
                            ReadIndex++;
                        }
                        else if(c == CHAR_QUOTE1) //如果是引号 则结束
                        {
                            state = Normal;
                            res[WriteIndex++] = c;
                            ReadIndex++;
                        }
                        else
                        {
                            res[WriteIndex++] = c;
                            ReadIndex++;
                        }
                        break;
                    case InEscape:
                       if(c == 'n') res[WriteIndex++]='\n';/*如果是 \n 换行*/
                       if(c == 't') res[WriteIndex++]='\t';/*如果是 \t 制表符*/
                       if(c == 'r') res[WriteIndex++]='\r';/*如果是 \r 回车*/
                       if(c == 'b') res[WriteIndex++]='\b';/*如果是 \b 退格*/
                       state = InString;//回到 字符串 状态
                       ReadIndex++;
                       break;
                }
            }
            res.resize(WriteIndex);
            return res;
        }
    }
}