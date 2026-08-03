#include "KF.hpp"
namespace KF
{
    namespace KLOGGER
    {
            /**
             * @brief 输出错误日志，根据错误码匹配说明文本
             * @param code 十六进制错误码
             * @param extra 附加自定义信息（可选）
             */
            void Log(Code code, const std::string extra,size_t type)
            {
                std::string desc = "未知错误码";
                auto iter = Table.find(code);
                if (iter != Table.end())
                    desc = iter->second;
                // 保存输出流标志，防止std::hex污染后续打印
                auto flag = std::cerr.flags();
                switch (type)
                {
                    case 1:
                        std::cerr << "\n[INFO] ";
                        break;
                    case 2:
                        std::cerr << "\n[WARNING] ";
                        break;
                    case 3:
                        std::cerr << "\n[ERROR] ";
                        break;
                    case 4:
                        std::cerr << "\n[FATAL] ";
                        break;
                    default:
                        std::cerr << "\n[UNKNOWN] ";
                }

                std::cerr << " Code: 0x"
                        << std::setw(8) << std::setfill('0') << std::hex << code
                        << " Msg: " << desc;
                if (!extra.empty())
                    std::cerr << " | " << extra;
                std::cerr << "\n";
                // 恢复原始输出格式
                std::cerr.flags(flag);
            }
            void Error(Code code, const std::string& extra)
            {
                Log(code, extra, 3);
            }
            void Warning(Code code, const std::string& extra)
            {
                Log(code, extra, 2);
            }
            void Info(Code code, const std::string& extra)
            {
                Log(code, extra, 1);
            }
            void Fatal(Code code, const std::string& extra)
            {
                Log(code, extra, 4);
                std::cerr << "\n程序将终止运行...\n";
                system("pause");
                exit(-1);
            }
            std::unordered_map<Code, std::string_view> Table = 
            {
                {0x01000001, "测试-信息"},
                {0x01000002, "测试-警告"},
                {0x01000003, "测试-错误"},
                {0x01000004, "测试-严重错误"},
                {0x02301001, "KSON AsSth 类型不匹配"},
                {0x02401001, "KFIO 文件打开失败"},
                {0x02401002, "KFIO 文件读取失败"},
            };
    };
}

/**
 * @brief 注意事项
{
    1. 错误码格式：0x[aa][b][cc][ddd]，其中：
        - [aa]：模块号（2位16进制）
        - [b]：等级（1位16进制）
        - [cc]: 类型（2位16进制）
        - [ddd]：具体错误码（3位16进制）
    2. 模块号
        - 01：通用模块 一般是测试用
        - 02：KSON
        - 03：KTIMER
        - 04：KFIO
    3. 等级
        - 1: 信息
        - 2: 警告
        - 3: 错误
        - 4: 严重错误
}
**/