# src 下 C++ 源文件逐文件说明

本文档逐个说明 `src` 目录下的 `.cpp` 文件职责和主要函数。测试框架文件 `rjsj_test.hpp` 不在本文档范围内。

## 1. `builtins.cpp`

### 概览

`builtins.cpp` 实现 Mini Lisp 的普通内置过程。所谓“普通内置过程”，是指参数在进入函数前已经由求值器完成求值，之后由 C++ 函数直接处理。这与 `if`、`define`、`lambda` 等特殊形式不同。

该文件覆盖的能力非常广：

- 数学运算。
- 字符串处理。
- 控制台输出和退出。
- 类型谓词。
- 列表操作。
- 高阶过程。
- 数值比较。
- 相等性判断。
- 图形过程注册。

文件末尾的 `BUILTIN_FUNCTIONS` 是全局内置过程注册表。`EvalEnv::createGlobal()` 会读取该表，把每个 C++ 函数包装成 `BuiltinProcValue` 并绑定到全局环境中。

### 主要函数说明

- `compareNumbers`：数字比较过程的公共实现。`=`、`<`、`>`、`<=`、`>=` 都复用该函数，避免重复写逐对比较逻辑。
- `add`：实现 `+`，支持任意数量参数，无参数时返回 `0`。
- `sub`：实现 `-`，一个参数时取相反数，多个参数时从左到右连续相减。
- `mul`：实现 `*`，支持任意数量参数，无参数时返回 `1`。
- `div`：实现 `/`，一个参数时返回倒数，多个参数时从左到右连续相除。
- `absValue`：实现 `abs`，返回数字绝对值。
- `expt`：实现 `expt`，调用 `std::pow` 进行幂运算。
- `quotient`：实现 `quotient`，返回截断后的整数商。
- `remainder`：实现 `remainder`，使用 `std::fmod` 得到余数。
- `modulo`：实现 `modulo`，采用更符合 Lisp 习惯的模运算语义。
- `sqrtProc`、`sinProc`、`cosProc`、`tanProc`、`asinProc`、`acosProc`、`atanProc`：实现常见数学函数。
- `logProc`：实现 `log`，支持单参数自然对数和双参数换底对数。
- `expProc`、`floorProc`、`ceilProc`、`roundProc`：实现指数、向下取整、向上取整和四舍五入。
- `maxProc`、`minProc`：返回多个数字中的最大值或最小值。
- `gcdProc`、`lcmProc`：实现最大公约数和最小公倍数。
- `stringAppendProc`：实现 `string-append`，拼接多个字符串。
- `stringLengthProc`：实现 `string-length`，返回字符串长度。
- `stringRefProc`：实现 `string-ref`，返回指定位置的单字符字符串。
- `substringProc`：实现 `substring`，按起止下标截取子串。
- `stringEqProc`、`stringLessProc`、`stringGreaterProc`：实现字符串比较。
- `numToStringProc`：实现 `number->string`，将数字转为字符串并去掉多余尾零。
- `stringToNumProc`：实现 `string->number`，转换失败时返回 `#f`。
- `stringUpcaseProc`、`stringDowncaseProc`：实现字符串大小写转换。
- `print`：打印所有参数并换行。
- `display`：打印所有参数但不自动换行。
- `newline`：输出换行。
- `displayln`：打印一个参数并换行。
- `errorProc`：主动抛出 Lisp 运行时错误。
- `exitProc`：退出解释器进程。
- `atomp`：判断参数是否为原子值。
- `booleanp`、`integerp`、`listp`、`numberp`、`nullp`、`pairp`、`procedurep`、`stringp`、`symbolp`：实现各类类型谓词。
- `car`：返回 pair 的左值。
- `cdr`：返回 pair 的右值。
- `cons`：构造一个 pair。
- `length`：计算正规列表长度。
- `list`：把参数构造成 Lisp 列表。
- `append`：拼接多个列表。
- `map`：对列表中每个元素应用过程并返回结果列表。
- `filter`：保留使谓词为真的列表元素。
- `reduce`：从左到右归约列表。
- `applyProc`：实现 `apply`，把列表参数展开后调用过程。
- `evalProc`：实现 `eval`，对传入表达式重新求值。
- `equalNum`、`less`、`greater`、`lessEqual`、`greaterEqual`：实现数值比较。
- `evenp`、`oddp`、`zerop`：实现数字谓词。
- `eqp`：实现 `eq?`，对简单值按值比较，对复合对象按引用比较。
- `equalp`：实现 `equal?`，进行递归深度比较。
- `notp`：实现 `not`，仅 `#f` 被视为假。
- `BUILTIN_FUNCTIONS`：普通内置过程注册表，包含普通内置过程和图形内置过程。

## 2. `eval_env.cpp`

### 概览

`eval_env.cpp` 是解释器求值系统的核心实现文件。它负责创建全局环境、管理符号绑定、查找变量、创建子环境、求值表达式，并完成过程调用。

Mini Lisp 的词法作用域也由该文件实现。每个 `EvalEnv` 都保存当前作用域的符号表，并可指向父环境。查找变量时，会先查当前环境，再逐层查父环境。

### 主要函数说明

- `EvalEnv::EvalEnv`：构造一个空环境，默认没有父环境。
- `EvalEnv::createGlobal`：创建全局环境，并把 `BUILTIN_FUNCTIONS` 中的所有内置过程注册进去。
- `EvalEnv::lookupBinding`：查找符号绑定。若当前环境没有该符号，则继续查找父环境；如果最终找不到则抛出错误。
- `EvalEnv::defineBinding`：在当前环境中定义或覆盖一个符号绑定。
- `EvalEnv::createChild`：根据形参名和实参值创建子环境，主要用于 lambda 调用和局部作用域。
- `EvalEnv::evalArguments`：对表达式参数列表逐一求值，用于普通函数调用前的参数准备。
- `EvalEnv::evalSequence`：按顺序求值多个表达式，并返回最后一个表达式的值。
- `EvalEnv::evalCombination`：求值一个组合式。它会先识别特殊形式，再处理普通过程调用。
- `EvalEnv::applyLambda`：调用用户自定义 lambda，创建闭包子环境并执行函数体。
- `EvalEnv::eval`：解释器最重要的入口之一。它处理自求值对象、符号查找、列表组合式求值等情况。
- `EvalEnv::apply`：统一的过程调用入口。它可以调用内置过程，也可以调用用户定义的 lambda。

## 3. `forms.cpp`

### 概览

`forms.cpp` 实现特殊形式。特殊形式与普通函数不同，它们的参数不会在进入函数前自动求值，因此可以自己决定哪些表达式求值、何时求值、是否短路。

例如 `if` 只会求值被选中的分支，`and` 和 `or` 会短路，`define` 会把符号写入环境，`lambda` 会构造闭包而不是立即执行函数体。

### 主要函数说明

- `takeTail`：从参数列表中截取指定位置之后的所有表达式，常用于提取函数体。
- `evalSequence`：按顺序求值多个表达式，并返回最后一个结果。
- `quoteForm`：实现 `quote`，直接返回参数表达式，不进行求值。
- `lambdaForm`：实现 `lambda`，读取形参和函数体，构造 `LambdaValue`。
- `defineForm`：实现 `define`。既支持 `(define x expr)`，也支持 `(define (f x) body...)` 的函数定义语法糖。
- `ifForm`：实现 `if`，根据条件结果只求值 then 或 else 分支。
- `andForm`：实现 `and`，从左到右求值，遇到假值立即返回。
- `orForm`：实现 `or`，从左到右求值，遇到真值立即返回。
- `condForm`：实现 `cond`，按顺序检查每个条件分支，支持 `else`。
- `beginForm`：实现 `begin`，顺序执行多个表达式。
- `letForm`：实现 `let`，先求值绑定表达式，再通过构造 lambda 的方式进入局部作用域。
- `quasiquoteWalk`：递归处理 quasiquote，遇到 `unquote` 时局部求值。
- `quasiquoteForm`：实现 `quasiquote`。
- `SPECIAL_FORMS`：特殊形式注册表，供 `EvalEnv` 在求值组合式时查询。

## 4. `graphics.cpp`

### 概览

`graphics.cpp` 实现 Windows 平台下的 2D 图形模块。它使用 Win32/GDI 创建窗口、维护后台缓冲区、处理窗口消息，并把绘图能力包装为 Lisp 内置过程。

该模块是黑白棋脚本能够运行的关键。Lisp 脚本通过 `graphics-open` 打开窗口，通过绘图函数绘制棋盘和棋子，通过事件函数读取鼠标点击，再通过 Lisp 代码更新游戏状态。

### 主要结构说明

- `Point`：窗口坐标点，保存 `x` 和 `y`。
- `Size`：二维尺寸，保存 `width` 和 `height`。
- `GraphicsEvent`：图形事件，包括事件名称和参数。
- `WindowState`：窗口生命周期状态，包括窗口句柄、是否创建完成、是否仍然打开。
- `BackBuffer`：双缓冲绘图资源，包括内存 DC、位图和当前绘图颜色。
- `GraphicsState`：图形模块全局状态，包括互斥锁、条件变量、事件队列、UI 线程、窗口状态和缓冲区。

### 主要函数说明

- `requireWindowOpen`：检查图形窗口是否已打开且缓冲区可用。
- `expectColor`：从 Lisp 参数中读取 RGB 颜色，并限制在 0 到 255 范围内。
- `expectPoint`：从 Lisp 参数中读取坐标点。
- `expectSize`：从 Lisp 参数中读取宽高。
- `pushEvent`：把窗口事件压入事件队列，并唤醒等待事件的 Lisp 线程。
- `popEventLocked`：在已持有锁的前提下弹出一个事件。
- `eventToValue`：把 C++ 图形事件转换为 Lisp 列表。
- `mouseButtonName`：把 Win32 鼠标消息转换为 Lisp 中的按钮名称。
- `windowProc`：Win32 窗口过程，处理绘制、关闭、鼠标、键盘等消息。
- `cleanupBackBuffer`：释放双缓冲相关 GDI 资源。
- `uiThreadMain`：图形窗口 UI 线程入口，负责注册窗口类、创建窗口、初始化缓冲区并运行消息循环。
- `invalidateWindow`：请求窗口重绘。
- `closeWindow`：关闭窗口并回收 UI 线程资源。
- `graphicsOpen`：实现 `graphics-open`，创建指定大小和标题的窗口。
- `graphicsClose`：实现 `graphics-close`，关闭当前窗口。
- `graphicsClear`：实现 `graphics-clear`，用指定颜色清空缓冲区。
- `graphicsColor`：实现 `graphics-color`，设置当前绘图颜色。
- `graphicsLine`：实现 `graphics-line`，绘制直线。
- `graphicsRect`：实现 `graphics-rect`，绘制矩形，可选择填充或描边。
- `graphicsCircle`：实现 `graphics-circle`，绘制圆形，可选择填充或描边。
- `graphicsText`：实现 `graphics-text`，在窗口中绘制文本。
- `graphicsRefresh`：实现 `graphics-refresh`，触发窗口把后台缓冲刷新到屏幕。
- `graphicsPollEvent`：实现 `graphics-poll-event`，非阻塞读取事件，没有事件时返回 `()`。
- `graphicsWaitEvent`：实现 `graphics-wait-event`，阻塞等待直到事件出现或窗口关闭。
- `graphicsSleep`：实现 `graphics-sleep`，暂停当前 Lisp 脚本一段时间。

## 5. `line_editor.cpp`

### 概览

`line_editor.cpp` 实现 REPL 的交互式行编辑器。相比简单的 `std::getline`，它支持光标移动、删除、Tab 缩进、语法高亮、多行输入辅助等功能。

该文件主要服务用户体验，使 Mini Lisp 的交互环境更接近一个可用的解释器终端。

### 主要函数说明

- `isIdentStart`：判断某个字符是否可以作为高亮扫描中的标识符起始字符。
- `isIdentChar`：判断某个字符是否可以继续作为标识符字符。
- `isKeyword`：判断字符串是否为需要高亮的 Lisp 关键字或内置过程名。
- `LineEditor::LineEditor`：构造行编辑器，并按配置初始化。
- `LineEditor::~LineEditor`：析构时恢复终端模式。
- `LineEditor::enableRawMode`：在 Windows 终端中启用原始输入模式和 ANSI 转义支持。
- `LineEditor::disableRawMode`：恢复进入 raw mode 前保存的终端模式。
- `LineEditor::readKey`：读取并解析一个按键，把平台相关按键转换为统一的 `KeyPress`。
- `LineEditor::highlight`：对当前输入文本做轻量语法高亮。
- `LineEditor::render`：重绘当前输入行，并把光标移动到正确位置。
- `LineEditor::countOpenParens`：统计未闭合左括号数量，忽略字符串和注释中的括号。
- `LineEditor::readLine`：行编辑器核心函数，循环读取按键、更新文本、移动光标、渲染界面，并在用户按 Enter 时返回输入。

## 6. `lisp_utils.cpp`

### 概览

`lisp_utils.cpp` 实现跨模块共享的运行时工具函数。内置过程、特殊形式和图形模块都会用到这些工具。

该文件的价值在于统一规则：参数数量错误怎么报、数字怎么取、字符串怎么取、假值怎么判断、列表怎么构造，都集中在这里，避免各文件重复实现。

### 主要函数说明

- `requireArgsSize`：要求参数数量必须等于指定值，否则抛出 `LispError`。
- `requireAtLeast`：要求参数数量不少于指定值。
- `expectNumber`：从 `ValuePtr` 中提取数字，否则抛出错误。
- `expectInt`：提取数字并转换为整数。
- `expectString`：从 `ValuePtr` 中提取字符串，否则抛出错误。
- `isFalseValue`：判断 Lisp 语义中的假值。当前只有 `#f` 为假。
- `isProperList`：判断一个值是否为正规列表。
- `equalValues`：递归比较两个 Lisp 值是否深度相等。
- `makeNil`：构造空列表。
- `makeBool`：构造布尔值。
- `makeNumber`：构造数字值。
- `makeSymbol`：构造符号值。
- `makeList`：从 C++ vector 构造 Lisp 列表。

## 7. `main.cpp`

### 概览

`main.cpp` 是程序入口。它根据命令行参数决定运行测试、执行脚本文件，或者进入交互式 REPL。

### 主要函数说明

- `TestCtx::eval`：测试框架使用的求值辅助函数，把输入字符串经过分词、解析、求值后返回字符串化结果。
- `evalFile`：执行一个脚本文件中的所有表达式。它会不断调用 Parser 解析表达式，并在同一个环境中求值。
- `main`：程序入口。支持三种模式：
  - `mini_lisp.exe --test`：运行内置测试。
  - `mini_lisp.exe file.scm`：执行指定脚本文件。
  - `mini_lisp.exe`：进入 REPL。

## 8. `parser.cpp`

### 概览

`parser.cpp` 实现递归下降解析器。它把 Tokenizer 产生的 token 流转换为解释器内部使用的 Value 树。

解析器不负责求值，只负责把外部文本结构变成内部数据结构。例如 `'x` 会被转换成 `(quote x)`，列表会转换成 `PairValue` 链。

### 主要函数说明

- `Parser::Parser`：接收 token 队列并初始化解析器。
- `Parser::peek`：查看当前 token，不消耗它；如果已经没有 token，则抛出语法错误。
- `Parser::take`：取出并消耗当前 token。
- `Parser::makeList`：把 quote 等简写语法包装成普通列表结构。
- `Parser::parseTails`：解析列表尾部，支持正规列表和点对。
- `Parser::parse`：解析一个完整表达式，处理数字、布尔值、字符串、标识符、列表、quote、quasiquote、unquote 等情况。
- `Parser::hasMore`：判断 token 流中是否还有未解析内容。

## 9. `repl.cpp`

### 概览

`repl.cpp` 实现交互式读取-求值-打印循环。它连接了 `LineEditor`、`Tokenizer`、`Parser` 和 `EvalEnv`，让用户可以在终端中直接输入 Lisp 表达式。

REPL 支持多行输入。当括号尚未闭合时，它会继续等待下一行，而不是立刻报错。

### 主要函数说明

- `runRepl`：REPL 主循环。它负责显示主提示符和续行提示符，读取输入，判断表达式是否完整，调用分词、解析和求值流程，并输出结果或错误。

## 10. `token.cpp`

### 概览

`token.cpp` 实现 token 的工厂函数和字符串化逻辑。它服务于分词器和调试输出。

### 主要函数说明

- `Token::fromChar`：根据单个字符创建括号、quote、quasiquote、unquote 等简单 token。
- `Token::dot`：创建点号 token，用于解析点对。
- `Token::toString`：把 token 转换为可读字符串。
- `BooleanLiteralToken::fromChar`：根据字符 `t` 或 `f` 创建布尔 token。
- `BooleanLiteralToken::toString`：输出 `#t` 或 `#f`。
- `NumericLiteralToken::toString`：输出数字 token 的文本形式。
- `StringLiteralToken::toString`：输出带引号的字符串 token。
- `IdentifierToken::toString`：输出标识符名称。
- `operator<<`：支持把 token 直接写入输出流。

## 11. `tokenizer.cpp`

### 概览

`tokenizer.cpp` 实现源码分词。它逐字符扫描输入字符串，把原始文本转换成 Parser 能够消费的 token 队列。

该文件处理注释、空白、字符串转义、布尔字面量、数字识别、标识符识别和非法字符错误。

### 主要函数说明

- `TOKEN_END`：标识符和数字候选文本的结束字符集合。
- `Tokenizer::Tokenizer`：保存待扫描源码。
- `Tokenizer::nextToken`：从当前位置开始读取下一个 token。它会跳过空白和注释，并识别各种 token 类型。
- `Tokenizer::tokenize`：不断调用 `nextToken`，直到输入结束，生成完整 token 队列。
- `Tokenizer::tokenize(const std::string&)`：静态快捷入口，用于直接把字符串转换成 token 队列。

## 12. `value.cpp`

### 概览

`value.cpp` 实现所有运行时值类型的具体行为。它是解释器数据模型的核心，包括类型判断、字符串化、列表转换和过程值调用。

### 主要函数说明

- `pairToVector`：把正规列表形式的 `PairValue` 链转换为 C++ vector；如果遇到点对尾部则抛出错误。
- `formatPairTail`：递归格式化列表尾部，保证正规列表和点对都能正确打印。
- `Value::isNil`、`Value::isSelfEvaluating`、`Value::isNumber`、`Value::isBoolean`、`Value::isString`、`Value::isPair`：基础类型查询默认实现。
- `Value::asNumber`、`Value::asSymbol`：默认返回空值，由对应子类覆盖。
- `Value::toVector`：默认抛出错误，只有列表相关类型覆盖。
- `Value::isProcedure`：默认不是过程。
- `BooleanValue` 相关函数：实现布尔值取值、类型判断、自求值判断和打印。
- `NumericValue` 相关函数：实现数字取值、类型判断、数字提取、自求值判断和打印。
- `StringValue` 相关函数：实现字符串取值、类型判断、自求值判断和带引号打印。
- `NilValue` 相关函数：实现空列表判断、转空 vector、打印为 `()`。
- `SymbolValue` 相关函数：实现符号名称读取、符号提取和打印。
- `PairValue` 相关函数：实现左右值读取、pair 判断、转 vector 和列表/点对打印。
- `BuiltinProcValue` 相关函数：包装 C++ 内置函数，并在调用时转发到函数指针。
- `LambdaValue` 相关函数：保存形参、函数体和闭包环境，并提供读取接口。
- `operator<<`：支持把任意 `Value` 直接写入输出流。

