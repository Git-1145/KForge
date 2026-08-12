#include "base/KF.hpp"
namespace KF
{
    namespace KBIGNUM
    {
        constexpr char CHAR_NEG = '-';
        constexpr char CHAR_POS = '+';
        constexpr char CHAR_DOT = '.';
        constexpr int64_t BASE = 1000000000ULL;
        constexpr int64_t BASEEXP = 9;
        constexpr size_t THRESHOLD_SCHOOL_LIMBS = 48;
        BigNum::BigNum(const std::string& str)
        {
            *this = ToBig(Normalize(str));
        }
        BigNum::BigNum(const dlimb& num)
        {
            *this = ToBig(std::to_string(num));
        }
        /// @brief  生成随机大数(符号 整数位数 小数位数)
        BigNum RandBigNum(bool isneg, size_t IntSize, size_t DecSize)
        {
            //整数小数长度都为0，直接返回0
            if (IntSize == 0 && DecSize == 0)
                return BigNum("0");

            BigNum res;
            res.limbs.clear();
            res.isneg = isneg;
            res.scale = DecSize;
            const size_t Total = IntSize + DecSize;
            size_t WritePtr = 0; // 已生成的位数（从最低位开始）
            while (WritePtr < Total)
            {
                const size_t remain = Total - WritePtr;
                const size_t genLen = (remain < BASEEXP) ? remain : BASEEXP;
                const sdlimb maxVal = KUTIL::Pow10(genLen) - 1;
                sdlimb num;
                // 确保首位不为0
                if (WritePtr + genLen == Total && IntSize > 0)
                    num = KUTIL::RandInt(KUTIL::Pow10(genLen - 1), maxVal);
                else
                    num = KUTIL::RandInt(0, maxVal);
                res.limbs.push_back(static_cast<limb>(num));
                WritePtr += genLen;
            }
            return res;
        }

        /// @brief 将任意字符串转换成合法的数字 
        std::string Normalize(const std::string& str)
        {
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
            return res;
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
            for(sdlimb i = static_cast<sdlimb>(limbs.size()) - 2; i >= 0; i--) //千万不是size_t
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
            // 去掉末尾的0
            if(scale > 0)
            {
                while(!result.empty() && result.back() == '0')
                    result.pop_back();
            }
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
            dlimb carry = 0; // 进位，跨 limb 传递
            for(size_t i = 0; i < res.limbs.size(); i++)
            {
                dlimb sum = (dlimb)(i < a.limbs.size() ? a.limbs[i] : 0)
                          + (i < b.limbs.size() ? b.limbs[i] : 0)
                          + carry;
                res.limbs[i] = static_cast<limb>(sum % BASE);
                carry = sum / BASE;
            }
            // 删除高位 0
            while(res.limbs.size() > 1 && res.limbs.back() == 0)
                res.limbs.pop_back();
            return res;
        }
        /// @brief  大数相减(无符号，要求 |a| >= |b|)
        BigNum AbsSub(const BigNum& a,const BigNum& b)
        {
            if(a < b)
                return AbsSub(b, a);
            BigNum res;
            res.limbs.resize(a.limbs.size());
            slimb borrow = 0; // 借位，跨 limb 传递
            for(size_t i = 0; i < res.limbs.size(); i++)
            {
                slimb diff = (i < a.limbs.size() ? (slimb)a.limbs[i] : 0)
                           - (i < b.limbs.size() ? (slimb)b.limbs[i] : 0)
                           - borrow;
                if(diff < 0)
                {
                    res.limbs[i] = static_cast<limb>(diff + BASE);
                    borrow = 1;
                }
                else
                {
                    res.limbs[i] = static_cast<limb>(diff);
                    borrow = 0;
                }
            }
            // 删除高位 0
            while(res.limbs.size() > 1 && res.limbs.back() == 0)
                res.limbs.pop_back();
            return res;
        }
        /// @brief 大数乘法 (无符号)
        BigNum AbsMul(const BigNum& a,const BigNum& b)
        {
            size_t len = a.limbs.size() > b.limbs.size() ? a.limbs.size() : b.limbs.size();//较大值
            if(len < THRESHOLD_SCHOOL_LIMBS) // limb数小于48使用朴素算法
                return AbsMulSchool(a,b);
            else // 其他算法暂不支持 以后这里会是NTT算法
                return AbsMulSchool(a,b);
        }
        BigNum AbsMul(const BigNum& a,const BigNum& b,size_t method) //选择算法
        {
            // 后续算法暂未实现，全部回退到朴素乘法
            (void)method;
            return AbsMulSchool(a,b);
        }
        //======================================================================
        /// @date 2026 8 10 
        /// @brief 朴素乘法
        BigNum AbsMulSchool(const BigNum& a, const BigNum& b)
        {
            BigNum res;
            const size_t lenA = a.limbs.size();
            const size_t lenB = b.limbs.size();

            // 预分配：lenA+lenB 是乘积最大 limb 数，+3 给行尾进位留足空间
            //   例: 999 × 999 = 998001  (最多 6 位)
            //       999 × 9999 = 9989001 (最多 7 位)
            res.limbs.resize(lenA + lenB + 3, 0);

            for (size_t i = 0; i < lenA; i++)
            {
                const limb aval = a.limbs[i];      // a 的第 i 个 limb（little-endian，低位在前）
                dlimb carry = 0;                    // 行内累积进位，64 位保证单步不溢出

                for (size_t j = 0; j < lenB; j++)
                {
                    const limb bval = b.limbs[j];  // b 的第 j 个 limb
                    const size_t pos = i + j;       // 乘积中的目标位置

                    // 64 位累加：aval × bval + 原有的低位 + 上一步的进位
                    const dlimb total = (dlimb)aval * bval + res.limbs[pos] + carry;

                    res.limbs[pos] = static_cast<limb>(total % BASE);  // 本位
                    carry          = total / BASE;                     // 进位到下一位
                }

                // 处理carry 最多两次
                size_t k = i + lenB;
                while (carry != 0)
                {
                    const dlimb total = (dlimb)res.limbs[k] + carry;
                    res.limbs[k] = static_cast<limb>(total % BASE);
                    carry = total / BASE;
                    ++k;
                }
            }
            while (res.limbs.size() > 1 && res.limbs.back() == 0)
                res.limbs.pop_back();
            return res;
        }
        //======================================================================
        BigNum BigNum::operator+(const BigNum& b) const
        {
            BigNum res;
            size_t dec = scale > b.scale ? scale : b.scale;//取较大值
            if(isneg == b.isneg) // 如果同号 直接加
                res = AbsAdd(*this, b);
            else if(isneg) // 如果a是负数 b是正数 则b>a 
                res = AbsSub(b, *this);
            else// a是正数 b是负数 则a>b
                res = AbsSub(*this, b);
            res.scale = dec;
            return res;
        }
        BigNum BigNum::operator-(const BigNum& b) const
        {
            BigNum res;
            size_t dec = scale > b.scale ? scale : b.scale;//取较大值
            if(isneg != b.isneg) // 如果异号 直接减
                res = AbsAdd(*this, b);
            else if(isneg) // 如果a是负数 b是正数 则a<b
                res = AbsSub(*this, b);
            else// a是正数 b是负数 则a>b
                res = AbsSub(b, *this);
            res.scale = dec;
            return res;
        }
        BigNum BigNum::operator*(const BigNum& b) const
        {
            BigNum res;
            res = AbsMul(*this, b);
            res.isneg = isneg ^ b.isneg; //相同就是正，不同就是负
            res.scale = scale + b.scale;
            return res;
        }
    }
}