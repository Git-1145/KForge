#include "KF.hpp"
namespace KF
{
    namespace KLOGGER
    {
        using ErrCode=uint32_t;
        class Logger
        {
        public:
            static Logger& instance()
            {
                static Logger ins;
                return ins;
            }

            /**
             * @brief 输出错误日志，根据错误码匹配说明文本
             * @param code 十六进制错误码
             * @param extra 附加自定义信息（可选）
             */
            void error(ErrCode code, const std::string& extra = "")
            {
                std::string desc = "未知错误码";
                auto iter = ErrTable.find(code);
                if (iter != ErrTable.end())
                {
                    desc = iter->second;
                }

                // 保存输出流标志，防止std::hex污染后续打印
                auto flag = std::cerr.flags();

                std::cerr << "[ERROR] Code:0x"
                        << std::setw(8) << std::setfill('0') << std::hex << code
                        << " Msg:" << desc;

                if (!extra.empty())
                {
                    std::cerr << " | " << extra;
                }
                std::cerr << "\n";

                // 恢复原始输出格式
                std::cerr.flags(flag);
            }

            // 外部动态注册错误码（可选，方便模块扩展）
            void register_err(ErrCode code, std::string msg)
            {
                ErrTable[code] = std::move(msg);
            }

        private:
            Logger()
            {
                // 在这里集中硬编码所有错误码
                ErrTable = {
                    {0x01040001,"你好世界"}
                };
            }

            std::unordered_map<ErrCode, std::string> ErrTable;
        };
        inline auto& log()
        {
            return KF::KLOGGER::Logger::instance();
        }
    }
}
/**
 * @brief 注意事项
{
    1. 错误码格式：0x[aa][b][ccccc]，其中：
        - [aa]：模块号（2位16进制）
        - [b]：错误类型（1位16进制）
        - [ccccc]：具体错误码（5位16进制）
    2. 模块号
        - 01：通用模块 一般是测试用
        - 02：KSON
        - 03：KTIMER
        - 04：KFIO
}
**/