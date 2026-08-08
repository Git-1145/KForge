# KForge API Reference

> **最后更新**: 2026-08-08
> **标准**: C++17
> **编译器**: MSVC 19.44 (x64)
> **头文件**: `base/KF.hpp`（唯一入口，`#include` 即可使用全部功能）

---

## 目录

- [命名空间结构](#命名空间结构)
- [KLOGGER 日志模块](#klogger-日志模块)
- [KSON 解析模块](#kson-解析模块)
- [KFIO 文件读写模块](#kfio-文件读写模块)
- [KCLI 命令行交互模块](#kcli-命令行交互模块)
- [KTIMER 计时器模块](#ktimer-计时器模块)
- [KBIGNUM 大数运算模块](#kbignum-大数运算模块)
- [KSON 数据格式](#kson-数据格式)
- [错误码速查表](#错误码速查表)
- [快速上手](#快速上手)

---

## 命名空间结构

```
KF
├── KLOGGER      日志、错误码、颜色常量
├── KSON         数据解析、节点树、路径访问
├── KFIO         文件读取
├── KTIMER       计时器管理
├── KBIGNUM      大数运算
└── KCLI         命令行 UI、链式 I/O
```

| 别名 | 实际命名空间 | 说明 |
|------|-------------|------|
| `KLOG` | `KF::KLOGGER` | 日志模块简写 |
| `KSON` | `KF::KSON` | 解析模块简写 |
| `KFIO` | `KF::KFIO` | 文件模块简写 |
| `KTIMER` | `KF::KTIMER` | 计时模块简写 |
| `KBIGNUM` | `KF::KBIGNUM` | 大数模块简写 |
| `KCLI` | `KF::KCLI` | CLI 模块简写 |

头文件底部通过 `using namespace KF::KLOGGER;` 将错误码、`Color`、`LogLevel`、`Module`、`MakeCode` 引入全局作用域，调用处无需前缀。

---

## KLOGGER 日志模块

**源文件**: `KLOGGER.cpp` | **命名空间**: `KF::KLOGGER`

### Color 颜色常量

VT100 转义序列，`KBegin()` 和 `Log()` 各自自动启用对应句柄的 VT100 处理。

| 常量 | 转义码 | 用途 |
|------|--------|------|
| `Color::Reset` | `\033[0m` | 重置所有样式 |
| `Color::Red` | `\033[31m` | Fatal 级别 / 错误提示 |
| `Color::Green` | `\033[32m` | Info 级别 |
| `Color::Yellow` | `\033[33m` | 标准黄色 |
| `Color::Blue` | `\033[34m` | 蓝色 |
| `Color::Magenta` | `\033[35m` | 紫红 |
| `Color::Cyan` | `\033[36m` | 标准青色 |
| `Color::LightYellow` | `\033[93m` | Warning 级别 / koutW |
| `Color::Orange` | `\033[38;5;208m` | Error 级别 / koutE |
| `Color::SkyBlue` | `\033[38;5;75m` | kout 默认色 / 框线 |
| `Color::Bold` | `\033[1m` | 加粗（可与其他色叠加） |

### LogLevel 枚举

```cpp
enum class LogLevel : uint32_t {
    Info    = 1,  // 信息（绿色前缀）
    Warning = 2,  // 警告（淡黄色前缀）
    Error   = 3,  // 错误（橙色前缀，程序继续）
    Fatal   = 4,  // 严重错误（红色加粗前缀，终止程序）
};
```

### Module 模块号

| 常量 | 值 | 说明 |
|------|----|------|
| `Module::Unknown` | `0x00` | 未知模块 |
| `Module::Common` | `0x01` | 通用/测试 |
| `Module::KFIO` | `0x02` | 文件读写 |
| `Module::KSON` | `0x03` | 数据解析 |
| `Module::KTIMER` | `0x04` | 计时 |
| `Module::KCLI` | `0x05` | 命令行交互 |
| `Module::KBIGNUM` | `0x02` | 大数运算 |

### MakeCode 错误码组装

```cpp
constexpr Code MakeCode(uint32_t module, LogLevel level, uint32_t type, uint32_t id) noexcept;
```

错误码格式：`0x[aa][b][cc][ddd]`（8 位十六进制）

| 段 | 位 | 含义 |
|----|-----|------|
| `[aa]` | bit24-31 | 模块号 |
| `[b]` | bit20-23 | 日志等级 |
| `[cc]` | bit12-19 | 错误类型 |
| `[ddd]` | bit0-11 | 序号 |

> **约定**: 错误码 `extern` 声明放在 `KF.hpp`，定义放在 `KLOGGER.cpp`（通过 `MakeCode` 组装），改码时只需重编译 `KLOGGER.cpp`。

### 日志函数

| 函数 | 级别 | 行为 |
|------|------|------|
| `Info(code, extra, file, line, func)` | Info | 输出日志，程序继续 |
| `Warning(code, extra, file, line, func)` | Warning | 输出日志，程序继续 |
| `Error(code, extra, file, line, func)` | Error | 输出日志，程序继续 |
| `Fatal(code, extra, file, line, func)` | Fatal | 输出日志后 `system("pause")` + `exit(EXIT_FAILURE)` |

> 以上函数为内部接口，**用户应通过宏调用**，不要直接调用。

### KLOG_* 宏

自动捕获 `__FILE__` / `__LINE__` / `__FUNCTION__`：

```cpp
KLOG_INFO(code, extra)      // → Info 级别
KLOG_WARNING(code, extra)   // → Warning 级别
KLOG_ERROR(code, extra)     // → Error 级别
KLOG_FATAL(code, extra)     // → Fatal 级别（终止程序）
```

| 参数 | 类型 | 说明 |
|------|------|------|
| `code` | `Code` | 错误码常量 |
| `extra` | `const std::string&` | 附加信息（可空字符串） |

**输出格式**（输出到 `stderr`）:

```
[ERROR]  Code: 0x03301001 Msg: KSON Parse string, expecting a quote | extra | at KSON.cpp:153 (ParseStr)
```

### Log 内部函数

```cpp
void Log(Code code, const std::string& extra, LogLevel level,
         const char* file, int line, const char* func);
```

- 查 `Table` 码表获取错误描述
- 按等级输出彩色前缀
- 保存/恢复 `cerr` 的 `flags` 和 `fill`，防止 `hex`/`setfill` 污染
- 首次调用时自动启用 `stderr` 的 VT100 处理

### Table 码表

```cpp
extern std::unordered_map<Code, std::string_view> Table;
```

定义在 `KLOGGER.cpp`，键为错误码常量，值为说明文本。新增错误码时必须同步在码表中添加条目。

---

## KSON 解析模块

**源文件**: `KSON.cpp` | **命名空间**: `KF::KSON`

### NodeType 枚举

```cpp
enum class NodeType {
    kInt,   // 整数 (long long)
    kDec,   // 浮点数 (double)
    kStr,   // 字符串 (std::string)
    kBool,  // 布尔值 (bool)
    kArr,   // 数组 (vector<Node>)
    kObj,   // 对象 (vector<pair<string,Node>>)
    kNull,  // 空值 (monostate)
};
```

### Node 类

KSON 数据树的底层节点，使用 `std::variant` 存储。

#### 构造函数

| 构造 | 类型 |
|------|------|
| `Node()` | kNull |
| `Node(bool)` | kBool |
| `Node(long long)` | kInt |
| `Node(double)` | kDec |
| `Node(std::string)` | kStr |
| `Node(std::vector<Node>)` | kArr |
| `Node(std::vector<pair<string,Node>>)` | kObj |

#### 类型判断

| 方法 | 返回 true 当 |
|------|-------------|
| `IsNull()` | kNull |
| `IsBool()` | kBool |
| `IsInt()` | kInt |
| `IsDec()` | kDec |
| `IsNumber()` | kInt 或 kDec |
| `IsString()` | kStr |
| `IsArray()` | kArr |
| `IsObject()` | kObj |

#### 取值

| 方法 | 返回类型 | 类型不匹配时 |
|------|----------|-------------|
| `AsBool()` | `bool` | KLOG_ERROR(KSON_TYPE_MISMATCH) |
| `AsInt()` | `long long` | KLOG_ERROR(KSON_TYPE_MISMATCH) |
| `AsDec()` | `double` | int 自动转 double；其他类型报错 |
| `AsStr()` | `string_view` | KLOG_ERROR(KSON_TYPE_MISMATCH) |
| `AsArr()` | `const vector<Node>&` | KLOG_ERROR(KSON_TYPE_MISMATCH) |
| `AsObj()` | `const vector<pair<string,Node>>&` | KLOG_ERROR(KSON_TYPE_MISMATCH) |

#### 大小与查找

| 方法 | 说明 |
|------|------|
| `size()` | 数组/对象的元素个数，标量返回 0 |
| `find(key)` | 对象按键查找，返回 `const Node*`，未找到返回 `nullptr` |
| `at(index)` | 数组按下标查找，越界返回 `nullptr` |

> **注意**: `find` 区分大小写。

### PathSeg 结构体

路径片段，记录一个访问步骤。

| 成员 | 类型 | 说明 |
|------|------|------|
| `key` | `std::string` | 键名（对象访问时有效） |
| `index` | `std::size_t` | 下标（数组访问时有效） |

构造：`PathSeg("key")` 或 `PathSeg(3)`

### NodePtr 类（别名 `kson`）

KSON 树的智能指针，持有根节点和访问路径，支持延迟解析（链式 `[]` 访问）。

```cpp
using kson = NodePtr;
```

#### 构造

| 构造 | 说明 |
|------|------|
| `NodePtr()` | 空指针 |
| `NodePtr(shared_ptr<Node> root)` | 指向根节点 |
| `NodePtr(root, path)` | 指向根节点 + 预设路径 |

#### 路径访问（链式，不立即解析）

| 操作 | 说明 |
|------|------|
| `operator[](key)` | 对象键访问，返回新的 NodePtr |
| `operator[](index)` | 数组下标访问，返回新的 NodePtr |
| `operator[]("key")` | `const char*` 重载 |

```cpp
kson doc = read(Preprocess(KFIO::ReadFileRaw("cfg.txt")));
auto val = doc["test"]["main"]["Array_Test"]["One Dim Array"][0];
```

#### 解析与取值

| 方法 | 返回类型 | 路径不存在时 |
|------|----------|-------------|
| `TryResolve()` | `const Node*` | 返回 `nullptr` |
| `Resolve()` | `const Node*` | KLOG_FATAL(UNKNOWN) |
| `Str()` | `string` | Fatal |
| `Int()` | `long long` | Fatal |
| `Dec()` | `double` | Fatal（int 自动转 double） |
| `Bool()` | `bool` | Fatal |
| `Size()` | `size_t` | Fatal |
| `Exists()` | `bool` | 返回 `false` |
| `Auto()` | `string` | 返回 `"null"` |

#### Auto() 类型自动转换

根据节点类型自动返回可打印字符串：

| 类型 | 输出 |
|------|------|
| kNull | `null` |
| kBool | `true` / `false` |
| kInt | 整数字符串 |
| kDec | 浮点字符串（去尾零，保留 15 位精度） |
| kStr | 字符串原文 |
| kArr | `[e1, e2, ...]` |
| kObj | `{"key": val, ...}` |

#### 静态工厂

| 方法 | 说明 |
|------|------|
| `Parse(text)` | 从字符串解析 |
| `ParseFile(filepath)` | 从文件读取并解析 |

### Parser 类（内部）

递归下降解析器，不对外暴露。

| 方法 | 说明 |
|------|------|
| `ParseStr()` | 解析字符串字面量，处理转义 |
| `ParseNum()` | 解析数字（整数/小数），含精度检查 |
| `ParseVal()` | 根据首字符分派到对应解析器 |
| `ParseArr()` | 解析数组 `[...]`，支持尾随逗号 |
| `ParseObj()` | 解析对象 `{...}`，支持尾随逗号、重复键后覆盖前 |
| `ParseImplicitObj()` | 解析隐式顶层对象（无外层 `{}`） |
| `PeekIsImplicitObj()` | 窥探是否为隐式对象 |

### 自由函数

| 函数 | 说明 |
|------|------|
| `Preprocess(raw)` | 预处理：去注释(`#`)、去空白、保留字符串内容 |
| `read(processed)` | 解析预处理后的文本，返回 `kson` |
| `ReadKsonFile(filename)` | 一站式：读文件 → 预处理 → 解析 |

```cpp
// 推荐用法
kson doc = read(Preprocess(KFIO::ReadFileRaw("cfg.txt")));
// 或一站式
kson doc = KSON::ReadKsonFile("cfg.txt");
```

---

## KFIO 文件读写模块

**源文件**: `KFIO.cpp` | **命名空间**: `KF::KFIO`

### ReadFileRaw

```cpp
std::string ReadFileRaw(std::string_view filepath);
```

读取文件全部内容（二进制模式，不做任何处理）。

| 参数 | 说明 |
|------|------|
| `filepath` | 文件路径（相对路径以进程 CWD 为基准） |

| 失败情况 | 错误码 | 级别 |
|----------|--------|------|
| 文件打开失败 | `KFIO_FILE_OPEN_FAIL` | Fatal（附带绝对路径 + errno） |
| 文件读取失败 | `KFIO_FILE_READ_FAIL` | Fatal（附带 errno） |

> 内部使用 `ToAbsolute()` 将相对路径解析为绝对路径，在错误信息中显示，便于排查 "找不到文件" 问题。

---

## KCLI 命令行交互模块

**源文件**: `KCLI.cpp` | **命名空间**: `KF::KCLI`

> `Color` 定义在 `KF::KLOGGER::Color`，KCLI 中通过命名空间别名引入：`namespace Color = KF::KLOGGER::Color;`

### Kout 类（链式输出）

```cpp
class Kout {
    explicit constexpr Kout(const char* color);
    template<typename T> Kout& operator<<(const T& val);
    Kout& operator<<(std::ostream& (*manip)(std::ostream&));
};
```

- 每次 `<<` 自动套用构造时的默认颜色
- 遇到 `std::endl` / `std::flush` 等 manipulator 时先输出 `Color::Reset` 再输出 manipulator
- 如需临时换色：`koutE << Color::Red << "严重" << std::endl;`

### Kin 类（链式输入）

```cpp
class Kin {
    template<typename T> Kin& operator>>(T& val);
};
```

- 每个 `>>` 读取一行（`getline`），按目标变量类型自动转换
- 转换失败时置 0 并发出 `KLOG_WARNING(KCLI_INPUT_INVALID, line)`

| 目标类型 | 转换方式 |
|----------|----------|
| `std::string` | 原样赋值 |
| `bool` | `"1"`/`"true"`/`"yes"` → true，其余 false |
| 整数类型 | `std::stoll` + `static_cast` |
| 浮点类型 | `std::stod` + `static_cast` |

### 全局对象

| 对象 | 默认颜色 | 用途 |
|------|----------|------|
| `kout` | SkyBlue（天蓝色） | 普通输出 |
| `koutW` | LightYellow（淡黄色） | 警告输出 |
| `koutE` | Orange（橙色） | 错误输出 |
| `koutF` | Red（红色） | 致命输出 |
| `kin` | — | 链式输入 |

```cpp
kout  << "普通信息" << 42 << std::endl;     // 天蓝色
koutW << "警告信息" << std::endl;             // 淡黄色
koutE << "错误信息" << std::endl;             // 橙色
koutF << "致命信息" << std::endl;             // 红色

int age; std::string name;
kin >> age >> name;                           // 每次读取一行
```

### CLI 功能函数

#### KBegin

```cpp
void KBegin(const KSON::kson& config);
```

初始化 CLI 环境：
- 启用 stdout/stderr 的 VT100 颜色处理
- 设置 UTF-8 输出编码
- 设置控制台窗口标题（支持中文）
- 打印标题框和描述

KSON 入参格式：

```
"title": "应用标题",
"description": "应用描述"
```

#### KOptions

```cpp
std::size_t KOptions(const KSON::kson& menu);
```

显示选项菜单，循环等待合法输入。

| 返回值 | 说明 |
|--------|------|
| `size_t` | 选中项索引（0-based） |

KSON 入参格式：

```
"title": "菜单标题",
"options": ["选项一", "选项二", "选项三"]
```

- 输入非法时红色提示并重新输入
- 菜单框使用 ASCII 字符 (`+ - |`) 绘制

#### kpause

```cpp
void kpause();
```

显示 "按任意键继续..." 并等待按键。

#### KEnd

```cpp
void KEnd();
```

调用 `kpause()` 后 `exit(0)` 退出程序。

### 内部工具函数

| 函数 | 说明 |
|------|------|
| `DisplayWidth(s)` | 计算 UTF-8 字符串终端显示宽度（CJK=2, ASCII=1） |
| `EnableVT100()` | 启用 stdout 的 VT100 处理 |
| `SetTitleUTF8(title)` | UTF-8 转宽字符设置控制台标题 |
| `GetStr(node, key)` | 从 kson 节点取字符串，不存在返回空串 |

---

## KTIMER 计时器模块

**源文件**: `KTIMER.cpp` | **命名空间**: `KF::KTIMER`

> `Color` 定义在 `KF::KLOGGER::Color`，KTIMER 中通过命名空间别名引入：`namespace Color = KF::KLOGGER::Color;`

### TimeUnit 时间单位枚举

```cpp
enum class TimeUnit {
    ns,  // 纳秒
    us,  // 微秒
    ms,  // 毫秒
    s,   // 秒
};
```

### TimerState 计时器状态枚举

```cpp
enum class TimerState {
    Running,  // 运行中
    Paused,   // 已暂停
};
```

### 计时器功能函数

#### AddTimer

```cpp
bool AddTimer(const std::string& name, TimeUnit unit);
```

新建计时器（指定名字和单位），创建后立即开始计时。同名计时器已存在时覆盖并发出警告。

| 参数 | 类型 | 说明 |
|------|------|------|
| `name` | `const std::string&` | 计时器名称（唯一标识） |
| `unit` | `TimeUnit` | 时间单位（ns/us/ms/s） |

| 返回值 | 说明 |
|--------|------|
| `true` | 新建成功 |
| `false` | 同名已存在（已覆盖） |

#### PauseTimer

```cpp
bool PauseTimer(const std::string& name);
```

暂停计时器（累计已运行时间）。

| 返回值 | 说明 |
|--------|------|
| `true` | 暂停成功 |
| `false` | 不存在或未在运行 |

#### StartTimer

```cpp
bool StartTimer(const std::string& name);
```

恢复已暂停的计时器。

| 返回值 | 说明 |
|--------|------|
| `true` | 恢复成功 |
| `false` | 不存在或未暂停 |

#### DeleteTimer

```cpp
bool DeleteTimer(const std::string& name);
```

删除计时器。

| 返回值 | 说明 |
|--------|------|
| `true` | 删除成功 |
| `false` | 不存在 |

#### GetTimer

```cpp
double GetTimer(const std::string& name);
```

获取计时器当前累计时间（按计时器单位）。

| 返回值 | 说明 |
|--------|------|
| `>= 0` | 累计时间（double） |
| `-1.0` | 计时器不存在 |

#### PrintTimer

```cpp
void PrintTimer(const std::string& name);
```

打印单个计时器信息（格式化框）。输出示例：

```
+------------------------------------------+
|  Timer: render                           |
+------------------------------------------+
|  State   : Running                       |
|  Elapsed : 1234.56 ms                    |
+------------------------------------------+
```

#### PrintAllTimers

```cpp
void PrintAllTimers();
```

打印所有计时器信息（格式化表格，按名称排序）。输出示例：

```
+--------------------------------------------------+
|  KTIMER - 所有计时器 (2)                          |
+--------------------------------------------------+
|  Name          State       Elapsed              |
|  load          Paused      567.89 us            |
|  render        Running     1234.56 ms           |
+--------------------------------------------------+
```

### 使用示例

```cpp
#include "KF.hpp"
using namespace KTIMER;
using namespace KCLI;

int main() {
    KBegin(read(Preprocess(
        "\"title\": \"计时器示例\","
        "\"description\": \"KTIMER 模块演示\""
    )));

    // 新建计时器（创建即开始计时）
    AddTimer("render", TimeUnit::ms);
    AddTimer("load",   TimeUnit::us);

    // 模拟工作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 暂停
    PauseTimer("render");

    // 恢复
    StartTimer("render");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    PauseTimer("render");
    PauseTimer("load");

    // 打印
    PrintTimer("render");
    PrintAllTimers();

    // 获取时间值
    double elapsed = GetTimer("render");
    kout << "render 耗时: " << elapsed << " ms" << std::endl;

    // 清理
    DeleteTimer("render");
    DeleteTimer("load");

    KEnd();
}
```

---

## KBIGNUM 大数运算模块

**源文件**: `KBIGNUM.cpp` | **命名空间**: `KF::KBIGNUM`

### 存储模型

base = 10⁹，每个 `limb`（`uint32_t`）存储 9 位十进制数字。选择 10⁹ 而非 2³² 是为了简化 `ToStr` 的十进制输出。

| 成员 | 类型 | 说明 |
|------|------|------|
| `limbs` | `vector<limb>` | 低位在前，`limbs[0]` 是最低 9 位 |
| `isneg` | `bool` | 负数标志 |
| `scale` | `size_t` | 小数位数（小数点后数字个数） |

> `limbs` 默认初始化为 `{0}`，表示零值。

### 类型别名

```cpp
using limb  = uint32_t;   // 基础分块（9 位十进制）
using dlimb = uint64_t;   // 扩展分块（运算时防溢出）
```

### Normalize 字符串合法化

```cpp
std::string Normalize(const std::string& str);
```

将任意输入字符串规整为 `[+|-]digits[.digits]` 格式：

- 提取首个 `-.+0123456789` 到末尾数字之间的有效字符
- 去前导零（保留小数点前必要的 `0`）
- 去小数点后尾随零
- 多余小数点视为分隔符（取第二个点之前的部分）
- 多余正负号做异或（`-` 偶数个 → 正，奇数个 → 负）
- 全零或无有效数字返回 `"0"`

| 输入 | 输出 |
|------|------|
| `000012340.3221000000` | `+12340.3221` |
| `-.123.123.` | `-0.123123` |
| `-0` | `0` |
| `dw1-abca00432.432100` | `+100432.4321` |
| `+42` | `+42` |

### ToBig 字符串转大数

```cpp
static BigNum ToBig(const std::string& str);
```

输入为 Normalize 后的字符串。流程：

1. `str[0]` 取符号位
2. `find('.')` 定位小数点 → 计算 `scale`
3. 跳过符号位和小数点，提取纯数字串 `digits`
4. 从右往左**每九位截取一个 limb**（`substr` 批量截取，非逐字符拼接）
5. 去除高位多余的零块
6. 零值强制 `isneg = false`

```cpp
// 核心循环：九位九位截取
dlimb pos = digits.size();
while(pos > 0)
{
    dlimb start = (pos >= BASEEXP) ? (pos - BASEEXP) : 0;
    res.limbs.push_back(std::stoll(digits.substr(start, pos - start)));
    pos = start;
}
```

### ToStr 大数转字符串

```cpp
std::string ToStr() const;
```

1. 零值直接返回 `"0"`
2. 最高位 limb 不补零，其余 limb 左补零到 9 位
3. 根据 `scale` 插入小数点（`scale >= digits.size()` 时补前导 `0.00...`）
4. 负数加 `-` 前缀

### 构造函数

| 构造 | 说明 |
|------|------|
| `BigNum()` | 默认构造，零值 |
| `BigNum(const std::string& str)` | 字符串构造（`Normalize` → `ToBig`） |
| `BigNum(const dlimb& num)` | 数字构造（`to_string` → `ToBig`） |

### 运算接口（待实现）

| 分类 | 函数 |
|------|------|
| 内部绝对值运算 | `AbsAdd` `AbsSub` `AbsMul` `AbsDiv` `AbsMod` `AbsCmp` `AbsPow` |
| 用户运算符 | `operator+ - * / %` `Pow` |
| 比较运算符 | `operator== != < <= > >=` |
| 输出 | `friend operator<<`（调用 `ToStr`） |

### 使用示例

```cpp
#include "KF.hpp"
using namespace KBIGNUM;

BigNum a("123456789012345678901234567890");
BigNum b("-0.0001");
kout << a << "\n";    // 123456789012345678901234567890
kout << b << "\n";    // -0.0001
```

---

## KSON 数据格式

KSON 是 KForge 自定义的类 JSON 数据格式。

### 语法规则

| 特性 | 支持情况 | 示例 |
|------|----------|------|
| 键值对 | `"key": value` | |
| 字符串 | 双引号包裹 | `"hello"` |
| 整数 | `long long` | `42`, `-7` |
| 小数 | `double` | `3.14`, `.5`, `-1.` |
| 布尔 | `true` / `false` | |
| 空值 | `null` | |
| 数组 | `[1, 2, 3]` | |
| 对象 | `{"a": 1, "b": 2}` | |
| 注释 | `#` 行注释 | `# 这是注释` |
| 尾随逗号 | 数组和对象均支持 | `[1, 2,]` |
| 隐式顶层对象 | 无需外层 `{}` | `"a": 1, "b": 2` |
| 转义字符 | `\n \t \r \b \" \\` | `"line1\nline2"` |
| 重复键 | 后覆盖前 | `{"a":1,"a":2}` → a=2 |

### 精度限制

- 小数有效数字超过 17 位时触发 `KSON_PARSE_NUM_PRECISION` 错误
- 整数超出 `long long` 范围时触发 `KSON_PARSE_NUMOR` 错误

### 完整示例

```
# 应用配置
"app_name": "MyApp",
"version": 1.0,
"settings": {
    "debug": true,
    "max_connections": 100,
    "tags": ["prod", "v2",]
},
"empty_array": [],
"empty_obj": {}
```

---

## 错误码速查表

### 通用模块 (01)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `TEST_INFO` | `0x01101001` | Info | 测试-信息 |
| `TEST_WARN` | `0x01201002` | Warning | 测试-警告 |
| `TEST_ERROR` | `0x01301003` | Error | 测试-错误 |
| `TEST_FATAL` | `0x01401004` | Fatal | 测试-严重错误 |

### KFIO 模块 (02)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KFIO_FILE_OPEN_FAIL` | `0x02401001` | Fatal | 文件打开失败 |
| `KFIO_FILE_READ_FAIL` | `0x02401002` | Fatal | 文件读取失败 |

### KSON 模块 (03)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KSON_PARSE_STRE` | `0x03301001` | Error | 字符串不以双引号开头 |
| `KSON_PARSE_STR_NOEND` | `0x03301002` | Error | 字符串缺少闭合双引号 |
| `KSON_PARSE_MULPOINT` | `0x03201003` | Warning | 多个小数点 |
| `KSON_PARSE_NUM_UE` | `0x03201004` | Warning | 数字含非法字符 |
| `KSON_PARSE_NUMOR` | `0x03301005` | Error | 数字超出类型范围 |
| `KSON_PARSE_NUM_USTYPE` | `0x03301006` | Error | 不支持的数字类型 |
| `KSON_PARSE_ESCAPE_SPECIAL` | `0x03201007` | Warning | 无效的转义序列 |
| `KSON_PARSE_UNFINISHED_ESCAPE` | `0x0340100A` | Fatal | 转义序列不完整（越界） |
| `KSON_PARSE_NUM_PRECISION` | `0x0330100B` | Error | 小数精度超出 double 范围 |
| `KSON_PARSE_VAL_END` | `0x033010AA` | Error | 读到末尾仍缺值 |
| `KSON_PARSE_VAL_ERROR` | `0x033010AC` | Error | 值类型无法识别 |
| `KSON_PARSE_ARR_BEGIN` | `0x033010AE` | Error | 期望数组起始 `[` |
| `KSON_PARSE_ARRUE` | `0x033010B0` | Error | 数组中出现非预期字符 |
| `KSON_PARSE_OBJ_BEGIN` | `0x033010B2` | Error | 期望对象起始 `{` |
| `KSON_PARSE_OBJ_KEY_QUOTE` | `0x033010B4` | Error | 对象键未用双引号包裹 |
| `KSON_PARSE_OBJ_SEPERATOR` | `0x033010B6` | Error | 键后缺少冒号 `:` |
| `KSON_PARSE_OBJUE` | `0x033010B8` | Error | 对象中出现非预期字符 |
| `KSON_PARSE_TRAIL` | `0x03202011` | Warning | 结尾有多余字符 |
| `KSON_TYPE_MISMATCH` | `0x03401001` | Fatal | AsXxx 类型不匹配 |

### KCLI 模块 (05)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KCLI_INPUT_INVALID` | `0x05201001` | Warning | 输入解析失败 |

### KBIGNUM 模块 (02)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KBIGNUM_MULPOINT` | `0x02201002` | Warning | 多余的小数点 |
| `KBIGNUM_INVALIDCHAR` | `0x02201004` | Warning | 数字中含非法字符 |

### KTIMER 模块 (04)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KTIMER_NOT_FOUND` | `0x04201001` | Warning | 计时器不存在 |
| `KTIMER_ALREADY_EXISTS` | `0x04201002` | Warning | 计时器已存在（将被覆盖） |
| `KTIMER_STATE_ERROR` | `0x04201003` | Warning | 计时器状态不允许此操作 |

### 未知模块 (00)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `UNKNOWN` | `0x00400000` | Fatal | 未知错误码 |

---

## 快速上手

### 最小示例

```cpp
#include "KF.hpp"
using namespace KSON;
using namespace KCLI;

int main() {
    // 1. 初始化 CLI
    kson cfg = read(Preprocess(
        "\"title\": \"我的应用\","
        "\"description\": \"KForge 框架演示\""
    ));
    KBegin(cfg);

    // 2. 彩色输出
    kout  << "程序启动成功" << std::endl;
    koutW << "配置项缺失，使用默认值" << std::endl;

    // 3. 读取 KSON 配置文件
    kson doc = read(Preprocess(KFIO::ReadFileRaw("cfg.txt")));
    //Alter: kson doc = ReadKsonFile("cfg.txt");
    kout << "版本号: " << doc["version"].Auto() << std::endl;

    // 4. 数组遍历
    auto arr = doc["tags"];
    for (size_t i = 0; i < arr.size(); i++)
        kout << "  [" << i << "] " << arr[i].Auto() << std::endl;

    // 5. 菜单交互
    kson menu = read(Preprocess(
        "\"title\": \"请选择操作\","
        "\"options\": [\"开始\", \"设置\", \"退出\"]"
    ));
    size_t choice = KOptions(menu);
    kout << "你选择了: " << choice << std::endl;

    KEnd();  // 暂停后退出
}
```

### 编译方式

```bat
cl /EHsc /std:c++17 /utf-8 /I..\base ^
    ..\base\KSON.cpp ..\base\KLOGGER.cpp ..\base\KFIO.cpp ..\base\KCLI.cpp ..\base\KTIMER.cpp ..\base\KBIGNUM.cpp ^
    main.cpp /Fe:app.exe
```

---
### 测试文件一览

| 文件 | 测试模块 | 覆盖功能 |
|------|----------|----------|
| `dbgKSON.cpp` | KSON | 字符串/文件解析、类型判断、取值、路径访问、find/at、size、Auto、多维数组遍历、转义、注释/尾随逗号、重复键、空容器、Preprocess、错误条件 |
| `dbgKFIO.cpp` | KFIO | ReadFileRaw 读取/验证、与 KSON 集成、空文件、Fatal 测试（读取不存在文件） |
| `dbgKLOGGER.cpp` | KLOGGER | KLOG_INFO/WARNING/ERROR/FATAL 宏、各模块错误码、MakeCode 组装、Table 码表查询、LogLevel/Module 枚举、Color 常量展示 |
| `dbgKCLI.cpp` | KCLI | kout/koutW/koutE/koutF 链式输出、临时换色、Color 常量展示、kin 链式输入、KOptions 菜单、从文件读取配置、kpause/KEnd |
| `dbgKTIMER.cpp` | KTIMER | AddTimer 新建/重名覆盖、计时精度验证、PauseTimer/StartTimer 暂停恢复、错误处理（不存在/状态错误）、GetTimer、PrintTimer、PrintAllTimers、DeleteTimer、综合工作流、空表打印 |
| `dbgKBIGNUM.cpp` | KBIGNUM | Normalize 合法化、ToBig+ToStr 往返测试、limbs/scale/isneg 验证、大整数/小数/边界值/非法输入 |

### 测试配置文件

| 文件 | 用途 |
|------|------|
| `cfg.kson` | KSON 全功能测试数据（所有数据类型、嵌套结构、转义、注释、重复键） |

### Fatal 测试说明

`dbgKSON.cpp`、`dbgKFIO.cpp`、`dbgKLOGGER.cpp` 中的 Fatal 级别测试放在文件最后执行，触发后会终止程序。非 Fatal 测试在 Fatal 测试之前全部完成。

### 统一格式约定

所有测试脚本遵循统一的格式：
- 使用 `SECTION` 宏标记测试段落
- 使用 `CHECK` 宏输出 `[PASS]` / `[FAIL]` 结果
- 使用 `SHOW` 宏输出键值对信息
- 使用 `kout` / `koutW` / `koutE` / `koutF` 彩色输出
- 文件头部注释列出所有测试项

---

## 维护指南

### 新增错误码

1. 在 `KF.hpp` 对应模块下添加 `extern const Code XXX_YYY_ZZZ;`
2. 在 `KLOGGER.cpp` 中添加 `const Code XXX_YYY_ZZZ = MakeCode(Module::XXX, LogLevel::YYY, 0xZZ, 0xNNN);`
3. 在 `KLOGGER.cpp` 的 `Table` 中添加 `{XXX_YYY_ZZZ, "说明文本"}`
4. 更新本文档的错误码速查表

### 新增 KSON 解析错误处理

1. 添加错误码（同上）
2. 在 `KSON.cpp` 的 `Parser` 类对应方法中调用 `KLOG_ERROR/KLOG_WARNING/KLOG_FATAL`
3. 确保非预期字符时推进 `ReadPtr`，避免死循环

### 新增 CLI 功能

1. 在 `KF.hpp` 的 `KCLI` namespace 中添加函数声明
2. 在 `KCLI.cpp` 的 `namespace KF::KCLI` 中实现
3. 使用 `Color::xxx` 设置输出颜色
4. 更新本文档的 KCLI 章节

### 新增 KTIMER 功能

1. 在 `KF.hpp` 的 `KTIMER` namespace 中添加函数声明
2. 在 `KTIMER.cpp` 的 `namespace KF::KTIMER` 中实现
3. 使用 `Color::xxx` 设置输出颜色，保持与 KCLI 统一的框线风格
4. 更新本文档的 KTIMER 章节
