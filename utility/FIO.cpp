#include "KF.hpp"
namespace KF::UTI
{
    namespace FIO
    {
        nodeView nodeView::operator[](size_t index)
        {
            if (ptr == nullptr)
                throw std::runtime_error("cannot index null node");
            //std::cerr << "DEBUG operator[]: ptr=" << ptr << " is_array=" << ptr->is_array() 
                  //<< " arr.size=" << ptr->arr.size() << " index=" << index << "\n";
            if (!ptr->is_array())
            {
               // std::cerr << "DEBUG operator[] abort: not array\n";
                throw std::runtime_error("Unexpected index: current node is not an array");
            }
            if (index >= ptr->arr.size())
            {
                //std::cerr << "DEBUG operator[] abort: out of range\n";
                throw std::runtime_error(
                "array out of range, size:" + std::to_string(ptr->arr.size()) +
                " idx:" + std::to_string(index));
            }
            //std::cerr << "DEBUG operator[] success: returning element\n";
            return nodeView(&ptr->arr[index]);
        }
        //放置在你的类外部，作为静态辅助函数
        static std::vector<std::string> splitBracket(const std::string& str)
        {
            std::vector<std::string> path;
            size_t pos = 0;
            const size_t len = str.size();
            while (pos < len)
            {
                size_t leftBracket = str.find('[', pos);
                if (leftBracket == std::string::npos)
                    break;
                size_t rightBracket = str.find(']', leftBracket + 1);
                if (rightBracket == std::string::npos)
                    break;

                std::string key = str.substr(leftBracket + 1, rightBracket - leftBracket - 1);
                path.push_back(key);
                pos = rightBracket + 1;
            }
            return path;
        }

        nodeView nodeView::get(const std::string& key)
        {
            auto it = ptr->obj.find(key);
            if (it == ptr->obj.end())
                throw std::runtime_error("key not exist: " + key);
            return nodeView(it->second.get());
        }
        namespace
        {
            std::string trim(const std::string& str);
        }
        nodeView nodeView::value(const std::string& path)
        {
            nodeView current = *this;
            size_t pos = 0;
            const std::string& str = path;
            while (pos < str.size())
            {
                if (str[pos] == FIO_KEY_BEG)
                {
                    size_t right = str.find(FIO_KEY_END, pos + 1);
                    if (right == std::string::npos)
                        throw std::runtime_error("path syntax error, missing FIO_KEY_END");
                    std::string key = trim(str.substr(pos + 1, right - pos - 1));
                    pos = right + 1;

                    bool isNum = true;
                    bool dot = false;
                    size_t idx = 0;
                    if (!key.empty() && (key[0] == '-' || key[0] == '+'))
                        idx = 1;
                    int digitCount = 0;
                    for (; idx < key.size(); idx++) //键可能包含符号或数字
                    {
                        if (std::isdigit(static_cast<unsigned char>(key[idx])))
                        {
                            digitCount++;
                            continue;
                        }
                        if (key[idx] == '.' && !dot)
                        {
                            dot = true;
                            continue;
                        }
                        isNum = false;
                        break;
                    }
                    if (digitCount == 0)
                        isNum = false;
                    if (isNum && !key.empty())
                    {
                        size_t idx = static_cast<size_t>(std::stoll(key));
                        current = current[idx];
                    }
                    else
                    {
                        current = current.get(key);
                    }
                }
                else
                {
                    size_t left = str.find(FIO_KEY_BEG, pos);
                    std::string key = trim(str.substr(pos, left == std::string::npos ? std::string::npos : left - pos));
                    if (!key.empty())
                        current = current.get(key);
                    if (left == std::string::npos)
                        break;
                    pos = left;
                }
            }
            return current;
        }
        namespace//文件读取 别乱用
        {
            
            node root;
            std::string text;
            std::string trim(const std::string& str)//跳过两端空白符号
            {
                if (str.empty())
                    return {};
                auto isWhitespace=[](char c)
                {
                    return (c==' '||c=='\n'||c=='\r'||c=='\t');
                };
                size_t begin=0,end=str.size()-1;
                //考虑一下用find
                while(begin<str.size() && isWhitespace(str[begin]))
                    begin++;
                while(end>begin && isWhitespace(str[end]))
                    end--;
                return str.substr(begin,end-begin+1);
            }
            void skipWs(std::string& str,size_t& pos)//跳过空白符号
            {
                auto isWhitespace=[](char c)
                {
                    return (c==' '||c=='\n'||c=='\r'||c=='\t');
                };
                while(pos<str.size()&&isWhitespace(str[pos]))
                    pos++;
                //感觉和trim很像 考虑一下
            }
            std::string readBracket(std::string& str,size_t& pos)//读取[]里的内容 键
            {
                skipWs(str,pos);
                if(pos>=str.size() || str[pos]!=FIO_KEY_BEG)
                    throw std::runtime_error(
                "parse error at pos:" + std::to_string(pos) +
                ", expected FIO_KEY_BEG");
                pos++;//从[后面开始读
                size_t tmp=pos;//第一个字
                while(pos<str.size()&&str[pos]!=FIO_KEY_END)
                {
                    pos++;
                    if(pos >= str.size())//没找到]
                    return "";
                }
                std::string res=str.substr(tmp,pos-tmp);
                pos++;//到]结束
                return trim(res);//中括号两边空白无效
            }
            std::string readQuote(const std::string& str, size_t& pos)
            {
                if (pos >= str.size() || str[pos] != FIO_STRING)
                    throw std::runtime_error(
                    "parse error at pos:" + std::to_string(pos) +
                    ", expected '\"'");
                ++pos;
                const size_t begin = pos;
                while (pos < str.size() && str[pos] != FIO_STRING)
                    ++pos;
                if (pos >= str.size())
                    throw std::runtime_error(
                    "parse error at pos:" + std::to_string(begin) +
                    ", missing closing quote");
                std::string result = str.substr(begin, pos - begin);
                ++pos;
                return result;
            }
            node parse(std::string& str,size_t& pos)
            {
                skipWs(str,pos);
                node Node;
                if (pos < str.size() && str[pos] == FIO_OBJECT_BEG) //读到对象
                {
                    pos++;
                    skipWs(str,pos);
                    while(pos<str.size()&&str[pos]!=FIO_OBJECT_END)
                    {
                        std::string key=readBracket(str,pos);//键对应一个对象
                        skipWs(str,pos);
                        //[]  here  =
                        if(str[pos]==FIO_SEPERATOR)
                            pos++;
                        skipWs(str,pos);
                        
                        Node.obj[key]=std::make_unique<node>(std::move(parse(str,pos)));//新建对象(递归)
                        skipWs(str,pos);
                        if(str[pos]==FIO_COMMA)
                            pos++;
                        skipWs(str,pos);
                    }
                    pos++;
                }
                else if(str[pos]==FIO_ARRAY_BEG)//读到数组
                {
                    pos++;
                    skipWs(str,pos);
                    while(pos<str.size()&&str[pos]!=FIO_ARRAY_END)
                    {
                        Node.arr.push_back(std::move(parse(str,pos)));//省mmr
                        skipWs(str,pos);
                        if(str[pos]==FIO_COMMA)
                            pos++;
                        skipWs(str,pos);
                    }
                    pos++;
                }
                else if(str[pos]==FIO_STRING)//读到字符串
                {
                    Node.val=readQuote(str,pos);
                    Node.type=valueType::Str;
                }
                else//除了符号外的任何字符 abc 114514之类的
                {
                    size_t tmp = pos;
                    while (pos < str.size()
                        && str[pos] != FIO_COMMA && str[pos] != FIO_OBJECT_END && str[pos] != FIO_ARRAY_END
                        && str[pos] != FIO_KEY_BEG && str[pos] != FIO_SEPERATOR)
                    {
                        pos++;
                    }
                    if (pos == tmp)
                        throw std::runtime_error("parse error at pos:"+std::to_string(pos)+" unexpected character '" + std::string(1, str[pos]) + "'");
                    //读取数字
                    std::string rawStr = trim(str.substr(tmp, pos - tmp));
                    if (rawStr.empty())
                        throw std::runtime_error("parse error: empty token");
                    bool isNum = true, dot = false;
                    size_t idx = 0;
                    if (!rawStr.empty() && rawStr[0] == '-')//允许负数 不支持多个前缀
                        idx++;
                    for (; idx < rawStr.size(); idx++)
                    {
                        if (std::isdigit(static_cast<unsigned char>(rawStr[idx])))//读到数字
                            continue;
                        if (rawStr[idx] == '.' && (!dot))//读到小数点
                        {
                            dot = true;
                            continue;
                        }
                        isNum = false;
                        break;//牌有问题
                    }
                    if (isNum)
                    {
                        if (dot)//有小数点就是小数
                        {
                            Node.val = std::stod(rawStr);
                            Node.type = valueType::F64;
                        }
                        else//整数
                        {
                            Node.val = std::stoll(rawStr);
                            Node.type = valueType::I64;
                        }
                    }
                    else//字符串
                    {
                        Node.val = rawStr;
                        Node.type = valueType::Str;
                    }
                }
                return Node;
            }
        }
        void parseRoot(std::string& str,size_t& pos)//根目录 只执行一次
        {
            skipWs(str,pos);
            while(pos<str.size())
            {
                skipWs(str,pos);
                if(pos>=str.size()) break;
                std::string topKey=readBracket(str,pos);
                if(topKey.empty()) {pos++;continue;}//空的?!
                skipWs(str,pos);
                if(str[pos]==FIO_SEPERATOR) pos++;
                skipWs(str,pos);
                root.obj[topKey]=std::make_unique<node>(std::move(parse(str,pos)));
                skipWs(str,pos);
                if(str[pos]==FIO_COMMA) pos++;
            }
        }
        std::vector<std::string> 
        nVectoStrVec(std::vector<node>& vec)
        {
            std::vector<std::string> ret;
            for(auto& i:vec)
            {
                ret.push_back(std::get<std::string>(i.val));
            }
            return ret;
        }
        void open(const std::string& path)
        {
            std::ifstream fin(path);
            if (!fin.is_open())
                throw std::runtime_error("open file failed!!!");
            std::stringstream ss;
            ss << fin.rdbuf();
            text = ss.str();
            fin.close();
            size_t pos = 0;
            root = node{};
            parseRoot(text, pos);
        }
        nodeView read(const std::string& topKey)
        {
            auto path = splitBracket(topKey);
            if (path.empty())
            {
                nodeView view(&root);
                return view.get(topKey);
            }

            nodeView view(&root);
            for (const std::string& seg : path)
            {
                if (KF::UTI::KMATH::isPosInt(seg))
                {
                    // 数字 → 走数组at
                    size_t idx = std::stoull(seg);
                    view = view.at(idx);
                }
                else
                {
                    // 普通key → 对象get
                    view = view.get(seg);
                }
            }
            return view;
        }

    }
}