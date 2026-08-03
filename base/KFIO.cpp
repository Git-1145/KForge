#include "KF.hpp"
namespace KF
{
    namespace KFIO
    {
        std::string ReadFileRaw(std::string_view FilePath)
        {
            std::ifstream File(std::string(FilePath), std::ios::binary);
            if (!File)
                KLOG::Fatal(KFIO_FILE_OPEN_FAIL,std::string(FilePath));
            // 获取文件大小，预分配内存
            File.seekg(0, std::ios::end);
            std::size_t size = static_cast<std::size_t>(File.tellg());
            File.seekg(0, std::ios::beg);
            
            // 一次性读取整个文件
            std::string content;
            content.resize(size);
            File.read(content.data(), static_cast<std::streamsize>(size));
            
            if (!File) {
                KLOG::Fatal(KFIO_FILE_READ_FAIL,std::string(FilePath));
            }
            return content;
            ///@example std::string text = KFIO::ReadFile("readme.txt");
        }
    }
}