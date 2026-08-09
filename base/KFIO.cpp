#define _CRT_SECURE_NO_WARNINGS
#include "KF.hpp"
#include <cerrno>
#include <cstring>
namespace KF
{
    namespace KFIO
    {
        /// @brief 把相对/绝对路径解析为基于当前工作目录的绝对路径（用于失败诊断）
        /// @note  使用 Win32 GetFullPathNameA，基准为进程 CWD，正好暴露
        ///        "相对路径解析到了哪个目录" 这一排错关键信息
        static std::string ToAbsolute(std::string_view path)
        {
            char buf[MAX_PATH] = {0};
            DWORD len = GetFullPathNameA(std::string(path).c_str(), MAX_PATH, buf, nullptr);
            // 返回 0 或超出缓冲长度则回退用原始路径，保证不丢信息
            if (len == 0 || len >= MAX_PATH)
                return std::string(path);
            return std::string(buf);
        }

        /// @brief 读取文件（粗文本，不做任何处理）
        /// @param filepath 文件路径（相对路径以进程 CWD 为基准，跨目录运行易出错）
        /// @return 文件全部内容的 std::string
        /// @note  失败时会 Fatal 并附带绝对路径与 errno，便于定位
        ///        @example std::string text = KFIO::ReadFileRaw("readme.txt");
        std::string ReadFileRaw(std::string_view filepath)
        {
            std::ifstream File(std::string(filepath), std::ios::binary);
            if (!File)
            {
                // 立即取 errno，避免被后续调用覆盖
                int err = errno;
                std::string absPath = ToAbsolute(filepath);
                std::string extra = absPath + " | errno=" + std::to_string(err)
                                  + ": " + std::strerror(err);
                KLOG_FATAL(KFIO_FILE_OPEN_FAIL, extra);
            }

            // 获取文件大小，预分配内存
            File.seekg(0, std::ios::end);
            std::size_t size = static_cast<std::size_t>(File.tellg());
            File.seekg(0, std::ios::beg);

            // 一次性读取整个文件
            std::string content;
            content.resize(size);
            File.read(content.data(), static_cast<std::streamsize>(size));

            if (!File)
            {
                int err = errno;
                std::string extra = std::string(filepath) + " | errno=" + std::to_string(err)
                                  + ": " + std::strerror(err);
                KLOG_FATAL(KFIO_FILE_READ_FAIL, extra);
            }
            return content;
        }
    }
}
