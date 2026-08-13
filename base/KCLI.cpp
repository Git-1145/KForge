#include "base/KF.hpp"
using namespace KFIO;
using namespace KSON;
using namespace KLOG;

/**
 * @file KCLI.cpp
 * @brief KCLI 命令行交互模块实现
 * @version 1.0.0
 * @date 2026-08-13
 * @author Git-1145
 * KSON 配置格式（KBegin 入参）：
 *   "title": "应用标题",
 *   "description": "应用描述",
 *   "author": "作者",
 *   "cmdtitle": "命令行标题"
 *
 * KSON 配置格式（KOptions 入参，一个 kson 索引）：
 *   "name": "菜单标题",
 *   "options": ["选项一", "选项二", "选项三"]
 */

namespace KF
{
    namespace KCLI
    {
        /////////////////////////////////////////////////////////
        // 内部工具函数
        /////////////////////////////////////////////////////////

        /// @brief 计算 UTF-8 字符串的终端显示宽度（CJK=2, ASCII=1）
        /// @note  用于框线对齐：中文字符占 2 列，ASCII 占 1 列
        static size_t DisplayWidth(std::string_view s)
        {
            size_t width = 0;
            for (size_t i = 0; i < s.size(); )
            {
                unsigned char c = static_cast<unsigned char>(s[i]);
                if      (c < 0x80) { width += 1; i += 1; }  // ASCII
                else if (c < 0xC0) { i += 1; }               // continuation byte（跳过）
                else if (c < 0xE0) { width += 1; i += 2; }  // 2-byte（拉丁扩展，窄）
                else if (c < 0xF0) { width += 2; i += 3; }  // 3-byte（CJK，宽）
                else               { width += 2; i += 4; }  // 4-byte（emoji，宽）
            }
            return width;
        }

        /// @brief 启用 Windows 终端 VT100 转义序列处理（支持颜色输出）
        static void EnableVT100()
        {
            HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode;
            if (GetConsoleMode(h, &mode))
                SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }

        /// @brief 设置控制台窗口标题（UTF-8 转宽字符，支持中文）
        static void SetTitleUTF8(const std::string& title)
        {
            int wlen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
            if (wlen <= 0) return;
            std::wstring wtitle(wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, wtitle.data(), wlen);
            SetConsoleTitleW(wtitle.c_str());
        }

        /// @brief 从 kson 节点按键取字符串，不存在时返回空串
        static std::string GetStr(const kson& node, std::string_view key)
        {
            std::string val = node[std::string(key)].Auto();
            if (val == "null") return "";
            return val;
        }

        /////////////////////////////////////////////////////////
        // CLI 功能函数实现
        /////////////////////////////////////////////////////////
        /// @brief KBegin 内部实现
        /// @param args 参数数组，语义由个数决定：
        ///   2 参: [title, desc]            -> 窗口标题=title, 框=title/desc/空
        ///   3 参: [title, desc, author]    -> 窗口标题=title, 框=title/desc/author
        ///   4 参: [cmdtitle, title, desc, author] -> 窗口标题=cmdtitle, 框=title/desc/author
        void KBeginImpl(const std::vector<std::string>& args)
        {
            std::string cmdtitle, title, description, author;
            if (args.size() >= 4)
            {
                cmdtitle    = args[0];
                title       = args[1];
                description = args[2];
                author      = args[3];
            }
            else if (args.size() == 3)
            {
                cmdtitle = title = args[0];
                description = args[1];
                author      = args[2];
            }
            else // 2 参
            {
                cmdtitle = title = args[0];
                description = args[1];
            }

            // 启用 VT100 颜色 + UTF-8 输出
            EnableVT100();
            SetConsoleOutputCP(CP_UTF8);
            SetTitleUTF8(cmdtitle);
            size_t w = DisplayWidth(title);
            if (DisplayWidth(description) > w) w = DisplayWidth(description);
            if (DisplayWidth(author) > w) w = DisplayWidth(author);
            std::string bar(w + 4, '-');
            auto line = [w](const std::string& content) {
                size_t pad = w - DisplayWidth(content);
                std::cout << "|  " << content << std::string(pad, ' ') << "  |\n";
            };
            std::cout << Color::SkyBlue << "+" << bar << "+\n";
            line(title);
            line("");
            line(description);
            line(author);
            std::cout << "+" << bar << "+" << Color::Reset << "\n";
            std::cout << std::endl;
        }

        void KBegin(const KSON::kson file)
        {
            auto meta = file[FILE_META];
            /// @attention 缺键不报错，用 Unknown 补充
            auto get = [&meta](const char* key) {
                auto v = meta[key];
                if (!v.Exists()) return std::string(KBEGIN_UNKNOWN);
                return v.Str();
            };
            std::string cmdtitle    = get(FILE_CMDTITLE);
            std::string title       = get(FILE_TITLE);
            std::string description = get(FILE_DESC);
            std::string author      = get(FILE_AUTHOR);
            // 窗口标题与框标题一致（保持原有行为）
            std::vector<std::string> args{cmdtitle, title, description, author};
            KBeginImpl(args);
        }
        std::size_t KOptions(const kson& menu)
        {
            std::string title = GetStr(menu, FILE_OPTNAME);
            auto opts = menu[FILE_OPTION];
            std::size_t count = opts.size();

            if (count == 0)
            {
                std::cout << Color::Red << "错误：菜单没有可选项" << Color::Reset << "\n";
                return 0;
            }

            // 收集选项标签
            std::vector<std::string> labels;
            for (std::size_t i = 0; i < count; i++)
                labels.push_back(opts[i].Auto());

            // 计算内容最大显示宽度（标题 + 所有选项行）
            size_t maxW = DisplayWidth(title);
            for (std::size_t i = 0; i < count; i++)
            {
                std::string prefix = "  [" + std::to_string(i + 1) + "] ";
                size_t lineW = DisplayWidth(prefix) + DisplayWidth(labels[i]);
                if (lineW > maxW) maxW = lineW;
            }
            size_t innerW = maxW + 2; // 两侧各 1 空格 padding

            // 打印菜单框（ASCII: + - |）
            std::string bar(innerW, '-');
            std::cout << "\n" << Color::SkyBlue << "+" << bar << "+\n";

            // 标题行（居中，加粗）
            size_t titleW = DisplayWidth(title);
            size_t leftPad = (innerW - titleW) / 2;
            size_t rightPad = innerW - titleW - leftPad;
            std::cout << "|" << std::string(leftPad, ' ')
                      << Color::Bold << title << Color::Reset << Color::SkyBlue
                      << std::string(rightPad, ' ') << "|\n";

            // 分隔线
            std::cout << "+" << bar << "+\n";

            // 选项行
            for (std::size_t i = 0; i < count; i++)
            {
                std::string prefix = "  [" + std::to_string(i + 1) + "] ";
                std::string content = prefix + labels[i];
                size_t pad = innerW - DisplayWidth(content);
                std::cout << "|" << content << std::string(pad, ' ') << "|\n";
            }

            std::cout << "+" << bar << "+" << Color::Reset << "\n";

            // 输入循环：非法输入时提示并重试
            while (true)
            {
                std::cout << Color::SkyBlue << "请输入选择 [1-" << count << "]: " << Color::Reset;
                std::string line;
                std::getline(std::cin, line);
                try {
                    size_t choice = std::stoull(line);
                    if (choice >= 1 && choice <= count)
                        return choice; // 转为 0-based 索引
                } catch (...) {}
                // 输入不是数字或超出范围
                std::cout << Color::Red << "输入超出范围，请重新输入" << Color::Reset << "\n";
            }
        }

        void kpause()
        {
            std::cout << "\n" << Color::SkyBlue << "按任意键继续..." << Color::Reset << std::flush;
            system("pause >nul");
            std::cout << "\n";
        }

        void KEnd()
        {
            kpause();
            exit(0);
        }
    }
}
