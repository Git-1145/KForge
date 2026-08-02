# KForge [base] API 参考手册

> 版本：v3.0  
> 最后更新：2026-07-30  
> 编译标准：C++17  
> 平台：Windows（主），Linux / macOS（ANSI 颜色兼容）

---

## 目录

- [头文件与编译](#头文件与编译)
- [命名规范速查](#命名规范速查)
- [FIO — 配置解析器](#fio--配置解析器)
  - [配置文件格式](#配置文件格式)
  - [ValueType](#valuetype)
  - [Node](#node)
  - [NodeView](#nodeview)
  - [Document](#document)
  - [辅助函数](#辅助函数)
  - [使用示例](#fio-使用示例)
- [CLI — 控制台交互](#cli--控制台交互)
  - [KIoOut / KIoIn](#kiout--kiin)
  - [Color](#color)
  - [Random](#random)
  - [控制函数](#cli-控制函数)
  - [使用示例](#cli-使用示例)
- [KTimer — 高精度计时器](#ktimer--高精度计时器)
  - [Timer](#timer)
  - [TimerManager](#timermanager)
  - [使用示例](#ktimer-使用示例)
- [Utility — 工具函数](#utility--工具函数)
- [简写别名](#简写别名)
- [完整示例](#完整示例)
- [变更日志](#变更日志)

---

## 头文件与编译

```cpp
#include "base/KF.hpp"
```

依赖的 Windows API（通过 `<windows.h>`）：
- `SetConsoleTitleW` — 设置控制台标题
- `Sleep` — 毫秒级休眠
- `system("cls")` — 清屏

编译命令：
```bash
g++ -std=c++17 -I. main.cpp base/FIO.cpp base/CLI.cpp base/KTIMER.cpp base/UTILITY.cpp -o main.exe
```

---

## 命名规范速查

| 类别 | 规范 | 示例 |
|------|------|------|
| 类 / 结构体 | PascalCase | `Document`, `NodeView`, `TimerManager` |
| 函数 / 方法 | camelCase | `isObject()`, `getTime()`, `nodeVecToStrVec()` |
| 成员变量 | camelCase + `_` 后缀 | `name_`, `accumulated_` |
| 全局常量 | `k` + PascalCase | `kFioObjectBeg`, `kCliSeperator` |

---

## FIO — 配置解析器

### 配置文件格式

FIO 使用自定义的类 JSON 语法：

```
[顶层键]={
    [子键]=([数组元素1], [数组元素2]),
    [子键2]={
        [嵌套键]="字符串值",
        [数值键]=114514,
        [浮点键]=3.14
    }
}
```

| 符号 | 含义 |
|------|------|
| `{` `}` | 对象（Object） |
| `(` `)` | 数组（Array） |
| `"` `"` | 字符串字面量 |
| `[` `]` | 键名包裹 |
| `=` | 键值分隔符 |
| `,` | 元素分隔符 |

支持多顶层键：
```
[firstRoot]={ ... }
[secondRoot]={ ... }
```

---

### ValueType

```cpp
enum class ValueType {
    Empty,  // 未初始化
    Str,    // 字符串
    I64,    // 64 位整数
    F64,    // 64 位浮点数
    Object, // 对象（显式标记，含空对象 {}）
    Array   // 数组（显式标记，含空数组 ()）
};
```

---

### Node

配置文件的内存表示节点。

```cpp
struct Node {
    ObjMap obj;                       // 对象子节点
    std::vector<Node> arr;            // 数组元素
    ValueType type = ValueType::Empty; // 节点类型
    std::variant<std::string, long long, double> val; // 值

    Node() = default;
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;

    // 类型判断
    bool isObject() const;  // type == Object || !obj.empty()
    bool isArray()  const;  // type == Array  || !arr.empty()
    bool isString() const;  // type == Str
    bool isI64()    const;  // type == I64
    bool isF64()    const;  // type == F64
};
```

**注意**：`Node` 不可复制（含 `unique_ptr`），只可移动。

---

### NodeView

`Node` 的轻量级只读/查询视图，持裸指针，禁止拥有所有权。

#### 构造

```cpp
explicit NodeView(Node* p);
```

#### 查询

```cpp
// 判断子键是否存在（不抛异常）
bool isExist(const std::string& key) const;

// 按键名获取子节点（键不存在时抛 std::runtime_error）
NodeView get(const std::string& key);

// 按索引获取数组元素（越界或不是数组时抛 std::runtime_error）
NodeView at(size_t idx);
NodeView operator[](size_t index);  // 同 at()

// 路径解析。path 格式如 "[a][b][0]"
// 逐层解析：数字走数组索引，非数字走对象键名
NodeView value(const std::string& path);
```

#### 类型代理

```cpp
bool isString() const;
bool isI64()    const;
bool isF64()    const;
bool isObject() const;
bool isArray()  const;
```

#### 取值

```cpp
// 返回数组引用，避免拷贝
std::vector<Node>& arr();

// 取值（类型不匹配时 std::get 抛 std::bad_variant_access）
std::string str()  const;   // 取 string
long long   i64()  const;   // 取 long long
double      f64()  const;   // 取 double
size_t      size() const;   // 数组大小
```

---

### Document

配置解析器的核心类，封装了一份配置文件的解析结果。支持多实例，互不干扰。

```cpp
class Document {
public:
    Document();
    Document(Document&&);
    Document& operator=(Document&&);
    Document(const Document&) = delete;            // 禁止复制
    Document& operator=(const Document&) = delete; // 禁止复制

    // 打开并解析配置文件
    // 解析失败时抛 std::runtime_error（含详细错误位置）
    void open(const std::string& path);

    // 读取指定路径的节点
    // topKey 格式："[顶层键][子键][索引]" 或 "顶层键"
    NodeView read(const std::string& topKey) const;

    // 获取根节点视图
    NodeView root() const;
};
```

#### open() 异常

| 场景 | 异常消息 |
|------|---------|
| 文件无法打开 | `"open file failed: <path>"` |
| 顶层键为空 | `"parse error: empty top-level key"` |
| 缺少 `]` | `"parse error: missing closing bracket ']' near pos:..."` |
| 缺少 `"` | `"parse error at pos:..., expected '\"'"` |
| 意外字符 | `"parse error at pos:... unexpected character 'x'"` |
| 空 token | `"parse error: empty token"` |

---

### 辅助函数

```cpp
// 将节点数组（每个节点 type 为 Str）转换为字符串数组
// 若节点不是字符串，std::get 抛 std::bad_variant_access
std::vector<std::string> nodeVecToStrVec(std::vector<Node>& vec);
```

---

### FIO 使用示例

```cpp
#include "base/KF.hpp"

int main() {
    // 加载第一份配置
    fio::Document docA;
    docA.open("cfgA.txt");

    // 同时加载第二份配置（互不覆盖）
    fio::Document docB;
    docB.open("cfgB.txt");

    // 深嵌套读取
    auto v = docA.read("[app][settings][database][host]");
    std::string host = v.str();

    // 数组遍历
    auto arr = docA.read("[app][menu][options]");
    for (size_t i = 0; i < arr.size(); ++i)
        std::cout << arr.at(i).str() << "\n";

    // 路径解析
    auto ver = docA.root().value("[app][info][version]");
    std::cout << ver.str();

    return 0;
}
```

---

## CLI — 控制台交互

### KIoOut / KIoIn

#### KIoOut（`kout`）

封装 `std::cout`，提供统一的输出格式：

| 输出位置 | 前缀 |
|---------|------|
| 每组第一行 | `- ` |
| `std::endl` 后的续行 | `  `（两个空格缩进） |
| `kend` 之后 | 结束当前组，下一行重新打印 `- ` |

```cpp
kout << "Hello" << kend;              // - Hello
kout << "Line 1" << std::endl;         //   Line 1
kout << "Line 2" << kend;              //   Line 2
kout << "New Group" << kend;           // - New Group
```

#### KIoIn（`kin`）

封装 `std::cin`，自动添加 `> ` 提示符：

```cpp
std::string name;
int age;
kin >> name >> age;
// > Alice
// > 20
```

---

### Color

ANSI 颜色控制结构体，通过 `kout` 直接输出使用。

```cpp
struct Color {
    static constexpr const char* black   = "\033[30m";
    static constexpr const char* red     = "\033[31m";
    static constexpr const char* green   = "\033[32m";
    static constexpr const char* yellow  = "\033[33m";
    static constexpr const char* blue    = "\033[34m";
    static constexpr const char* magenta = "\033[35m";
    static constexpr const char* cyan    = "\033[36m";
    static constexpr const char* white   = "\033[37m";
    static constexpr const char* reset   = "\033[0m";   // 重置颜色
    static constexpr const char* bold    = "\033[1m";   // 加粗
};
```

**使用方式：**

```cpp
kout << cli::Color::red << "Error" << cli::Color::reset << kend;
kout << cli::Color::bold << cli::Color::green << "OK" << cli::Color::reset << kend;
```

**注意**：Windows CMD 默认不解析 ANSI，需调用 `system("color")` 或改用 Windows Terminal。

---

### Random

轻量级随机数工具，基于 `std::mt19937`。

```cpp
struct Random {
    // 生成 [min, max] 范围内的均匀分布整数
    static int nextInt(int min, int max);

    // 生成 [min, max) 范围内的均匀分布浮点数
    static double nextDouble(double min, double max);

    // 50% 概率返回 true
    static bool nextBool();

    // Fisher-Yates 洗牌算法，原地打乱 vector
    template<typename T>
    static void shuffle(std::vector<T>& vec);
};
```

**使用方式：**

```cpp
int dice = cli::Random::nextInt(1, 6);
double prob = cli::Random::nextDouble(0.0, 1.0);
bool flag = cli::Random::nextBool();

std::vector<int> v = {1, 2, 3, 4, 5};
cli::Random::shuffle(v);
```

---

### CLI 控制函数

```cpp
// 显示选项菜单并获取用户选择
// doc: 已加载的配置文档
// path: 指向包含 [options] 数组和 [prompt] 字符串的节点路径
size_t option(const FIO::Document& doc, const std::string& path);

// 清屏（Windows system("cls")）
void clear();

// 暂停，等待用户按两次 ENTER
void pause();

// 程序结束模板：打印分隔线、"Program ended"、自动 pause()
void programEnd();

// 程序开始模板
// doc:        已加载的配置文档
// consoleTitle: 控制台窗口标题（Windows SetConsoleTitleW）
// path:       配置文件中 info 节点路径，如 "[program]"
void programBegin(const FIO::Document& doc, const std::wstring& consoleTitle,
                  const std::string& path);
```

---

### CLI 使用示例

```cpp
#include "base/KF.hpp"

int main() {
    fio::Document doc;
    doc.open("cfg.txt");

    cli::programBegin(doc, L"KForge Demo", "[program]");

    size_t choice = cli::option(doc, "[program][menu]");
    kout << "You selected: " << choice << kend;

    // 彩色输出结果
    kout << cli::Color::green << "Done!" << cli::Color::reset << kend;

    cli::programEnd();
    return 0;
}
```

---

## KTimer — 高精度计时器

### Timer

基于 `std::chrono::steady_clock` 的单个计时器。

```cpp
enum class TimeUnit { Us, Ms, S };

class Timer {
public:
    explicit Timer(std::string name = "Timer", TimeUnit unit = TimeUnit::Ms);

    void start();   // 开始/继续计时
    void pause();   // 暂停，累计已流逝时间
    void clear();   // 清零累计时间并停止
    void setUnit(TimeUnit u);      // 切换显示单位
    void setName(std::string n);   // 修改名称
    double getTime() const;        // 获取累计时间（按当前单位转换）
    void print() const;            // 输出: [ name ] Time: X unit
};
```

---

### TimerManager

管理多个命名计时器。

```cpp
class TimerManager {
public:
    // 创建并自动启动。同名会覆盖旧计时器。
    void create(const std::string& name, TimeUnit unit);

    // 获取计时器引用（不存在时抛 std::runtime_error）
    Timer& get(const std::string& name);

    bool exists(const std::string& name);
    void remove(const std::string& name);
};
```

---

### KTimer 使用示例

```cpp
#include "base/KF.hpp"

int main() {
    timer::TimerManager tm;

    tm.create("sort", timer::TimeUnit::Ms);
    Sleep(100);  // 模拟排序
    tm.get("sort").pause();
    tm.get("sort").print();  // [ sort ] Time: 100.xxx ms

    // 继续计时
    tm.get("sort").start();
    Sleep(50);
    tm.get("sort").print();  // 累计时间

    // 单位转换
    tm.get("sort").setUnit(timer::TimeUnit::S);
    tm.get("sort").print();  // [ sort ] Time: 0.15xxx s

    return 0;
}
```

---

## Utility — 工具函数

```cpp
namespace KF::Utility {
    // 检查字符串是否为正整数
    // 规则：非空、每个字符都是数字（0-9），不含符号/空格
    // 空字符串返回 false
    bool isPosInt(const std::string& str);
}
```

| 输入 | 输出 |
|------|------|
| `"42"` | `true` |
| `"0"` | `true` |
| `""` | `false` |
| `"-1"` | `false` |
| `"3.14"` | `false` |
| `" 42"` | `false` |
| `"12a34"` | `false` |

---

## 简写别名

`KF.hpp` 末尾定义的全局别名：

```cpp
namespace cli   = KF::CLI;
namespace fio   = KF::FIO;
namespace timer = KF::KTimer;
namespace uti   = KF::Utility;

inline auto& kout = KF::CLI::kOut;   // 全局 KIoOut 实例
inline auto& kin  = KF::CLI::kIn;    // 全局 KIoIn 实例
inline auto& kend = KF::CLI::kEnd;   // EndTag 实例
```

---

## 完整示例

**cfg.txt：**
```
[program]={
    [info]={
        [title]="KForge Demo",
        [description]="A simple demo program.",
        [version]="1.0.0",
        [create_time]="2026-07-30 16:00:00",
    },
    [menu]={
        [prompt]="Please select an option:",
        [options]=("Start", "Settings", "Exit"),
    }
}
```

**main.cpp：**
```cpp
#include "base/KF.hpp"

int main() {
    fio::Document doc;
    doc.open("cfg.txt");

    cli::programBegin(doc, L"KForge Demo", "[program]");

    size_t choice = cli::option(doc, "[program][menu]");
    kout << "You selected: " << choice << kend;

    timer::TimerManager tm;
    tm.create("task", timer::TimeUnit::Ms);
    Sleep(100);
    tm.get("task").pause();
    tm.get("task").print();

    cli::programEnd();
    return 0;
}
```

---

## 变更日志

### v3.0（2026-07-30）
- **CLI 新增**：`Color` ANSI 颜色、`Random` 随机数工具
- **FIO**：`Document` 类支持多实例；`ValueType` 显式标记 Object/Array
- **Bug 修复**：`isPosInt()` 空字符串、`option()` 溢出判断、`readBracket()` 静默吞错
- **命名**：统一 camelCase / PascalCase

### v2.0（2026-07-30）
- FIO 重构为 `Document` 类，消除全局单例
- 减小 Windows 依赖（后 v3.0 恢复）
