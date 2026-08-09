/*
namespace KBIGNUM
    {
        using limb = uint32_t; // 基础分块
        using dlimb = uint64_t; // 扩展分块 运算时会用
        class BigNum
        {
            private:
                std::vector<limb> limbs; // 存储 (无小数点 无符号)
                /// @attention base = 10 ^ 9 ,之所以不用 2^32 是因为这 ToStr 太麻烦且太慢
                bool isneg = false; // 是否为负数
                enum class Type { Int, Dec };// 类型(暂不支持分数 毕竟不是专业大数库)
                std::string Normalize(const std::string& str); //合法化 包括但不限于去小数点 去前后导0
                BigNum ToBig(const std::string& str); // 字符串转大数
                std::string ToStr() const; // 大数转字符串
            public:
                /// @brief 构造 支持空 字符串 数字
                BigNum() = default;
                BigNum(const std::string& str);// 用字符串构造
                BigNum(const dlimb& num);// 用数字构造

                /// @brief 面向内部的运算 (不放在private里是为了Debug test)
                BigNum AbsAdd(const BigNum& b) ;
                BigNum AbsSub(const BigNum& b);
                BigNum AbsMul(const BigNum& b);
                BigNum AbsDiv(const BigNum& b);
                BigNum AbsMod(const BigNum& b);
                int    AbsCmp(const BigNum& b);
                BigNum AbsPow(const BigNum& b);

                /// @brief 面向用户的运算
                BigNum operator+(const BigNum& b) const;
                BigNum operator-(const BigNum& b) const;
                BigNum operator*(const BigNum& b) const;
                BigNum operator/(const BigNum& b) const;
                BigNum operator%(const BigNum& b) const;
                BigNum Pow(const BigNum& b) const;
                /// @brief 比较 (由于这是c++ 17 所以不使用三目运算符)
                bool operator==(const BigNum& b) const;
                bool operator!=(const BigNum& b) const;
                bool operator<(const BigNum& b) const;
                bool operator<=(const BigNum& b) const;
                bool operator>(const BigNum& b) const;
                bool operator>=(const BigNum& b) const;

                /// @brief 输出
                friend std::ostream& operator<<(std::ostream& os, const BigNum& b)
                {
                    os << b.ToStr();
                    return os;
                }
        };
    }
*/
#include "Kf.hpp"
namespace KF
{
    namespace KBIGNUM
    {
        constexpr char CHAR_NEG = '-';
        constexpr char CHAR_POS = '+';
        constexpr char CHAR_DOT = '.';
        constexpr int64_t BASE = 1000000000ULL;
        constexpr int64_t BASEEXP = 9;

        BigNum::BigNum(const std::string& str)
        {
            *this = ToBig(Normalize(str));
        }
        BigNum::BigNum(const dlimb& num)
        {
            *this = ToBig(std::to_string(num));
        }
        /// @brief 将任意字符串转换成合法的数字 
        std::string Normalize(const std::string& str)
        {
            ///@date 2026-08-08

            size_t start = 0,end = 0; //小数点位置 合法开始 结束位置 读取 写入位置(预留一个符号位)
            ///@brief 首先去掉前后的0
            start = str.find_first_of("-+.0123456789");
            if (start == std::string::npos) return "0"; // 如果合法数字全都没有 那肯定是0
            end = str.find_last_of(".0123456789"); // 找到最后一个数字字符
            if (end == std::string::npos)   return "0";
            /// @brief 逐位检查
            std::string res;
            res.resize(end - start + 2); //预留一个符号位
            bool dot = false; //是否有小数点
            bool isneg = false;// false为正数 true为负数
            bool numstart = 0; //是否开始有数字 0 = 无数字,1 = 有数字
            size_t WritePos = 1,ReadPos = start;
            while(ReadPos <= end)
            {
                if(isdigit(static_cast<unsigned char>(str[ReadPos]))) //如果是数字
                {
                    if(str[ReadPos] == '0' && !numstart && !dot) //如果是0 且还没有数字 跳过
                    {
                        ReadPos ++;
                        continue;
                    }
                    numstart = true;
                    res[WritePos++] = str[ReadPos++];
                    continue;
                }
                else if (str[ReadPos] == CHAR_DOT && !dot) //如果是小数点
                {
                    if(res.empty() || numstart == 0) //处理 -.123 这种情况 前面补一个0
                    {
                        res.insert(WritePos,1,'0');
                        WritePos++;
                    }
                    dot = true;
                    res[WritePos++] = CHAR_DOT;
                    ReadPos++;
                    continue;
                }
                else if(!numstart && (str[ReadPos] == CHAR_NEG || str[ReadPos] == CHAR_POS))//如果是正负号 且还没有读到数字
                {
                    if(str[ReadPos] == CHAR_NEG) //如果是符号
                        isneg ^= 1; // 翻转正负号
                    ReadPos++;
                    continue;
                }
                else //如果是其他字符
                {
                    //KLOG_WARNING(KBIGNUM_INVALIDCHAR,"");
                    ReadPos++;
                }
            }
            if(!numstart) //一个有效数字都没有
                return "0";
            res[0] = isneg ? CHAR_NEG : CHAR_POS;
            res.resize(WritePos);
            // 去掉末尾小数点
            if(!res.empty() && res.back() == CHAR_DOT)
                res.pop_back();
            // 去掉小数点后的尾随0
            if(dot)
            {
                while(!res.empty() && res.back() == '0')
                    res.pop_back();
                if(!res.empty() && res.back() == CHAR_DOT)
                    res.pop_back();
            }
            // 检查是否为0
            if(res.size() <= 1) return "0";
            bool isZero = true;
            for(size_t i = 1; i < res.size(); i++)
            {
                if(res[i] != '0') { isZero = false; break; }
            }
            if(isZero) return "0";
            return std::move(res);
        }

        /// @brief 将字符串转换为BigNum
        BigNum BigNum::ToBig(const std::string& str)
        {
            BigNum res;
            res.limbs.clear();

            // 符号位
            res.isneg = (str[0] == CHAR_NEG);

            // 小数点位置 → scale
            size_t dot = str.find(CHAR_DOT);
            res.scale = (dot == std::string::npos) ? 0 : (str.size() - dot - 1);

            // 提取纯数字串（跳过符号位和小数点）
            std::string digits;
            digits.reserve(str.size());
            for(size_t i = 1; i < str.size(); i++)
                if(str[i] != CHAR_DOT)
                    digits += str[i];

            if(digits.empty())
            {
                res.limbs.push_back(0);
                res.isneg = false;
                return res;
            }

            // 从右往左，每九位截取一个 limb
            dlimb pos = static_cast<dlimb>(digits.size());
            while(pos > 0)
            {
                dlimb start = (pos >= BASEEXP) ? (pos - BASEEXP) : 0;
                res.limbs.push_back(static_cast<limb>(
                    std::stoll(digits.substr(static_cast<size_t>(start),
                                             static_cast<size_t>(pos - start)))));
                pos = start;
            }

            // 去掉高位多余的0块
            while(res.limbs.size() > 1 && res.limbs.back() == 0)
                res.limbs.pop_back();

            // 零不是负数
            if(res.limbs.size() == 1 && res.limbs[0] == 0)
                res.isneg = false;

            return res;
        }
        /// @brief 将BigNum转换为字符串 

        std::string BigNum::ToStr() const
        {
            // 处理0
            if(limbs.size() == 1 && limbs[0] == 0)
                return "0";
            // 从最高位limb开始拼接数字字符串
            std::string digits;
            digits += std::to_string(limbs.back()); // 最高位不补0
            for(int i = static_cast<int>(limbs.size()) - 2; i >= 0; i--)
            {
                std::string chunk = std::to_string(limbs[i]);
                chunk = std::string(static_cast<size_t>(BASEEXP) - chunk.size(), '0') + chunk;
                digits += chunk;
            }
            // 根据 scale 插入小数点
            std::string result;
            if(scale > 0)
            {
                if(scale >= digits.size())// 0.00...digits
                    result = "0." + std::string(scale - digits.size(), '0') + digits;
                else
                    result = digits.substr(0, digits.size() - scale) + "." + digits.substr(digits.size() - scale);
            }
            else
                result = digits;
            // 加符号
            if(isneg)
                result = "-" + result;
            return result;
        }
        
        /// @brief 大数比较(无符号)
        slimb AbsCmp(const BigNum& a,const BigNum& b)
        {
            if(a.limbs.size() > b.limbs.size()) // 位数 a比b多 a大
                return 1;
            else if(a.limbs.size() < b.limbs.size())// b大
                return -1;
            else // 位数相等
            {
                for(int i = static_cast<int>(a.limbs.size()) - 1; i >= 0; i--) //逐位看 
                {
                    if(a.limbs[i] > b.limbs[i])
                        return 1;
                    else if(a.limbs[i] < b.limbs[i])
                        return -1;
                }
            }
            return 0;// 相等
        }

        /// @brief 内部大数加法 (无符号)
        BigNum AbsAdd(const BigNum& a,const BigNum& b)
        {
            BigNum res;
            res.limbs.resize((std::max)(a.limbs.size(), b.limbs.size()) + 1);
            for(size_t i = 0; i < res.limbs.size(); i++)
            {
                //自动补 0
                res.limbs[i] = (i < a.limbs.size() ? a.limbs[i] : 0) + (i < b.limbs.size() ? b.limbs[i] : 0);
                if(res.limbs[i] >= BASE)
                {
                    res.limbs[i] -= BASE;
                    res.limbs[i + 1]++;
                }
            }
            // 删除高位 0
            while(res.limbs.size() > 1 && res.limbs.back() == 0)
                res.limbs.pop_back();
            return std::move(res);
        }
        /// @brief  大数相减(无符号)
        BigNum AbsSub(const BigNum& a,const BigNum& b)
        {
            /// @attention a >= b
            BigNum res;
            if(a < b)
                return AbsSub(b, a);
            res.limbs.resize(a.limbs.size());
            for(size_t i = 0; i < res.limbs.size(); i++)
            {
                slimb diff = (i < a.limbs.size() ? a.limbs[i] : 0) - (i < b.limbs.size() ? b.limbs[i] : 0);
                /// @brief 差 可能是负数 所以是signed limb
                if(diff < 0)
                {
                    res.limbs[i] += BASE;
                    res.limbs[i + 1]--;
                }
                else
                    res.limbs[i] = diff;
                /// @brief 删除高位 0
                while(res.limbs.size() > 1 && res.limbs.back() == 0)
                    res.limbs.pop_back();
            }
            return std::move(res);
        }
    }
}