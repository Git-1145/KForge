#include "base/KF.hpp"
/**
 * @file KBIGNUM.cpp
 * @brief 大数模块
 * @version 1.1.0
 * @date 2026-08-14
 * @author Git-1145
**/
namespace KF
{
    namespace KBIGNUM
    {

        // 定义常量
        constexpr char CHAR_NEG = '-';    // 负号常量
        constexpr char CHAR_POS = '+';    // 正号常量
        constexpr char CHAR_DOT = '.';    // 小数点常量
        constexpr size_t PRECISION = 9;
        constexpr int64_t BASE = 1000000000ULL;  // 基数，用于大数存储
        constexpr int64_t BASEEXP = 9;    // 基数的指数，表示每个limb存储的位数
        constexpr size_t THRESHOLD_SCHOOL_LIMBS = 48;  // 使用朴素乘法的阈值
        /**
         * @brief 构造函数：从字符串构造大数
         * @param str 输入的数字字符串（inf/-inf/nan 大小写不敏感，容忍首尾空白）
         */
        BigNum::BigNum(const std::string& str)
        {
            // 先识别特殊状态：inf / -inf / nan（大小写不敏感，容忍首尾空白）
            std::string low = str;
            size_t b = 0, e = low.size();
            while (b < e && isspace(static_cast<unsigned char>(low[b]))) b++;
            while (e > b && isspace(static_cast<unsigned char>(low[e - 1]))) e--;
            for (size_t i = b; i < e; i++)
                low[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(low[i])));
            const std::string tok = low.substr(b, e - b);
            if (tok == "inf" || tok == "+inf")  { state = State::Inf;    return; }
            if (tok == "-inf")                  { state = State::NegInf; return; }
            if (tok == "nan")                   { state = State::Nan;    return; }

            *this = ToBig(Normalize(str));
        }
        /**
         * @brief 构造函数：从特殊状态构造（Inf / NegInf / Nan）
         * @param s 特殊状态枚举
         */
        BigNum::BigNum(State s)
        {
            state = s;
            limbs = {0};
            isneg = (s == State::NegInf);
            scale = 0;
        }
        /**
         * @brief 构造函数：从64位整数构造大数
         * @param num 64位整数
         */
        BigNum::BigNum(const dlimb& num)
        {
            *this = ToBig(std::to_string(num));
        }
        /// @brief  生成随机大数(整数位数范围 小数位数范围 符号:0随机/1全正/2全负)
        BigNum RandBigNum(std::pair<size_t,size_t> IntRand, std::pair<size_t,size_t> DecRand, int sign)
        {
            // 范围自动矫正(min>max 时交换)
            if (IntRand.second < IntRand.first) std::swap(IntRand.first, IntRand.second);
            if (DecRand.second < DecRand.first) std::swap(DecRand.first, DecRand.second);

            // 符号位: 1全正 / 2全负 / 其他(含0)随机
            bool isneg;
            if (sign == 1)      isneg = false;
            else if (sign == 2) isneg = true;
            else                isneg = (KUTIL::RandInt(0, 1) == 1);

            // 位数: 在范围内随机取值(含端点); 上界为0则固定为0
            const size_t IntSize = IntRand.second == 0 ? 0 : KUTIL::RandInt(IntRand.first, IntRand.second);
            const size_t DecSize = DecRand.second == 0 ? 0 : KUTIL::RandInt(DecRand.first, DecRand.second);

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
            // 特殊状态
            if(state == State::Nan)    return "nan";
            if(state == State::Inf)    return "inf";
            if(state == State::NegInf) return "-inf";
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
                // 去掉孤立的小数点（如 "-3.0" -> "-3"）
                if(!result.empty() && result.back() == '.')
                    result.pop_back();
            }
            return result;
        }
        
        /// @brief 对齐小数位 把x 变成 Newscale 位的
        BigNum ScaleTo(const BigNum& x, size_t NewScale)
        {
            if(x.scale >= NewScale) { BigNum r = x; r.scale = NewScale; return r; }
            size_t k = NewScale - x.scale; // 差了k位 需乘以 10^k
            BigNum r = x;
            // 0 还是 0
            if(r.limbs.size() == 1 && r.limbs[0] == 0) { r.scale = NewScale; return r; }
            // 先乘 10^(k%9)：低位进位 通过乘实现进位
            const size_t low = k % BASEEXP;
            if(low > 0)
            {
                const dlimb mult = KUTIL::Pow10(low);
                dlimb carry = 0;
                for(size_t i = 0; i < r.limbs.size(); i++)
                {
                    dlimb v = (dlimb)r.limbs[i] * mult + carry;
                    r.limbs[i] = static_cast<limb>(v % BASE);
                    carry = v / BASE;
                }
                if(carry) r.limbs.push_back(static_cast<limb>(carry));
            }
            // 再补 k/9 个高位零块（等价于乘以 10^9）：必须在低位端前置零块做移位
            // 不能用 push_back(0)（那只是高位补零、不改变数值），否则跨 limb 边界会错误
            r.limbs.insert(r.limbs.begin(), k / BASEEXP, 0);
            // 去多余 0
            while(r.limbs.size() > 1 && r.limbs.back() == 0)
                r.limbs.pop_back();
            r.scale = NewScale;
            return r;
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

        /// @brief 内部大数加法 (无符号，自动对齐小数位)
        BigNum AbsAdd(const BigNum& a,const BigNum& b)
        {
            const size_t smax = (std::max)(a.scale, b.scale);
            BigNum aa = ScaleTo(a, smax);
            BigNum bb = ScaleTo(b, smax);
            BigNum res;
            res.limbs.resize((std::max)(aa.limbs.size(), bb.limbs.size()) + 1);
            dlimb carry = 0; // 进位，跨 limb 传递
            for(size_t i = 0; i < res.limbs.size(); i++)
            {
                dlimb sum = (dlimb)(i < aa.limbs.size() ? aa.limbs[i] : 0)
                          + (i < bb.limbs.size() ? bb.limbs[i] : 0)
                          + carry;
                res.limbs[i] = static_cast<limb>(sum % BASE);
                carry = sum / BASE;
            }
            // 删除高位 0
            while(res.limbs.size() > 1 && res.limbs.back() == 0)
                res.limbs.pop_back();
            res.scale = smax;
            return res;
        }
        /// @brief  大数相减(无符号，要求 |a| >= |b|，自动对齐小数位)
        BigNum AbsSub(const BigNum& a,const BigNum& b)
        {
            // 用绝对值确定谁大，保证 |a| >= |b|（不能用带符号 operator<，否则负数会错位）
            const size_t smax = (std::max)(a.scale, b.scale);
            if(AbsCmp(ScaleTo(a, smax), ScaleTo(b, smax)) < 0)
                return AbsSub(b, a);
            BigNum aa = ScaleTo(a, smax);
            BigNum bb = ScaleTo(b, smax);
            BigNum res;
            res.limbs.resize(aa.limbs.size());
            slimb borrow = 0; // 借位，跨 limb 传递
            for(size_t i = 0; i < res.limbs.size(); i++)
            {
                slimb diff = (slimb)aa.limbs[i]
                           - (i < bb.limbs.size() ? (slimb)bb.limbs[i] : 0)
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
            res.scale = smax;
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
        BigNum AbsDiv(const BigNum& a,const BigNum& b, size_t keep)
        {
            // 自动检测：a、b 均为整数且能整除 → 返回整数结果（整除则整数）
            if(a.scale == 0 && b.scale == 0)
            {
                BigNum q = AbsDivSchool(a, b, 0); // 整数商
                if(AbsMul(q, b) == a)             // 能整除
                    return q;
            }
            // 否则：保留 keep 位小数（默认 9 位）
            return AbsDivSchool(a, b, keep);
        }
        BigNum AbsMul(const BigNum& a,const BigNum& b,size_t method) //选择算法
        {
            // 后续算法暂未实现，全部回退到朴素乘法
            (void)method;
            return AbsMulSchool(a,b);
        }
        //======================================================================
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
                const limb aval = a.limbs[i];      // a 的第 i 个 limb
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
            res.scale = a.scale + b.scale; // 乘积的小数位 = 两因数小数位之和
            return res;
        }
        /// @brief 除法（朴素算法）：abs(a) / abs(b)，保留 keep 位小数（默认 9 位）
        /// @param a 被除数 / b 除数（仅用 limbs，忽略符号）
        /// @param keep 保留的小数位数，0 表示只取整数部分
        /// @return 商（limbs，scale=a.scale+keep，isneg=false）
        BigNum AbsDivSchool(const BigNum& a,const BigNum& b, size_t keep)
        {
            BigNum t; t.limbs = a.limbs; t.scale = 0; t.isneg = false;
            t = ScaleTo(t, b.scale + keep); // t.limbs = a.limbs * 10^(b.scale+keep)
            const size_t n = t.limbs.size(), m = b.limbs.size();
            BigNum Q; Q.scale = 0; Q.isneg = false;
            // 除数 0 或 被除数 < 除数 → 商 0
            if(m == 1 && b.limbs[0] == 0) { Q.limbs = {0}; Q.scale = a.scale + keep; return Q; }
            if(AbsCmp(t, b) < 0)          { Q.limbs = {0}; Q.scale = a.scale + keep; return Q; }
            const size_t qlen = n - m + 1; // 商的 limb 数
            Q.limbs.assign(qlen, 0);
            BigNum rem; rem.limbs = {0}; rem.scale = 0; rem.isneg = false; // 当前余数
            for(sdlimb i = static_cast<sdlimb>(n) - 1; i >= 0; i--)
            {
                // 余数左移一块（×10^9）再叠加 t[i]：必须在低位端前置零块做移位
                rem.limbs.insert(rem.limbs.begin(), 0);
                rem.limbs[0] = t.limbs[i];
                // 去掉前置移位产生的高位 0 块（否则 AbsCmp 先比 limb 数量会误判大小）
                while(rem.limbs.size() > 1 && rem.limbs.back() == 0)
                    rem.limbs.pop_back();
                // 二分查找最大 d ∈ [0, BASE-1] 使 d*b <= rem
                slimb lo = 0, hi = static_cast<slimb>(BASE); // 左闭右开
                while(lo + 1 < hi)
                {
                    slimb mid = lo + (hi - lo) / 2;
                    // 内联单块乘法：b * mid
                    BigNum mul; mul.scale = 0; mul.isneg = false;
                    mul.limbs.assign(b.limbs.size(), 0);
                    dlimb carry = 0;
                    for(size_t j = 0; j < b.limbs.size(); j++)
                    {
                        dlimb v = (dlimb)b.limbs[j] * static_cast<dlimb>(mid) + carry;
                        mul.limbs[j] = static_cast<limb>(v % BASE);
                        carry = v / BASE;
                    }
                    while(carry) { mul.limbs.push_back(static_cast<limb>(carry % BASE)); carry /= BASE; }
                    while(mul.limbs.size() > 1 && mul.limbs.back() == 0) mul.limbs.pop_back();
                    if(AbsCmp(mul, rem) <= 0)
                        lo = mid;
                    else
                        hi = mid;
                }
                if(i < static_cast<sdlimb>(qlen)) // 前 (m-1) 块必为前导 0，不存
                    Q.limbs[static_cast<size_t>(i)] = static_cast<limb>(lo);
                if(lo > 0)
                {
                    // 内联单块乘法：b * lo，并从余数中减去
                    BigNum mul; mul.scale = 0; mul.isneg = false;
                    mul.limbs.assign(b.limbs.size(), 0);
                    dlimb carry = 0;
                    for(size_t j = 0; j < b.limbs.size(); j++)
                    {
                        dlimb v = (dlimb)b.limbs[j] * static_cast<dlimb>(lo) + carry;
                        mul.limbs[j] = static_cast<limb>(v % BASE);
                        carry = v / BASE;
                    }
                    while(carry) { mul.limbs.push_back(static_cast<limb>(carry % BASE)); carry /= BASE; }
                    while(mul.limbs.size() > 1 && mul.limbs.back() == 0) mul.limbs.pop_back();
                    rem = AbsSub(rem, mul);
                }
            }
            // 去高位 0
            while(Q.limbs.size() > 1 && Q.limbs.back() == 0) Q.limbs.pop_back();
            Q.scale = a.scale + keep;
            Q.isneg = false;
            return Q;
        }
        /// @brief 取模（除法取余）：abs(a) mod abs(b) = abs(a) - trunc(abs(a)/abs(b))*abs(b)
        /// @return 余数（非负，符号由 operator% 按被除数设置）
        BigNum AbsMod(const BigNum& a,const BigNum& b)
        {
            // 对齐小数位后做整数取余：a mod b = (A' mod B') / 10^smax
            const size_t smax = (std::max)(a.scale, b.scale);
            BigNum aa = ScaleTo(a, smax); // 整数化被除数
            BigNum bb = ScaleTo(b, smax); // 整数化除数
            aa.scale = 0; bb.scale = 0;   // 视作纯整数
            BigNum q = AbsDivSchool(aa, bb, 0);       // 整数商（截断）
            BigNum r = AbsSub(aa, AbsMul(q, bb));     // 余数（整数）
            r.scale = smax;                            // 还原小数位
            r.isneg = false;
            return r;
        }
        //======================================================================
        BigNum BigNum::operator+(const BigNum& b) const
        {
            // 特殊状态：任一 NaN → NaN；inf + (-inf) → NaN；其余取无穷
            if(state == State::Nan || b.state == State::Nan)
                return BigNum(State::Nan);
            if(state != State::Normal || b.state != State::Normal)
            {
                if(IsInf() && b.IsInf() && state != b.state) // inf + (-inf) → nan
                    return BigNum(State::Nan);
                return BigNum(state != State::Normal ? state : b.state);
            }
            BigNum res;
            if(isneg == b.isneg) // 同号：绝对值相加，符号不变
            {
                res = AbsAdd(*this, b);
                res.isneg = isneg;
            }
            else // 异号：取绝对值大的，符号跟它
            {
                const size_t smax = (std::max)(scale, b.scale);
                const slimb c = AbsCmp(ScaleTo(*this, smax), ScaleTo(b, smax));
                if(c == 0) return BigNum("0");
                if(c > 0) { res = AbsSub(*this, b); res.isneg = isneg; }
                else      { res = AbsSub(b, *this); res.isneg = b.isneg; }
            }
            return res;
        }
        BigNum BigNum::operator-(const BigNum& b) const
        {
            // 特殊状态：任一 NaN → NaN；inf - inf → NaN；其余按 a + (-b) 处理
            if(state == State::Nan || b.state == State::Nan)
                return BigNum(State::Nan);
            if(state != State::Normal || b.state != State::Normal)
            {
                if(IsInf() && b.IsInf() && state == b.state) // inf - inf → nan
                    return BigNum(State::Nan);
                if(state != State::Normal) // 自身是无穷：结果 = 自身无穷
                    return BigNum(state);
                // 自身有限，b 是无穷：a - inf = ∓inf
                return BigNum(b.state == State::Inf ? State::NegInf : State::Inf);
            }
            BigNum res;
            if(isneg != b.isneg) // 异号：绝对值相加，符号跟 a（a - b = a + (-b)）
            {
                res = AbsAdd(*this, b);
                res.isneg = isneg;
            }
            else // 同号：绝对值相减，符号看绝对值大小
            {
                const size_t smax = (std::max)(scale, b.scale);
                const slimb c = AbsCmp(ScaleTo(*this, smax), ScaleTo(b, smax));
                if(c == 0) return BigNum("0");
                if(c > 0) { res = AbsSub(*this, b); res.isneg = isneg; }
                else      { res = AbsSub(b, *this); res.isneg = !isneg; }
            }
            return res;
        }
        BigNum BigNum::operator*(const BigNum& b) const
        {
            // 特殊状态：任一 NaN → NaN；inf × 0 → NaN；±inf × ±非零 → ±inf
            if(state == State::Nan || b.state == State::Nan)
                return BigNum(State::Nan);
            const bool aInf = IsInf(), bInf = b.IsInf();
            if(aInf || bInf)
            {
                const bool aZero = state == State::Normal && limbs.size() == 1 && limbs[0] == 0;
                const bool bZero = b.state == State::Normal && b.limbs.size() == 1 && b.limbs[0] == 0;
                if((aInf && bZero) || (bInf && aZero)) // inf × 0 → nan
                    return BigNum(State::Nan);
                const bool aNeg = (state == State::NegInf) || (state == State::Normal && isneg);
                const bool bNeg = (b.state == State::NegInf) || (b.state == State::Normal && b.isneg);
                return BigNum((aNeg ^ bNeg) ? State::NegInf : State::Inf);
            }
            BigNum res;
            res = AbsMul(*this, b);
            res.isneg = isneg ^ b.isneg; //相同就是正，不同就是负
            res.scale = scale + b.scale;
            return res;
        }
        BigNum BigNum::operator/(const BigNum& b) const
        {
            // 特殊状态：任一 NaN → NaN；inf/inf → NaN；±inf/有限 → ±inf；有限/±inf → 0
            if(state == State::Nan || b.state == State::Nan)
                return BigNum(State::Nan);
            if(IsInf() || b.IsInf())
            {
                if(IsInf() && b.IsInf()) return BigNum(State::Nan); // inf / inf
                const bool aNeg = (state == State::NegInf) || (state == State::Normal && isneg);
                const bool bNeg = (b.state == State::NegInf) || (b.state == State::Normal && b.isneg);
                if(IsInf()) return BigNum((aNeg ^ bNeg) ? State::NegInf : State::Inf);
                return BigNum("0"); // 有限 / 无穷 = 0（忽略符号，避免 -0）
            }
            // 以下均为 Normal
            const bool bZero = b.limbs.size() == 1 && b.limbs[0] == 0;
            if(bZero) // 除以 0：先报错，再返回 ±inf（0/0 返回 nan）
            {
                KLOG_ERROR(KBIGNUM_DIVBYZERO, "divide by zero");
                const bool aZero = limbs.size() == 1 && limbs[0] == 0;
                if(aZero) return BigNum(State::Nan);
                return BigNum(isneg ^ b.isneg ? State::NegInf : State::Inf);
            }
            if(limbs.size() == 1 && limbs[0] == 0) return BigNum("0"); // 0 / x = 0
            BigNum res = AbsDiv(*this, b); // 自动检测：整除则整数，否则保留 9 位小数
            res.isneg = isneg ^ b.isneg;
            return res;
        }
        BigNum BigNum::operator%(const BigNum& b) const
        {
            // 特殊状态：NaN / ±inf 参与取模均无意义 → NaN
            if(state == State::Nan || b.state == State::Nan) return BigNum(State::Nan);
            if(IsInf() || b.IsInf()) return BigNum(State::Nan);
            const bool bZero = b.limbs.size() == 1 && b.limbs[0] == 0;
            if(bZero) { KLOG_ERROR(KBIGNUM_DIVBYZERO, "modulo by zero"); return BigNum(State::Nan); }
            if(limbs.size() == 1 && limbs[0] == 0) return BigNum("0"); // 0 % x = 0
            BigNum r = AbsMod(*this, b); // abs(a) mod abs(b)（对齐小数位后做整数取余）
            r.isneg = isneg; // 余数符号跟随被除数（C++ 惯例）
            return r;
        }
        /**
         * @brief 幂运算：a^b（支持负指数、整数快速幂、分数指数开方、inf/nan 规则）
         * @note  整数指数：二进制快速幂（精确，全程 BigNum 运算）
         * @note  负指数：a^(-n) = 1 / a^n（保留 9 位小数）
         * @note  分数指数：用 double 近似计算并四舍五入到 9 位小数
         * @note  inf/nan：按 IEEE-754 惯例
         */
        BigNum Pow(const BigNum& a, const BigNum& b)
        {
            // 底数或指数为 NaN → NaN（|底数|==1 时恒为 1，IEEE-754 惯例）
            if (b.IsNan())
            {
                BigNum absA = a; absA.isneg = false;
                return (absA == BigNum("1")) ? BigNum("1") : BigNum(BigNum::State::Nan);
            }
            if (a.IsNan()) return BigNum(BigNum::State::Nan);

            // 底数为 ±inf
            if (a.IsInf())
            {
                if (b == BigNum("0")) return BigNum("1");               // inf^0 = 1
                if (b.IsInf()) return b.state == BigNum::State::Inf ? BigNum(BigNum::State::Inf) : BigNum("0"); // inf^±inf
                if (b.isneg) return BigNum("0");                        // inf^负 = 0
                const bool intExp = b.IsNormal() && b.scale == 0;
                if (a.state == BigNum::State::Inf) return BigNum(BigNum::State::Inf);   // +inf^正 = +inf
                // -inf^正：整数奇数次 → -inf，整数偶数次 → +inf，非整数 → NaN
                if (intExp)
                {
                    const bool odd = b.limbs.size() == 1 && b.limbs[0] % 2 == 1;
                    return BigNum(odd ? BigNum::State::NegInf : BigNum::State::Inf);
                }
                return BigNum(BigNum::State::Nan);
            }

            // 指数为 ±inf（底数为有限普通数）：按 |底数| 与 1 比较
            if (b.IsInf())
            {
                BigNum absA = a; absA.isneg = false;
                const bool gt1 = (absA > BigNum("1"));
                const bool lt1 = (absA < BigNum("1"));
                if (!gt1 && !lt1) return BigNum("1");                   // |a| == 1 → 1
                if (b.state == BigNum::State::Inf) return gt1 ? BigNum(BigNum::State::Inf) : BigNum("0"); // a^+inf
                return gt1 ? BigNum("0") : BigNum(BigNum::State::Inf);          // a^-inf
            }

            // 底数为 0
            if (a.limbs.size() == 1 && a.limbs[0] == 0)
            {
                if (b == BigNum("0")) return BigNum("1");               // 0^0 = 1
                return b.isneg ? BigNum(BigNum::State::Inf) : BigNum("0");      // 0^负 = inf；0^正 = 0
            }

            // 负底数 + 非整数指数 → NaN（实数域无意义）
            const bool intExp = b.scale == 0;
            if (a.isneg && !intExp) return BigNum(BigNum::State::Nan);

            // 负指数：a^(-n) = 1 / a^n（保留 9 位小数）
            if (b.isneg)
            {
                BigNum pos(b); pos.isneg = false;
                return BigNum("1") / Pow(a, pos);
            }

            // ============ 整数指数：二进制快速幂（精确） ============
            if (intExp)
            {
                BigNum result("1");
                BigNum base = a;
                BigNum exp = b;
                while (exp != BigNum("0"))
                {
                    if (exp % BigNum("2") == BigNum("1")) // 指数最低位为 1
                        result = result * base;
                    base = base * base;                   // 底数平方
                    exp = AbsDivSchool(exp, BigNum("2"), 0); // 指数右移 1 位（整除）
                }
                return result;
            }

            // ============ 分数指数：double 近似（四舍五入到 9 位小数） ============
            const double baseD = std::stod(a.ToStr());
            const double expD  = std::stod(b.ToStr());
            const double d = std::pow(baseD, expD);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(PRECISION) << d;
            return BigNum(oss.str());
        }
    }
}