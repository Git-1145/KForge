#include "kf.hpp"
namespace KF::UTI
{
    namespace FIO
    {
        
        nodeView nodeView::operator[](size_t index)
        {
            if (ptr == nullptr)
                throw std::runtime_error("cannot index null node");
            //std::cerr << "DEBUG operator[]: ptr=" << ptr << " is_array=" << ptr->is_array() 
            //      << " arr.size=" << ptr->arr.size() << " index=" << index << "\n";
            if (!ptr->is_array())
            {
            //    std::cerr << "DEBUG operator[] abort: not array\n";
                throw std::runtime_error("Unexpected index: current node is not an array");
            }
            if (index >= ptr->arr.size())
            {
            //    std::cerr << "DEBUG operator[] abort: out of range\n";
                throw std::runtime_error(
                "array out of range, size:" + std::to_string(ptr->arr.size()) +
                " idx:" + std::to_string(index));
            }
            //std::cerr << "DEBUG operator[] success: returning element\n";
            return nodeView(&ptr->arr[index]);
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
                size_t left = str.find('[', pos),
                       right = str.find(']', left + 1);
                if (left == std::string::npos) break;//没有项
                if (right == std::string::npos) throw std::runtime_error("path syntax error, missing ']'");
                std::string key = trim(str.substr(left + 1, right - left - 1));
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
                //    std::cerr << "DEBUG: numeric key='" << key << "' idx=" << idx << " current.is_array=" << current.is_array() << " current.is_object=" << current.is_object() << "\n";
                    current = current[idx];//如果是数字 则进入其列表的第idx项
                //    std::cerr << "DEBUG: after numeric current assignment, current.ptr=" << current.ptr << " current.is_array=" << current.is_array() << " current.is_object=" << current.is_object() << "\n";
                }
                else
                {
                //    std::cerr << "DEBUG: string key='" << key << "' current.is_array=" << current.is_array() << " current.is_object=" << current.is_object() << "\n";
                    current = current.get(key);//如果是字符串 则进入其子节点 named key        
                //    std::cerr << "DEBUG: after string current assignment, current.ptr=" << current.ptr << " current.is_array=" << current.is_array() << " current.is_object=" << current.is_object() << "\n";
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
                if(pos>=str.size() || str[pos]!='[')
                    throw std::runtime_error(
                "parse error at pos:" + std::to_string(pos) +
                ", expected '['");
                pos++;//从[后面开始读
                size_t tmp=pos;//第一个字
                while(pos<str.size()&&str[pos]!=']')
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
    if (pos >= str.size() || str[pos] != '"')
        throw std::runtime_error(
            "parse error at pos:" + std::to_string(pos) +
            ", expected '\"'");

    ++pos;
    const size_t begin = pos;

    while (pos < str.size() && str[pos] != '"')
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
                if (pos < str.size() && str[pos] == '{') //读到对象
                {
                    pos++;
                    skipWs(str,pos);
                    while(pos<str.size()&&str[pos]!='}')
                    {
                        std::string key=readBracket(str,pos);//键对应一个对象
                        skipWs(str,pos);
                        //[]  here  =
                        if(str[pos]=='=')
                            pos++;
                        skipWs(str,pos);
                        
                        Node.obj[key]=std::make_unique<node>(std::move(parse(str,pos)));//新建对象(递归)
                        skipWs(str,pos);
                        if(str[pos]==',')
                            pos++;
                        skipWs(str,pos);
                    }
                    pos++;
                }
                else if(str[pos]=='(')//读到数组
                {
                    pos++;
                    skipWs(str,pos);
                    while(pos<str.size()&&str[pos]!=')')
                    {
                        Node.arr.push_back(std::move(parse(str,pos)));//省mmr
                        skipWs(str,pos);
                        if(str[pos]==',')
                            pos++;
                        skipWs(str,pos);
                    }
                    pos++;
                }
                else if(str[pos]=='"')//读到字符串
                {
                    Node.val=readQuote(str,pos);
                    Node.type=valueType::Str;
                }
                else//除了符号外的任何字符 abc 114514之类的
                {
                    size_t tmp = pos;
                    while (pos < str.size()
                        && str[pos] != ',' && str[pos] != '}' && str[pos] != ')'
                        && str[pos] != '[' && str[pos] != '=')
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
                            try
                            {
                                Node.val = std::stod(rawStr);
                                Node.type = valueType::F64;
                            }
                            catch (const std::exception&)
                            {
                                Node.val = rawStr;
                                Node.type = valueType::Str;
                            }
                        }
                        else//整数
                        {
                            try
                            {
                                Node.val = std::stoll(rawStr);
                                Node.type = valueType::I64;
                            }
                            catch (const std::exception&)
                            {
                                Node.val = rawStr;
                                Node.type = valueType::Str;
                            }
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
                if(str[pos]=='=') pos++;
                skipWs(str,pos);
                root.obj[topKey]=std::make_unique<node>(std::move(parse(str,pos)));
                skipWs(str,pos);
                if(str[pos]==',') pos++;
            }
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
            nodeView view(&root);
            return view.get(topKey);
        }
    }


    
}