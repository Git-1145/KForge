#include "base/KF.hpp"
#include <unordered_map>
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

        /// @brief 计算含 ANSI 转义序列的字符串的可见显示宽度
        /// @note  先剥离 \033[...m 颜色/样式码，再按 CJK=2/ASCII=1 计算，供框线对齐用
        static size_t DisplayWidthNoAnsi(std::string_view s)
        {
            // 去 ANSI 序列
            std::string clean;
            clean.reserve(s.size());
            for (size_t i = 0; i < s.size(); )
            {
                if (s[i] == '\033' && i + 1 < s.size() && s[i + 1] == '[')
                {
                    i += 2;
                    while (i < s.size() && !(s[i] >= '0' && s[i] <= '?')) ++i; // 跳过参数
                    if (i < s.size()) ++i; // 跳过结尾字母
                    continue;
                }
                clean += s[i];
                ++i;
            }
            return DisplayWidth(clean);
        }

        /// @brief 从 kson 节点按键取字符串，不存在时返回空串
        static std::string GetStr(const kson& node, std::string_view key)
        {
            std::string val = node[std::string(key)].Auto();
            if (val == "null") return "";
            return val;
        }

        /////////////////////////////////////////////////////////
        // Kout 核心 tag 解析：{tag} → ANSI 颜色码
        /////////////////////////////////////////////////////////
        static const std::unordered_map<std::string_view, const char*>& TagMap()
        {
            static const std::unordered_map<std::string_view, const char*> m = {
                // 前景色
                {"red",          KLOGGER::Color::Red},
                {"green",        KLOGGER::Color::Green},
                {"blue",         KLOGGER::Color::Blue},
                {"yellow",       KLOGGER::Color::Yellow},
                {"skyblue",      KLOGGER::Color::SkyBlue},
                {"orange",       KLOGGER::Color::Orange},
                {"magenta",      KLOGGER::Color::Magenta},
                {"cyan",         KLOGGER::Color::Cyan},
                {"lightyellow",  KLOGGER::Color::LightYellow},
                // 样式
                {"bold",         "\033[1m"},
                {"dim",          "\033[2m"},
                {"underline",    "\033[4m"},
                {"blink",        "\033[5m"},
                // 关闭
                {"/",            KLOGGER::Color::Reset},
                {"/bold",        "\033[22m"},
                {"/dim",         "\033[22m"},
                {"/underline",   "\033[24m"},
                {"/blink",       "\033[25m"},
            };
            return m;
        }

        std::string Kout::ParseTags(std::string_view str)
        {
            std::string out;
            out.reserve(str.size());
            auto& map = TagMap();
            for (std::size_t i = 0; i < str.size(); ++i)
            {
                // 过滤真实制表符（来自 KSON ParseStr 或直接输入）
                if (str[i] == '\t') continue;

                // ---- 反斜杠转义：\n \r \b \t \" \\ ----
                if (str[i] == '\\' && i + 1 < str.size())
                {
                    char esc = str[i + 1];
                    switch (esc)
                    {
                        case 'n':  out += '\n'; ++i; continue;
                        case 'r':  out += '\r'; ++i; continue;
                        case 'b':  out += '\b'; ++i; continue;
                        case 't':               ++i; continue; //制表符过滤
                        case '"':  out += '"';  ++i; continue;
                        case '\\': out += '\\'; ++i; continue;
                        default: break; // 未知转义，走下面的原样保留
                    }
                }
                // ---- {tag} 颜色/样式标签 ----
                else if (str[i] == '{')
                {
                    std::size_t close = str.find('}', i);
                    if (close != std::string::npos)
                    {
                        std::string_view tag(str.data() + i + 1, close - i - 1);
                        if (!tag.empty() && tag.find(' ') == std::string::npos)
                        {
                            auto it = map.find(tag);
                            if (it != map.end())
                            {
                                out += it->second;
                                i = close;
                                continue;
                            }
                        }
                        out += '{';
                    }
                    else
                    {
                        out += str[i];
                    }
                    continue;
                }
                out += str[i];
            }
            return out;
        }

        /////////////////////////////////////////////////////////
        // CLI 功能函数实现
        /////////////////////////////////////////////////////////
        /// @brief KBegin 内部实现
        /// @param cmdtitle 窗口标题
        /// @param title 标题
        /// @param description 描述
        /// @param author 作者（亮黄显示，带 "author: " 前缀）
        /// @param date 日期（亮黄显示，带 "date: " 前缀）
        void KBeginImpl(const std::string& cmdtitle, const std::string& title,
                        const std::string& description, const std::string& author,
                        const std::string& date)
        {
            // 启用 VT100 颜色 + UTF-8 输出
            EnableVT100();
            SetConsoleOutputCP(CP_UTF8);
            SetTitleUTF8(cmdtitle);

            // 将含 \n 的字段拆成多个物理行
            auto splitLines = [](const std::string& s) {
                std::vector<std::string> lines;
                std::string cur;
                for (char c : s)
                {
                    if (c == '\n') { lines.push_back(cur); cur.clear(); }
                    else cur.push_back(c);
                }
                lines.push_back(cur);
                return lines;
            };

            // 构建显示内容行（先解析 {tag} 颜色标签，content 已含 ANSI 码）
            struct LineInfo { std::string content; bool highlight; };
            std::vector<LineInfo> lines;

            // 标题
            for (auto& l : splitLines(title))       lines.push_back({Kout::ParseTags(l), false});
            // 空行
            lines.push_back({"", false});
            // 描述
            for (auto& l : splitLines(description)) lines.push_back({Kout::ParseTags(l), false});
            // 空行
            lines.push_back({"", false});
            // 作者
            {
                std::string authorLabel = "author: ";
                std::string authorLine = authorLabel + (author.empty() ? KBEGIN_UNKNOWN : author);
                lines.push_back({Kout::ParseTags(authorLine), true});
            }
            // 日期
            {
                std::string dateLabel = "date: ";
                std::string dateLine = dateLabel + (date.empty() ? KBEGIN_UNKNOWN : date);
                lines.push_back({Kout::ParseTags(dateLine), true});
            }

            // 取所有行中最大的可见宽度（剥离 ANSI 后计算，颜色码不计入对齐）
            size_t w = 0;
            for (auto& li : lines) { size_t dw = DisplayWidthNoAnsi(li.content); if (dw > w) w = dw; }

            // 打印标题框（ASCII: + - |），每行按 w 补齐，右边界对齐
            std::string bar(w + 4, '-');
            std::cout << Color::SkyBlue << "+" << bar << "+\n";
            for (auto& li : lines)
            {
                size_t pad = w - DisplayWidthNoAnsi(li.content);
                std::cout << Color::SkyBlue << "|  ";
                if (li.highlight)
                    std::cout << Color::LightYellow << li.content << std::string(pad, ' ');
                else
                    std::cout << li.content << std::string(pad, ' ');
                std::cout << Color::SkyBlue << "  |\n";
            }
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
            std::string date        = get(FILE_DATE);
            KBeginImpl(cmdtitle, title, description, author, date);
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
