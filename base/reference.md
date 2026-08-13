# KForge API Reference

> **最后更新**: 2026-08-10 | **标准**: C++17 | **编译器**: MSVC 19.44 (x64)
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
- [构建系统](#构建系统)
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
├── KUTILITY     内部工具函数(不必了解)
└── KCLI         命令行 UI、链式 I/O
```

头文件底部 `using namespace KF::KLOGGER;` 将错误码、`Color`、`LogLevel`、`Module`、`MakeCode` 引入全局，调用处无需前缀。

---

## KLOGGER 日志模块

**源文件**: `KLOGGER.cpp` | **命名空间**: `KF::KLOGGER`

### Color 颜色常量

VT100 转义序列，`KBegin()` 和 `Log()` 自动启用对应句柄的 VT100 处理。

| 常量 | 转义码 | 用途 |
|------|--------|------|
| `Reset` | `\033[0m` | 重置所有样式 |
| `Red` | `\033[31m` | Fatal / koutF |
| `Green` | `\033[32m` | Info |
| `Yellow` | `\033[33m` | 标准黄色 |
| `Blue` / `Magenta` / `Cyan` | `34m` / `35m` / `36m` | 基本色 |
| `LightYellow` | `\033[93m` | Warning / koutW |
| `Orange` | `\033[38;5;208m` | Error / koutE |
| `SkyBlue` | `\033[38;5;75m` | kout 默认色 / 框线 |
| `Bold` | `\033[1m` | 加粗（可叠加） |

### LogLevel / Module / MakeCode

```cpp
enum class LogLevel : uint32_t { Info=1, Warning=2, Error=3, Fatal=4 };
```

| Module | 值 | Module | 值 |
|--------|----|--------|----|
| `Unknown` | `0x00` | `KSON` | `0x03` |
| `Common` | `0x01` | `KTIMER` | `0x04` |
| `KFIO` / `KBIGNUM` | `0x02` | `KCLI` | `0x05` |

```cpp
constexpr Code MakeCode(uint32_t module, LogLevel level, uint32_t type, uint32_t id) noexcept;
```

错误码格式 `0x[aa][b][cc][ddd]`：`[aa]`=模块(bit24-31) `[b]`=等级(bit20-23) `[cc]`=类型(bit12-19) `[ddd]`=序号(bit0-11)。

> 错误码 `extern` 声明在 `KF.hpp`，定义在 `KLOGGER.cpp`（通过 `MakeCode` 组装），改码时只需重编译 `KLOGGER.cpp`。

### KLOG_* 宏

自动捕获 `__FILE__` / `__LINE__` / `__FUNCTION__`：

```cpp
KLOG_INFO(code, extra)      // Info 级别，程序继续
KLOG_WARNING(code, extra)   // Warning 级别，程序继续
KLOG_ERROR(code, extra)     // Error 级别，程序继续
KLOG_FATAL(code, extra)     // Fatal 级别，system("pause") + exit(EXIT_FAILURE)
```

输出到 `stderr`，格式：`[ERROR]  Code: 0x03301001 Msg: ... | extra | at file:line (func)`

### Table 码表

`extern std::unordered_map<Code, std::string_view> Table;` 定义在 `KLOGGER.cpp`，键为错误码，值为说明文本。新增错误码时必须同步添加条目。

---

## KSON 解析模块

**源文件**: `KSON.cpp` | **命名空间**: `KF::KSON`

### NodeType 枚举

```cpp
enum class NodeType { kInt, kDec, kBig, kStr, kBool, kArr, kObj, kNull };
```

### Node 类

`std::variant` 存储的 KSON 数据树节点。

| 构造 | 类型 |
|------|------|
| `Node()` | kNull |
| `Node(bool)` | kBool |
| `Node(long long)` | kInt |
| `Node(double)` | kDec |
| `Node(BigNum)` | kBig |
| `Node(string)` | kStr |
| `Node(vector<Node>)` | kArr |
| `Node(vector<pair<string,Node>>)` | kObj |

类型判断：`IsNull()` `IsBool()` `IsInt()` `IsDec()` `IsBig()` `IsNumber()` `IsString()` `IsArray()` `IsObject()`

取值（类型不匹配时 `KLOG_ERROR(KSON_TYPE_MISMATCH)`）：

| 方法 | 返回 | 备注 |
|------|------|------|
| `AsBool()` | `bool` | |
| `AsInt()` | `long long` | |
| `AsDec()` | `double` | int 自动转 double |
| `Big()` | `const BigNum&` | 大数引用 |
| `AsStr()` | `string_view` | |
| `AsArr()` | `const vector<Node>&` | |
| `AsObj()` | `const vector<pair<string,Node>>&` | |

| 方法 | 说明 |
|------|------|
| `size()` | 数组/对象元素个数，标量返回 0 |
| `find(key)` | 对象按键查找（区分大小写），返回 `const Node*`，未找到 `nullptr` |
| `at(index)` | 数组按下标查找，越界 `nullptr` |

### PathSeg

路径片段：`PathSeg("key")` 键访问，`PathSeg(3)` 下标访问。

### NodePtr（别名 `kson`）

KSON 树智能指针，持有根节点和访问路径，支持链式 `[]` 延迟解析。

```cpp
kson doc = read(Preprocess(KFIO::ReadFileRaw("cfg.txt")));
auto val = doc["test"]["main"]["Array_Test"][0];
```

| 方法 | 返回 | 路径不存在时 |
|------|------|-------------|
| `TryResolve()` | `const Node*` | `nullptr` |
| `Resolve()` | `const Node*` | KLOG_FATAL(UNKNOWN) |
| `Str()` / `Int()` / `Dec()` / `Big()` / `Bool()` / `Size()` | 对应类型 | Fatal |
| `Exists()` | `bool` | `false` |
| `Auto()` | `string` | `"null"` |

`Auto()` 按类型自动转换：kNull→`null` kBool→`true/false` kInt→整数 kDec→浮点(去尾零,15位) kBig→大数字符串 kStr→原文 kArr→`[e1,...]` kObj→`{"key":val,...}`

静态工厂：`Parse(text)` 从字符串解析，`ParseFile(filepath)` 从文件解析。

### Parser（内部）

递归下降解析器：`ParseStr`(字符串,处理转义) `ParseNum`(整数/小数/科学计数法/大数,支持B后缀,自动转BigNum) `ParseVal`(分派) `ParseArr`(数组,支持尾随逗号) `ParseObj`(对象,支持尾随逗号/重复键后覆盖) `ParseImplicitObj`(隐式顶层对象)

### 自由函数

```cpp
string Preprocess(string_view raw);     // 去注释(#)、去空白、保留字符串内容
kson   read(string_view processed);      // 解析预处理后文本
kson   ReadKsonFile(std::string filepath);    // 一站式：读文件→预处理→解析
> **配置文件路径**：kson 文件统一放在根目录 `config/` 下。测试数据使用 `config/test/cfg.kson`，通用配置使用 `config/config.kson`。路径相对于项目根目录（运行 exe 时 CWD 须为项目根）。
```

---

## KFIO 文件读写模块

**源文件**: `KFIO.cpp` | **命名空间**: `KF::KFIO`

```cpp
std::string ReadFileRaw(std::string_view filepath);
```

二进制模式读取文件全部内容。失败时 Fatal 并附带绝对路径 + errno。

| 错误码 | 级别 | 说明 |
|--------|------|------|
| `KFIO_FILE_OPEN_FAIL` | Fatal | 文件打开失败 |
| `KFIO_FILE_READ_FAIL` | Fatal | 文件读取失败 |

---

## KCLI 命令行交互模块

**源文件**: `KCLI.cpp` | **命名空间**: `KF::KCLI`

### Kout / Kin 链式 I/O

`Kout` 每次 `<<` 自动套用默认颜色，遇 `endl`/`flush` 先输出 `Reset`。`Kin` 每个 `>>` 读一行并按类型转换，失败置 0 并 `KLOG_WARNING`。

| 全局对象 | 默认颜色 | 用途 |
|----------|----------|------|
| `kout` | SkyBlue | 普通输出 |
| `koutW` | LightYellow | 警告输出 |
| `koutE` | Orange | 错误输出 |
| `koutF` | Red | 致命输出 |
| `kin` | — | 链式输入 |

```cpp
kout  << "普通信息" << 42 << std::endl;       // 天蓝色
koutE << Color::Red << "临时换色" << std::endl; // 临时红色
int age; string name; kin >> age >> name;       // 每次读一行
```

### CLI 功能函数

| 函数 | 说明 |
|------|------|
| `KBegin(title, description="")` | 初始化 CLI：启用 VT100、UTF-8、设置标题、打印标题框。直接传字符串，无需 kson |
| `KOptions(kson menu)` | 显示选项菜单，返回选中索引(0-based)。入参 `"title"`/`"options":[...]` |
| `kpause()` | 显示"按任意键继续..."并等待 |
| `KEnd()` | `kpause()` + `exit(0)` |

---

## KTIMER 计时器模块

**源文件**: `KTIMER.cpp` | **命名空间**: `KF::KTIMER`

```cpp
enum class TimeUnit { ns, us, ms, s };
enum class TimerState { Running, Paused };
```

| 函数 | 返回 | 说明 |
|------|------|------|
| `AddTimer(name, unit)` | `bool` | 新建计时器（创建即开始），同名覆盖返回 false |
| `PauseTimer(name)` | `bool` | 暂停（累计已运行时间） |
| `StartTimer(name)` | `bool` | 恢复已暂停的计时器 |
| `DeleteTimer(name)` | `bool` | 删除计时器 |
| `GetTimer(name)` | `double` | 获取累计时间（不存在返回 -1.0） |
| `PrintTimer(name)` | `void` | 打印单个计时器（格式化框） |
| `PrintAllTimers()` | `void` | 打印所有计时器（格式化表格，按名称排序） |

---

## KBIGNUM 大数运算模块

**源文件**: `KBIGNUM.cpp` | **命名空间**: `KF::KBIGNUM`

### 存储模型

base = 10⁹，每个 `limb`（`uint32_t`）存储 9 位十进制数字。选择 10⁹ 而非 2³² 是为了简化 `ToStr` 的十进制输出。

| 成员 | 类型 | 说明 |
|------|------|------|
| `limbs` | `vector<limb>` | 低位在前，`limbs[0]` 是最低 9 位 |
| `isneg` | `bool` | 负数标志 |
| `scale` | `size_t` | 小数位数 |

类型别名：`using limb = uint32_t; using dlimb = uint64_t;`

### Normalize 字符串合法化

```cpp
std::string Normalize(const std::string& str);
```

规整为 `[+|-]digits[.digits]` 格式：提取有效字符、去前导零、去小数点后尾随零、多余小数点取分隔、多余正负号异或、全零返回 `"0"`。

| 输入 | 输出 |
|------|------|
| `000012340.3221000000` | `+12340.3221` |
| `-.123.123.` | `-0.123123` |
| `-0` | `0` |
| `dw1-abca00432.432100` | `+100432.4321` |

### ToBig / ToStr

```cpp
static BigNum ToBig(const std::string& str);  // Normalize 后字符串 → BigNum
std::string ToStr() const;                     // BigNum → 字符串
```

**ToBig** 流程：取符号 → 定位小数点算 scale → 提取纯数字 → 从右往左每九位截取一个 limb（`substr` 批量截取，O(n)）→ 去高位零块 → 零值强制 `isneg=false`。

**ToStr** 流程：零值返回 `"0"` → 最高位 limb 不补零，其余补零到 9 位 → 按 scale 插入小数点 → 负数加 `-`。

### 构造函数

| 构造 | 说明 |
|------|------|
| `BigNum()` | 默认构造，零值 |
| `BigNum(const string& str)` | 字符串构造（Normalize → ToBig） |
| `BigNum(const dlimb& num)` | 数字构造（to_string → ToBig） |

### 运算接口

| 分类 | 函数 | 状态 |
|------|------|------|
| 绝对值运算 | `AbsAdd` `AbsSub` `AbsCmp` | 已实现 |
| 绝对值运算 | `AbsMul` `AbsDiv` `AbsMod` `AbsPow` | 待实现 |
| 用户运算符 | `operator+ - * / %` `Pow` | 待实现 |
| 比较运算符 | `operator<` | 已实现（使用 `AbsCmp`） |
| 比较运算符 | `operator== != <= > >=` | 待实现 |
| 输出 | `friend operator<<` | 调用 `ToStr` |

> **设计约定**：自由函数（`AbsCmp` 等）声明放在 `BigNum` 类定义之前，需前向声明 `class BigNum;`。类内调用时省略 `KBIGNUM::` 前缀。`AbsAdd` 中使用 `(std::max)` 避免 Windows max 宏冲突。

```cpp
#include "KF.hpp"
using namespace KBIGNUM;
BigNum a("123456789012345678901234567890");
BigNum b("-0.0001");
kout << a << "\n" << b << "\n";  // 123456789012345678901234567890 / -0.0001
```

---

## KSON 数据格式

类 JSON 自定义格式，支持：双引号字符串、整数(`long long`)、小数(`double`)、大数(`BigNum`)、布尔、`null`、数组、对象、`#`行注释、尾随逗号、隐式顶层对象（无需外层`{}`）、转义字符(`\n \t \r \b \" \\`)、重复键后覆盖前。

**大数支持**：
- 科学计数法（如 `1.23e50`）自动解析为 `BigNum`
- 数字后加 `B` 后缀（如 `123456789B`）强制存储为 `BigNum`
- 整数超出 `long long` 范围（`9223372036854775807`）自动转为 `BigNum`
- 通过 `Big()` 方法获取 `BigNum` 引用

精度限制：整数超出 `long long` 触发 `KSON_PARSE_NUMOR`，科学计数法指数过大触发 `KSON_PARSE_BIG_EXP`。

```
# 应用配置
"app_name": "MyApp",
"version": 1.0,
"settings": { "debug": true, "tags": ["prod", "v2",] },
"empty": []
```

---

## 构建系统

### 一键构建

| 脚本/文件 | 位置 | 用途 |
|-----------|------|------|
| `init_build.bat` | 根目录 | 预编译 `base/KF.lib` + 在 `study/` 下含 .cpp 的目录生成 `build.bat` / `fast_build.bat` |
| `build.bat` | study/ 子目录 | 由 init_build 生成，美观 CLI 选择文件编译 |
| `fast_build.bat` | study/ 子目录 | 由 init_build 生成，自动编译全部文件 |
| `cancel_init.bat` | 根目录 | 清理所有构建产物（KF.lib、obj、Release/、生成的 bat） |
| `CMakeLists.txt` | 根目录 | 可选，供 IDE / CI 使用（实际编译用 build.bat） |

### init_build.bat

运行 `init_build.bat` 后：
1. 设置 MSVC 环境
2. 增量预编译 `base/*.cpp` → `base/obj/*.obj` → `base/KF.lib`
3. 扫描 `study/` 下所有含 .cpp 的子目录
4. 每个目录生成 `build.bat` + `fast_build.bat`

### build.bat / fast_build.bat

每个脚本自包含，直接调用 `cl.exe` + `link.exe`，不依赖 CMake：

- **build.bat**：显示文件列表，输入编号选择（如 `1,3`），Enter 编译全部。每次运行前清空 `Release/`
- **fast_build.bat**：自动编译全部文件，每次运行前清空 `Release/`
- 编译产物（exe）输出到同目录的 `Release/` 文件夹

### 编译要求

- Visual Studio 2022（含 C++ 工具链）
- Windows 10 SDK
- 首次使用运行 `init_build.bat`

### 配置文件结构

```
项目根目录
├── config/
│   ├── config.kson        # 通用配置（KBegin 使用）
│   └── test/
│       └── cfg.kson       # 测试数据配置
├── base/                  # 基础库源码
├── test/                  # 测试程序
└── study/                 # 学习示例

---

## 错误码速查表

### 通用 (01) / KFIO (02)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `TEST_INFO` | `0x01101001` | Info | 测试-信息 |
| `TEST_WARN` | `0x01201002` | Warning | 测试-警告 |
| `TEST_ERROR` | `0x01301003` | Error | 测试-错误 |
| `TEST_FATAL` | `0x01401004` | Fatal | 测试-严重错误 |
| `KFIO_FILE_OPEN_FAIL` | `0x02401001` | Fatal | 文件打开失败 |
| `KFIO_FILE_READ_FAIL` | `0x02401002` | Fatal | 文件读取失败 |

### KSON (03)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KSON_PARSE_STRE` | `0x03301001` | Error | 字符串不以双引号开头 |
| `KSON_PARSE_STR_NOEND` | `0x03301002` | Error | 字符串缺少闭合双引号 |
| `KSON_PARSE_MULPOINT` | `0x03201003` | Warning | 多个小数点 |
| `KSON_PARSE_NUM_UE` | `0x03201004` | Warning | 数字含非法字符 |
| `KSON_PARSE_NUMOR` | `0x03301005` | Error | 数字超出类型范围 |
| `KSON_PARSE_NUM_USTYPE` | `0x03301006` | Error | 不支持的数字类型 |
| `KSON_PARSE_ESCAPE_SPECIAL` | `0x03201007` | Warning | 无效转义序列 |
| `KSON_PARSE_UNFINISHED_ESCAPE` | `0x0340100A` | Fatal | 转义序列不完整 |
| `KSON_PARSE_BIG_EXP` | `0x0330100C` | Error | 科学计数法指数过大 |
| `KSON_PARSE_VAL_END` | `0x033010AA` | Error | 读到末尾仍缺值 |
| `KSON_PARSE_VAL_ERROR` | `0x033010AC` | Error | 值类型无法识别 |
| `KSON_PARSE_ARR_BEGIN` | `0x033010AE` | Error | 期望数组起始 `[` |
| `KSON_PARSE_ARRUE` | `0x033010B0` | Error | 数组中出现非预期字符 |
| `KSON_PARSE_OBJ_BEGIN` | `0x033010B2` | Error | 期望对象起始 `{` |
| `KSON_PARSE_OBJ_KEY_QUOTE` | `0x033010B4` | Error | 对象键未用双引号包裹 |
| `KSON_PARSE_OBJ_SEPERATOR` | `0x033010B6` | Error | 键后缺少冒号 |
| `KSON_PARSE_OBJUE` | `0x033010B8` | Error | 对象中出现非预期字符 |
| `KSON_PARSE_TRAIL` | `0x03202011` | Warning | 结尾有多余字符 |
| `KSON_TYPE_MISMATCH` | `0x03401001` | Fatal | AsXxx 类型不匹配 |

### KTIMER (04) / KCLI (05) / KBIGNUM (02) / 未知 (00)

| 常量 | 码值 | 等级 | 说明 |
|------|------|------|------|
| `KTIMER_NOT_FOUND` | `0x04201001` | Warning | 计时器不存在 |
| `KTIMER_ALREADY_EXISTS` | `0x04201002` | Warning | 计时器已存在 |
| `KTIMER_STATE_ERROR` | `0x04201003` | Warning | 计时器状态不允许此操作 |
| `KCLI_INPUT_INVALID` | `0x05201001` | Warning | 输入解析失败 |
| `KBIGNUM_MULPOINT` | `0x02201002` | Warning | 多余的小数点 |
| `KBIGNUM_INVALIDCHAR` | `0x02201004` | Warning | 数字中含非法字符 |
| `UNKNOWN` | `0x00400000` | Fatal | 未知错误码 |

---

## 快速上手

```cpp
#include "base/KF.hpp"
using namespace KSON;
using namespace KCLI;

int main() {
    kson file = ReadKsonFile("config/config.kson");
    kson main = file["test"]
    KBegin(main);
    auto arr = doc["tags"];
    for (size_t i = 0; i < arr.size(); i++)
        kout << "  [" << i << "] " << arr[i].Auto() << std::endl;
    size_t choice = KOptions(main["settings"]);
    KEnd();
}
```

### 测试文件

| 文件 | 模块 | 覆盖功能 |
|------|------|----------|
| `dbgKSON.cpp` | KSON | 解析、类型判断、取值、路径访问、find/at、Auto、转义、注释、尾随逗号、重复键、BigNum/科学计数法 |
| `dbgKFIO.cpp` | KFIO | ReadFileRaw 读取/验证、Fatal 测试 |
| `dbgKLOGGER.cpp` | KLOGGER | KLOG_* 宏、错误码、MakeCode、Table、枚举、Color |
| `dbgKCLI.cpp` | KCLI | kout/koutW/koutE/koutF 输出、kin 输入、KOptions 菜单 |
| `dbgKTIMER.cpp` | KTIMER | AddTimer/PauseTimer/StartTimer/DeleteTimer/GetTimer/Print* |
| `dbgKBIGNUM.cpp` | KBIGNUM | Normalize、ToBig+ToStr 往返、limbs/scale/isneg 验证、边界值 |

> Fatal 级别测试放在文件最后执行，触发后终止程序。所有测试脚本使用 `SECTION`/`CHECK`/`SHOW` 宏，配合 `kout`/`koutW`/`koutE`/`koutF` 彩色输出。

### 维护指南

**新增错误码**：`KF.hpp` 加 `extern const Code` → `KLOGGER.cpp` 加 `MakeCode` 定义 + `Table` 条目 → 更新本文档。

**新增模块功能**：`KF.hpp` 对应 namespace 加声明 → `.cpp` 中 `namespace KF::XXX` 实现 → 更新本文档。
