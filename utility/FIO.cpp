#include<utility/kf.hpp>
namespace KF::UTI
{
    namespace FIO
    {
        using nodePtr = std::unique_ptr<node>;
        using objMap = std::unordered_map<std::string, nodePtr>;
        struct node//节点
        {
            //基础数值
            objMap obj;//{} 孩子有哪些
            std::vector<node> arr;//()数组 node退化成variant<ll,str> 
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
        nodeView nodeView::operator[](size_t index)
        {
            if (index >= ptr->arr.size())
                throw std::runtime_error("array out of range, size:" + std::to_string(ptr->arr.size())+ " idx:" + std::to_string(index));
            return nodeView(&ptr->arr[index]);
        }
        namespace//文件读取 别乱用
        {
            
            node root;
            std::string text;
            std::string trim(const std::string& str)//跳过两端空白符号
            {
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
                return str.substr(begin,end-begin);
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
            std::string readBracket(std::string& str,size_t pos)//读取[]里的内容 键
            {
                skipWs(str,pos);
                if(pos>=str.size() || str[pos]!='[')
                    return "";
                pos++;//从[后面开始读
                size_t tmp=pos;//第一个字
                while(pos<str.size()&&str[pos]!=']')
                    pos++;
                std::string res=str.substr(tmp,pos-tmp);
                pos++;//到]结束
                return trim(res);//中括号两边空白无效
            }
            std::string readQuote(std::string& str,size_t pos)//读取""里的内容
            {
                //因为是字符串 所以不跳过空白
                pos++;//从第一个"后面开始
                size_t tmp=pos;
                while(pos<str.size()&&str[pos]!='"')
                    pos++;
                std::string res = str.substr(tmp, pos - tmp);
                ++pos;
                return res;
                /*
                return str.substr(tmp,(pos++)-tmp);
                这样写可能有bug 以后测试一下
                */
            }
            node parse(std::string& str,size_t& pos)
            {
                skipWs(str,pos);
                node Node;
                if(str[pos]='{')//读到对象
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
                    size_t tmp=pos;
                    while (pos < str.size()
                    && str[pos] != ',' && str[pos] != '}' && str[pos] != ')'
                    && str[pos] != '[' && str[pos] != '=')
                    {
                        pos++;
                    }
                    //读取数字
                    std::string rawStr=trim(str.substr(tmp,pos-tmp));
                    bool isNum=true,dot=false;
                    size_t idx=0;
                    if(rawStr[0]='-')//允许负数 不支持多个前缀
                        idx++;
                    for(;idx<rawStr.size();idx++)
                    {
                        if(std::isdigit(rawStr[idx]))//读到数字
                            continue;
                        if(rawStr[idx]=='.' && (!dot))//读到小数点
                        {
                            dot=true;
                            continue;
                        }
                        isNum=false;
                        break;//牌有问题
                    }
                    if(isNum)
                    {
                        if(dot)//有小数点就是小数
                        {
                            Node.val=std::stod(rawStr);
                            Node.type=valueType::F64;
                        }
                        else//整数
                        {
                            Node.val=std::stoll(rawStr);
                            Node.type=valueType::I64;
                        }
                    }
                    else//字符串
                    {
                        Node.val=rawStr;
                        Node.type=valueType::Str;
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
    }


    
}