# Mini Lisp 项目总说明

## 1. 项目概述

本项目实现了一个 C++ 编写的 Mini Lisp 解释器。解释器支持 Lisp 基本表达式的分词、解析、求值、作用域管理、内置过程调用、特殊形式处理以及交互式 REPL。项目在原有基础上进一步扩展了数学函数、字符串函数、列表与高阶过程、图形绘制接口，并通过 Scheme 脚本实现了一个可运行、可点击、可判定胜负的黑白棋游戏。

当前项目的重点不仅是“能运行”，也包括工程结构、代码规范和可读性。`src` 下的核心类型、结构体、枚举、注册表和关键实现区均已补充中文多行注释，便于阅读和维护。

## 2. 目录结构说明

- `src/`：解释器核心源码，包括值系统、分词器、解析器、求值环境、内置过程、特殊形式、REPL、图形模块等。
- `scripts/`：Lisp/Scheme 示例脚本与测试脚本，其中 `graphics_demo.scm` 是黑白棋游戏脚本。
- `docs/`：项目说明文档，包括图形接口说明、原始说明文档和本总说明。
- `bin/`：编译后生成的可执行文件目录。
- `build/`：CMake 构建目录。
- `CMakeLists.txt`：项目构建配置，当前会自动收集 `src/*.cpp` 参与编译。
- `.clang-format`：代码格式化规则文件。

## 3. 核心模块说明

### 3.1 值系统

相关文件：`src/value.h`、`src/value.cpp`

值系统定义了解释器运行时的所有数据类型，包括：

- `BooleanValue`：布尔值，对应 `#t` 和 `#f`。
- `NumericValue`：数字值，内部统一使用 `double` 保存。
- `StringValue`：字符串值。
- `NilValue`：空列表 `()`。
- `SymbolValue`：符号，用于变量名、函数名和 quoted 数据。
- `PairValue`：cons 单元，用于表示列表和点对。
- `BuiltinProcValue`：C++ 内置过程包装。
- `LambdaValue`：用户自定义函数和闭包。

### 3.2 分词与解析

相关文件：`src/token.h`、`src/token.cpp`、`src/tokenizer.h`、`src/tokenizer.cpp`、`src/parser.h`、`src/parser.cpp`

Tokenizer 负责把源代码字符串切分为 token，支持数字、字符串、布尔值、标识符、括号、quote、quasiquote、unquote 和注释。

Parser 负责把 token 流转换为运行时 Value 树。列表、点对和 quote 简写都会在这里转换成统一的数据结构，供后续求值器处理。

### 3.3 求值环境

相关文件：`src/eval_env.h`、`src/eval_env.cpp`

`EvalEnv` 是解释器运行时的核心之一，负责：

- 管理当前作用域的符号绑定。
- 链接父环境，形成词法作用域链。
- 初始化全局内置过程。
- 求值表达式。
- 调用普通内置过程和用户自定义 lambda。
- 创建子环境以支持函数调用和闭包。

### 3.4 特殊形式

相关文件：`src/forms.h`、`src/forms.cpp`

特殊形式与普通函数不同，它们接收未求值参数，可以自行控制求值顺序。当前支持：

- `quote`
- `define`
- `if`
- `and`
- `or`
- `lambda`
- `cond`
- `begin`
- `let`
- `quasiquote`

### 3.5 公共工具

相关文件：`src/lisp_utils.h`、`src/lisp_utils.cpp`

该模块集中保存运行时公共规则，包括：

- 参数数量检查。
- 数字、整数、字符串提取。
- 假值判断。
- 正规列表检查。
- 深度相等比较。
- 常用 Lisp 值构造函数。

这样可以避免多个内置过程重复实现相同逻辑，也能保持错误处理和类型规则一致。

## 4. 方向 1：添加更多的内置过程

本项目在内置过程扩展上做了较多工作，主要集中在数学函数、字符串函数、列表处理、高阶过程和图形绘制接口。

### 4.1 数学相关内置过程

除基础的 `+`、`-`、`*`、`/`、比较运算外，项目还扩展了更多数学函数，例如：

- `abs`
- `expt`
- `quotient`
- `remainder`
- `modulo`
- `sqrt`
- `sin`
- `cos`
- `tan`
- `asin`
- `acos`
- `atan`
- `log`
- `exp`
- `floor`
- `ceil`
- `round`
- `max`
- `min`
- `gcd`
- `lcm`

这些过程让 Mini Lisp 可以执行更复杂的数值计算，也为后续图形绘制、游戏逻辑、算法脚本提供了基础能力。

### 4.2 字符串相关内置过程

原始 Mini Lisp 没有字符串操作能力，本项目补充了常见字符串过程：

- `string-append`：字符串拼接。
- `string-length`：获取字符串长度。
- `string-ref`：访问指定位置字符。
- `substring`：截取子串。
- `string=?`：字符串相等比较。
- `string<?`：字符串小于比较。
- `string>?`：字符串大于比较。
- `number->string`：数字转字符串。
- `string->number`：字符串转数字，失败时返回 `#f`。
- `string-upcase`：转换为大写。
- `string-downcase`：转换为小写。

这些过程让 Lisp 脚本可以进行基本文本处理，也提高了程序输出、提示信息和交互逻辑的表达能力。

### 4.3 列表与高阶过程

项目支持了常用列表操作和高阶过程：

- `car`
- `cdr`
- `cons`
- `length`
- `list`
- `append`
- `map`
- `filter`
- `reduce`
- `apply`
- `eval`

这些过程使 Mini Lisp 更接近函数式语言的使用方式，也让脚本能够表达更复杂的数据处理逻辑。

### 4.4 图形绘制内置过程

本项目实现了一个 Windows 2D 图形层，使用 Win32/GDI 创建窗口、维护双缓冲，并通过 Lisp 内置过程暴露绘图和事件接口。

支持的图形过程包括：

- `graphics-open`
- `graphics-close`
- `graphics-clear`
- `graphics-color`
- `graphics-line`
- `graphics-rect`
- `graphics-circle`
- `graphics-text`
- `graphics-refresh`
- `graphics-poll-event`
- `graphics-wait-event`
- `graphics-sleep`

图形模块采用双缓冲绘制，脚本先画到离屏缓冲区，再刷新到窗口，避免棋盘频繁闪烁。事件系统支持鼠标点击、鼠标移动、键盘按键和窗口关闭事件。

### 4.5 黑白棋脚本

相关脚本：`scripts/graphics_demo.scm`

该脚本基于图形内置过程实现了一个正常运行的黑白棋游戏：

- 绘制棋盘和棋子。
- 高亮合法落子位置。
- 支持鼠标点击下棋。
- 自动翻转被夹住的棋子。
- 判断当前玩家是否合法可走。
- 无棋可走时自动跳过。
- 双方都无法落子时结束游戏。
- 统计黑白棋数量并显示胜负结果。

这说明图形接口不仅能画简单图案，也可以支撑完整交互式小游戏。

### 4.6 read/readline 方向说明

题目中提到的 `read` 内置过程实现难度较高，因为它需要从输入流读取一个完整 Lisp 外部表示，并正确处理括号、字符串、注释和 EOF。

当前项目尚未实现独立的 `read` 或 `readline` 内置过程，但 REPL 已经具备多行读取与完整表达式判断能力。后续如果继续扩展，可以优先实现较简单的 `readline`：

1. 从标准输入读取一整行字符串。
2. 返回 `StringValue`。
3. 用户可通过 `eval` 对字符串解析后的表达式求值。

再进一步，可以复用现有 Tokenizer 与 Parser，实现真正的 `read`。

## 5. 方向 2：改善用户体验

本项目对 REPL 用户体验做了增强，使其不只是简单的“读一行、执行一行”。

### 5.1 多行输入

相关文件：`src/repl.cpp`、`src/line_editor.h`、`src/line_editor.cpp`

REPL 支持连续多行输入。当表达式括号尚未闭合时，解释器不会立即执行，而是继续显示续行提示，等待用户补全表达式。

示例：

```scheme
mini-lisp> (define (square x)
...           (* x x))
()
mini-lisp> (square 4)
16
```

该功能让用户可以更自然地输入函数定义、`let`、`cond`、多层嵌套表达式等复杂代码。

### 5.2 自动缩进

REPL 会根据尚未闭合的括号数量计算续行缩进，使多行输入更容易阅读。该能力由 `LineEditor::countOpenParens` 和 REPL 续行逻辑共同完成。

### 5.3 光标编辑

`LineEditor` 支持基础行编辑能力，包括：

- 左右移动光标。
- Home/End。
- Backspace/Delete。
- Tab 缩进。
- Ctrl+C / Ctrl+D 处理。

这比简单的 `std::getline` 更适合交互式解释器使用。

### 5.4 语法高亮

REPL 对输入内容做轻量级 Lisp 语法高亮，能够区分：

- 括号。
- quote 相关符号。
- 字符串。
- 数字。
- 布尔值。
- 特殊形式。
- 内置过程。
- 注释。

语法高亮只服务显示，不参与真正解析。真正的语法合法性仍由 Tokenizer 和 Parser 判断。

### 5.5 错误显示

REPL 会捕获语法错误和运行时错误，并以 `Error: ...` 的形式输出，避免解释器直接崩溃。对于未闭合输入，REPL 也会等待用户继续输入，而不是立即报错。

## 6. 代码规范与工程改进

本项目对 `src` 下代码进行了结构化整理：

- 将公共 Lisp 运行时工具提取到 `lisp_utils` 模块。
- 简化 `builtins.h` 和 `forms.h` 的公开接口。
- 将普通内置过程、特殊形式和图形接口职责区分清楚。
- 将图形状态拆分为 `Point`、`Size`、`GraphicsEvent`、`WindowState`、`BackBuffer`、`GraphicsState` 等结构。
- 使用 `.clang-format` 统一格式化代码。
- 为主要类、结构体、枚举、注册表和实现区域补充中文多行注释。
- 通过完整测试确认重构和注释调整没有影响行为。

这些改动主要服务于可读性、可维护性、可复用性和正确性。

## 7. 运行方式

编译项目：

```powershell
cmake --build build
```

运行完整测试：

```powershell
bin\mini_lisp.exe --test
```

运行黑白棋图形脚本：

```powershell
bin\mini_lisp.exe scripts\graphics_demo.scm
```

进入 REPL：

```powershell
bin\mini_lisp.exe
```

## 8. 当前验证结果

当前项目已通过完整测试：

```text
Total: 437/437
```

说明现有基础语法、特殊形式、内置过程、列表处理、高阶过程和 SICP 风格测试均保持正常。

## 9. 后续可扩展方向

后续可以继续扩展：

- 实现真正的 `read` 或较简单的 `readline`。
- 增加文件读写过程。
- 增加更多字符串查找、替换、分割函数。
- 增加随机数、时间、排序等实用内置过程。
- 为图形模块增加图片加载、键盘状态查询、定时器等能力。
- 将图形接口抽象为跨平台后端，如 SDL、SFML 或 Raylib。
- 为 Lisp 脚本增加标准库文件，减少游戏脚本中的重复函数。
- 增加更多面向错误情况的自动化测试。
