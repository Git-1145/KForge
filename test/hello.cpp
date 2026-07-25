#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <memory>

namespace fio
{
    using StringVec = std::vector<std::string>;

    enum class ValueType
    {
        Empty,
        String,
        Int64
    };

    struct Node;
    using NodePtr = std::unique_ptr<Node>;
    using ObjectMap = std::unordered_map<std::string, NodePtr>;

    struct Node
    {
        ObjectMap obj;
        std::vector<Node> arr;

        ValueType type = ValueType::Empty;
        std::string str_val;
        long long num_val = 0LL;

        bool is_object() const { return !obj.empty(); }
        bool is_array()  const { return !arr.empty(); }
        bool is_string() const { return type == ValueType::String; }
        bool is_number() const { return type == ValueType::Int64; }

        Node() = default;
        Node(Node&&) noexcept = default;
        Node& operator=(Node&&) noexcept = default;
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
    };

    struct NodeRef
    {
        Node* ptr;
        NodeRef(Node* p) : ptr(p) {}

        std::string str() const
        {
            return ptr->str_val;
        }

        bool has(const std::string& key) const
        {
            return ptr->obj.count(key) > 0;
        }

        NodeRef get(const std::string& key)
        {
            auto it = ptr->obj.find(key);
            if (it == ptr->obj.end())
                throw std::runtime_error("key not exist: " + key);
            return NodeRef(it->second.get());
        }

        NodeRef operator[](size_t idx)
        {
            if (idx >= ptr->arr.size())
            {
                throw std::runtime_error("array out of range, size:" + std::to_string(ptr->arr.size())
                    + " idx:" + std::to_string(idx));
            }
            return NodeRef(&ptr->arr[idx]);
        }

        // 路径解析 [key][key][0] 支持数组下标
        NodeRef value(const std::string& path);

        // ========== 新增：解析 &opt 对象，返回vector ==========
        std::vector<std::string> opt(const std::string& path);

        bool is_string() const { return ptr->is_string(); }
        bool is_number() const { return ptr->is_number(); }
        bool is_object() const { return ptr->is_object(); }
        bool is_array() const  { return ptr->is_array(); }

        operator std::string() const
        {
            return ptr->str_val;
        }
        operator long long() const
        {
            return ptr->num_val;
        }

        std::vector<Node>& array() { return ptr->arr; }
        ObjectMap& object() { return ptr->obj; }
    };

    namespace
    {
        Node root;
        std::string file_text;

        std::string trim(const std::string& s)
        {
            auto is_whitespace = [](unsigned char c)
            {
                return c == ' ' || c == '\t' || c == '\r' || c == '\n';
            };
            size_t start = 0;
            while (start < s.size() && is_whitespace(static_cast<unsigned char>(s[start])))
                start++;
            size_t end = s.size();
            while (end > start && is_whitespace(static_cast<unsigned char>(s[end - 1])))
                end--;
            return s.substr(start, end - start);
        }

        void skip_ws(const std::string& src, size_t& pos)
        {
            
            auto is_whitespace = [](unsigned char c)
            {
                return c == ' ' || c == '\t' || c == '\r' || c == '\n';
            };
            while (pos < src.size() && is_whitespace(static_cast<unsigned char>(src[pos])))
                pos++;
        }

        std::string read_bracket(const std::string& src, size_t& pos)
        {
            skip_ws(src, pos);
            if (pos >= src.size() || src[pos] != '[')
                return "";
            pos++;
            size_t st = pos;
            while (pos < src.size() && src[pos] != ']')
                pos++;
            std::string res = src.substr(st, pos - st);
            pos++;
            return trim(res);
        }

        std::string read_quoted(const std::string& src, size_t& pos)
        {
            ++pos;
            size_t st = pos;
            while (pos < src.size() && src[pos] != '"')
                ++pos;
            std::string content = src.substr(st, pos - st);
            ++pos;
            return content;
        }

        static Node parse(const std::string& src, size_t& pos)
        {
            skip_ws(src, pos);
            Node node;

            if (src[pos] == '{')
            {
                pos++;
                skip_ws(src, pos);
                while (pos < src.size() && src[pos] != '}')
                {
                    std::string key = read_bracket(src, pos);
                    skip_ws(src, pos);
                    if (src[pos] == '=')
                        pos++;
                    skip_ws(src, pos);

                    node.obj[key] = std::make_unique<Node>(std::move(parse(src, pos)));

                    skip_ws(src, pos);
                    if (src[pos] == ',')
                        pos++;
                    skip_ws(src, pos);
                }
                pos++;
            }
            else if (src[pos] == '(')
            {
                pos++;
                skip_ws(src, pos);
                while (pos < src.size() && src[pos] != ')')
                {
                    node.arr.push_back(std::move(parse(src, pos)));
                    skip_ws(src, pos);
                    if (src[pos] == ',')
                        pos++;
                    skip_ws(src, pos);
                }
                pos++;
            }
            else if (src[pos] == '"')
            {
                node.str_val = read_quoted(src, pos);
                node.type = ValueType::String;
            }
            else
            {
                size_t st = pos;
                while (pos < src.size()
                    && src[pos] != ',' && src[pos] != '}' && src[pos] != ')'
                    && src[pos] != '[' && src[pos] != '=')
                {
                    pos++;
                }
                std::string raw = trim(src.substr(st, pos - st));

                bool is_number = true;
                size_t idx = 0;
                if (!raw.empty() && raw[0] == '-') idx = 1;
                for (; idx < raw.size(); ++idx)
                {
                    if (!std::isdigit(static_cast<unsigned char>(raw[idx])))
                    {
                        is_number = false;
                        break;
                    }
                }

                if (!raw.empty() && is_number)
                {
                    node.num_val = std::stoll(raw);
                    node.type = ValueType::Int64;
                }
                else
                {
                    node.str_val = raw;
                    node.type = ValueType::String;
                }
            }
            return node;
        }

        void parse_root(const std::string& src, size_t& pos)
        {
            skip_ws(src, pos);
            while (pos < src.size())
            {
                skip_ws(src, pos);
                if (pos >= src.size()) break;

                std::string top_key = read_bracket(src, pos);
                if(top_key.empty())
                {
                    pos++;
                    continue;
                }
                skip_ws(src, pos);
                if (src[pos] == '=')
                    pos++;
                skip_ws(src, pos);

                root.obj[top_key] = std::make_unique<Node>(std::move(parse(src, pos)));

                skip_ws(src, pos);
                if (src[pos] == ',')
                    pos++;
            }
        }
    }

    NodeRef NodeRef::value(const std::string& path)
    {
        NodeRef cur = *this;
        size_t pos = 0;
        const std::string& src = path;
        while (pos < src.size())
        {
            size_t lb = src.find('[', pos);
            if (lb == std::string::npos) break;
            size_t rb = src.find(']', lb + 1);
            if (rb == std::string::npos)
                throw std::runtime_error("path syntax error, missing ']': " + path);

            std::string seg = src.substr(lb + 1, rb - lb - 1);
            pos = rb + 1;

            bool all_digit = true;
            size_t start_idx = 0;
            if (!seg.empty() && seg[0] == '-') start_idx = 1;
            for (size_t i = start_idx; i < seg.size(); i++)
            {
                if (!std::isdigit(static_cast<unsigned char>(seg[i])))
                {
                    all_digit = false;
                    break;
                }
            }

            if (all_digit && !seg.empty())
            {
                size_t idx = static_cast<size_t>(std::stoll(seg));
                cur = cur[idx];
            }
            else
            {
                cur = cur.get(seg);
            }
        }
        return cur;
    }

    // ================= opt 实现 === ==============
    std::vector<std::string> NodeRef::opt(const std::string& path)
    {
        std::vector<std::string> res;
        NodeRef target = this->value(path);

        // 取出 title
        NodeRef titleNode = target.get("title");
        res.push_back(titleNode.str());

        // 取出 options 数组
        NodeRef optArr = target.get("options");
        if (!optArr.is_array())
            throw std::runtime_error("opt field [options] must be array ()");

        auto& arr = optArr.array();
        for (size_t i = 0; i < arr.size(); ++i)
        {
            NodeRef item(&arr[i]);
            res.push_back(item.str());
        }
        return res;
    }

    bool open(const std::string& path)
    {
        std::ifstream fin(path);
        if (!fin.is_open())
        {
            std::cerr << "[fio] open failed: " << path << std::endl;
            return false;
        }
        std::stringstream ss;
        ss << fin.rdbuf();
        file_text = ss.str();

        // 清除UTF8 BOM
        if(file_text.size() >= 3)
        {
            unsigned char c1 = static_cast<unsigned char>(file_text[0]);
            unsigned char c2 = static_cast<unsigned char>(file_text[1]);
            unsigned char c3 = static_cast<unsigned char>(file_text[2]);
            if(c1 == 0xEF && c2 == 0xBB && c3 == 0xBF)
            {
                file_text.erase(0,3);
            }
        }

        size_t p = 0;
        root = Node{};
        parse_root(file_text, p);
        return true;
    }

    namespace inner_api
    {
        NodeRef read(const std::string& top_key)
        {
            NodeRef ref(&root);
            if (!ref.has(top_key))
            {
                throw std::runtime_error("top key [" + top_key + "] not found!");
            }
            return ref.get(top_key);
        }
    }
    using inner_api::read;
}

// ========= MAIN 测试代码 =========
int main()
{
    try
    {
        if (!fio::open("cfg.txt"))
        {
            std::cout << "文件打开失败\n";
            std::cin.get();
            return -1;
        }

        fio::NodeRef File = fio::read("kf");

        // 数组下标路径访问
        long long a = File.value("[alice][msg][1]");
        std::cout << "alice msg[1] = " << a << "\n";

        // &opt 特殊对象读取，返回vector
        std::vector<std::string> optVec = File.opt("[sort]");
        std::cout << "opt title = " << optVec[0] << "\n";
        std::cout << "opt[1] = " << optVec[1] << "\n";
        std::cout << "opt[2] = " << optVec[2] << "\n";
        std::cout << "opt[3] = " << optVec[3] << "\n";

        std::string str_data = File.get("bob").get("msg")[0];
        long long num_data = File.get("bob").get("age")[1];

        std::cout << "str_data = [" << str_data << "]\n";
        std::cout << "num_data = " << num_data << "\n";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    std::cin.get();
    return 0;
}
