#include "./line_editor.h"

#include <Windows.h>
#include <conio.h>
#include <io.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <set>
#include <string>
#include <vector>

// ============================================================================
// ANSI 颜色常量
// ============================================================================
/*
 * REPL 语法高亮使用的 ANSI 颜色常量。
 * 把显示样式集中在此处，后续调整终端主题时不需要改动按键处理或词法判断。
 */
namespace Ansi {

constexpr const char* RESET = "\033[0m";
constexpr const char* BOLD = "\033[1m";
constexpr const char* DIM = "\033[2m";

constexpr const char* RED = "\033[31m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* MAGENTA = "\033[35m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* WHITE = "\033[37m";
constexpr const char* GRAY = "\033[90m";

constexpr const char* BRIGHT_RED = "\033[91m";
constexpr const char* BRIGHT_GREEN = "\033[92m";
constexpr const char* BRIGHT_YELLOW = "\033[93m";
constexpr const char* BRIGHT_BLUE = "\033[94m";
constexpr const char* BRIGHT_MAGENTA = "\033[95m";
constexpr const char* BRIGHT_CYAN = "\033[96m";
constexpr const char* BRIGHT_WHITE = "\033[97m";

// 高亮颜色方案
/*
 * 高亮颜色方案。
 * 这些别名把具体颜色和语法类别对应起来，让 highlight() 只关注 token 类型。
 */
constexpr const char* PROMPT_COLOR = BRIGHT_CYAN;
constexpr const char* PAREN_COLOR = BRIGHT_WHITE;
constexpr const char* QUOTE_COLOR = YELLOW;
constexpr const char* STRING_COLOR = BRIGHT_GREEN;
constexpr const char* NUMBER_COLOR = BRIGHT_YELLOW;
constexpr const char* BOOLEAN_COLOR = MAGENTA;
constexpr const char* KEYWORD_COLOR = BRIGHT_MAGENTA;
constexpr const char* COMMENT_COLOR = GRAY;
constexpr const char* DEFAULT_COLOR = RESET;
}  // namespace Ansi

// ============================================================================
// Lisp 关键字集合（special forms + builtins）
// ============================================================================
/*
 * 行编辑器用于高亮的轻量词法元数据。
 * 这里只服务终端显示，真正的语法合法性仍由 Tokenizer 和 Parser 负责。
 */
namespace {

const std::set<std::string> LISP_KEYWORDS = {
    "define",
    "lambda",
    "if",
    "cond",
    "let",
    "begin",
    "quote",
    "quasiquote",
    "unquote",
    "and",
    "or",
    "set!",
    "+",
    "-",
    "*",
    "/",
    "abs",
    "expt",
    "quotient",
    "remainder",
    "modulo",
    "sqrt",
    "sin",
    "cos",
    "tan",
    "asin",
    "acos",
    "atan",
    "log",
    "exp",
    "floor",
    "ceil",
    "round",
    "max",
    "min",
    "gcd",
    "lcm",
    "string-append",
    "string-length",
    "string-ref",
    "substring",
    "string=?",
    "string<?",
    "string>?",
    "number->string",
    "string->number",
    "string-upcase",
    "string-downcase",
    "print",
    "display",
    "displayln",
    "newline",
    "error",
    "exit",
    "atom?",
    "boolean?",
    "integer?",
    "list?",
    "number?",
    "null?",
    "pair?",
    "procedure?",
    "string?",
    "symbol?",
    "car",
    "cdr",
    "cons",
    "length",
    "list",
    "append",
    "map",
    "filter",
    "reduce",
    "apply",
    "eval",
    "=",
    "<",
    ">",
    "<=",
    ">=",
    "even?",
    "odd?",
    "zero?",
    "eq?",
    "equal?",
    "not",
    "graphics-open",
    "graphics-close",
    "graphics-clear",
    "graphics-color",
    "graphics-line",
    "graphics-rect",
    "graphics-circle",
    "graphics-text",
    "graphics-refresh",
    "graphics-poll-event",
    "graphics-wait-event",
    "graphics-sleep",
};

/*
 * 高亮扫描时会结束标识符的字符集合。
 * 该集合与 Tokenizer 的分词边界保持接近，但只用于视觉展示，不影响执行。
 */
const std::set<char> TOKEN_END_CHARS{'(', ')', '\'', '`', ',', '"'};

bool isIdentStart(char c) {
    return !std::isspace(static_cast<unsigned char>(c)) &&
           !TOKEN_END_CHARS.contains(c) && c != ';' && c != '#' && c != '.';
}

bool isIdentChar(char c) {
    return isIdentStart(c);
}
bool isKeyword(const std::string& s) {
    return LISP_KEYWORDS.contains(s);
}
}  // namespace

// ============================================================================
// LineEditor 构造与析构
// ============================================================================

LineEditor::LineEditor() : LineEditor(Config{}) {}

LineEditor::LineEditor(Config config)
    : config_(std::move(config)),
      useRawMode_(_isatty(_fileno(stdin)) && _isatty(_fileno(stdout))) {}

LineEditor::~LineEditor() {
    if (rawModeSaved_) {
        disableRawMode();
    }
}

// ============================================================================
// 原始终端模式控制
// ============================================================================

void LineEditor::enableRawMode() {
    if (!useRawMode_ || rawModeSaved_) return;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            savedOutputMode_ =
                reinterpret_cast<void*>(static_cast<uintptr_t>(dwMode));
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hIn, &dwMode)) {
            savedInputMode_ =
                reinterpret_cast<void*>(static_cast<uintptr_t>(dwMode));
            DWORD newMode = dwMode;
            newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
            newMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
            newMode |= ENABLE_WINDOW_INPUT;
            SetConsoleMode(hIn, newMode);
            rawModeSaved_ = true;
        }
    }
}

void LineEditor::disableRawMode() {
    if (!rawModeSaved_) return;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE && savedOutputMode_) {
        DWORD dwMode =
            static_cast<DWORD>(reinterpret_cast<uintptr_t>(savedOutputMode_));
        SetConsoleMode(hOut, dwMode);
        savedOutputMode_ = nullptr;
    }

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE && savedInputMode_) {
        DWORD dwMode =
            static_cast<DWORD>(reinterpret_cast<uintptr_t>(savedInputMode_));
        SetConsoleMode(hIn, dwMode);
        savedInputMode_ = nullptr;
    }

    rawModeSaved_ = false;
}

// ============================================================================
// 按键读取
// ============================================================================

LineEditor::KeyPress LineEditor::readKey() {
    if (!useRawMode_) {
        int ch = std::cin.get();
        if (ch == EOF) return {Key::CtrlD, '\0'};
        if (ch == '\n' || ch == '\r') return {Key::Enter, '\0'};
        return {Key::Char, static_cast<char>(ch)};
    }

    int ch = _getch();

    if (ch == 3) return {Key::CtrlC, '\0'};
    if (ch == 4 || ch == 26) return {Key::CtrlD, '\0'};
    if (ch == '\r' || ch == '\n') return {Key::Enter, '\0'};
    if (ch == 8 || ch == 127) return {Key::Backspace, '\0'};
    if (ch == '\t') return {Key::Tab, '\0'};

    // 扩展键：0xE0 或 0x00 前缀
    if (ch == 0xE0 || ch == 0x00) {
        int ext = _getch();
        switch (ext) {
            case 75: return {Key::ArrowLeft, '\0'};
            case 77: return {Key::ArrowRight, '\0'};
            case 71: return {Key::Home, '\0'};
            case 79: return {Key::End, '\0'};
            case 83: return {Key::Delete, '\0'};
            default: return {Key::Unknown, '\0'};
        }
    }

    // 转义序列 VT100
    if (ch == 27 && _kbhit()) {
        int next = _getch();
        if (next == '[' && _kbhit()) {
            int dir = _getch();
            switch (dir) {
                case 'D': return {Key::ArrowLeft, '\0'};
                case 'C': return {Key::ArrowRight, '\0'};
                case 'H': return {Key::Home, '\0'};
                case 'F': return {Key::End, '\0'};
                case '3':
                    if (_kbhit() && _getch() == '~') return {Key::Delete, '\0'};
                    break;
                default: break;
            }
        }
        return {Key::Unknown, '\0'};
    }

    if (ch >= 32 && ch < 127) return {Key::Char, static_cast<char>(ch)};

    return {Key::Unknown, '\0'};
}

// ============================================================================
// 语法高亮器
// ============================================================================

std::string LineEditor::highlight(const std::string& text) {
    if (!config_.syntaxHighlight) return text;

    std::string result;
    result.reserve(text.size() * 2);
    std::size_t i = 0;

    while (i < text.size()) {
        char c = text[i];

        // 注释
        if (c == ';') {
            result += Ansi::COMMENT_COLOR;
            while (i < text.size()) result += text[i++];
            result += Ansi::RESET;
            break;
        }

        // 字符串
        if (c == '"') {
            result += Ansi::STRING_COLOR;
            result += '"';
            i++;
            while (i < text.size()) {
                if (text[i] == '"') {
                    result += '"';
                    i++;
                    break;
                }
                if (text[i] == '\\' && i + 1 < text.size()) {
                    result += text[i++];
                    result += text[i++];
                } else {
                    result += text[i++];
                }
            }
            result += Ansi::RESET;
            continue;
        }

        // 括号
        if (c == '(' || c == ')') {
            result += Ansi::PAREN_COLOR;
            result += Ansi::BOLD;
            result += c;
            result += Ansi::RESET;
            i++;
            continue;
        }

        // 引号语法字符
        if (c == '\'' || c == '`' || c == ',') {
            result += Ansi::QUOTE_COLOR;
            result += c;
            result += Ansi::RESET;
            i++;
            continue;
        }

        // 布尔字面量 #t #f
        if (c == '#' && i + 1 < text.size() &&
            (text[i + 1] == 't' || text[i + 1] == 'f')) {
            result += Ansi::BOOLEAN_COLOR;
            result += text.substr(i, 2);
            result += Ansi::RESET;
            i += 2;
            continue;
        }

        // 数字（含负数）
        if (std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '-' && i + 1 < text.size() &&
             std::isdigit(static_cast<unsigned char>(text[i + 1])))) {
            std::size_t start = i;
            if (c == '-' || c == '+') i++;
            while (i < text.size() &&
                   (std::isdigit(static_cast<unsigned char>(text[i])) ||
                    text[i] == '.')) {
                i++;
            }
            result += Ansi::NUMBER_COLOR;
            result += text.substr(start, i - start);
            result += Ansi::RESET;
            continue;
        }

        // 标识符 / 关键字
        if (isIdentStart(c)) {
            std::size_t start = i;
            while (i < text.size() && isIdentChar(text[i])) i++;
            std::string ident = text.substr(start, i - start);
            if (isKeyword(ident)) {
                result += Ansi::KEYWORD_COLOR;
                result += ident;
                result += Ansi::RESET;
            } else {
                result += ident;
            }
            continue;
        }

        result += c;
        i++;
    }

    return result;
}

// ============================================================================
// 屏幕渲染
// ============================================================================

void LineEditor::render(const std::string& prompt, const std::string& text,
                        int cursorPos) {
    std::cout << "\r\033[0K";

    if (config_.syntaxHighlight) {
        std::cout << Ansi::PROMPT_COLOR << prompt << Ansi::RESET;
    } else {
        std::cout << prompt;
    }

    std::cout << highlight(text);

    if (cursorPos < static_cast<int>(text.size())) {
        int targetCol = static_cast<int>(prompt.size()) + cursorPos + 1;
        std::cout << "\033[" << targetCol << "G";
    }

    std::cout.flush();
}

// ============================================================================
// 括号计数 — 用于自动缩进
// ============================================================================

int LineEditor::countOpenParens(const std::string& text) {
    int count = 0;
    bool inString = false;
    bool inComment = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (inComment) {
            if (c == '\n') inComment = false;
            continue;
        }
        if (inString) {
            if (c == '"') inString = false;
            if (c == '\\') i++;
            continue;
        }
        if (c == ';') {
            inComment = true;
            continue;
        }
        if (c == '"') {
            inString = true;
            continue;
        }
        if (c == '(') count++;
        if (c == ')') count--;
    }
    return count > 0 ? count : 0;
}

// ============================================================================
// 主循环：readLine
// ============================================================================

std::optional<std::string> LineEditor::readLine(const std::string& prompt,
                                                int indentHint) {
    // 非交互模式（管道/重定向）：回退到标准逐行读取
    if (!useRawMode_) {
        std::cout << prompt;
        if (indentHint > 0) std::cout << std::string(indentHint, ' ');
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) return std::nullopt;
        // 去掉 pre-filled indent 后的实际输入
        std::string result = line;
        return result;
    }

    enableRawMode();

    std::string text;
    int cursor = 0;

    if (indentHint > 0) {
        text.assign(indentHint, ' ');
        cursor = indentHint;
    }

    render(prompt, text, cursor);

    while (true) {
        KeyPress kp = readKey();

        switch (kp.key) {
            case Key::Enter: {
                std::cout << "\r\n";
                disableRawMode();
                return text;
            }

            case Key::CtrlD:
            case Key::CtrlC: {
                if (text.empty()) {
                    std::cout << "\r\n";
                    disableRawMode();
                    return std::nullopt;
                }
                if (cursor < static_cast<int>(text.size())) {
                    text.erase(cursor, 1);
                    render(prompt, text, cursor);
                }
                break;
            }

            case Key::Backspace: {
                if (cursor > 0) {
                    text.erase(cursor - 1, 1);
                    cursor--;
                    render(prompt, text, cursor);
                }
                break;
            }

            case Key::Delete: {
                if (cursor < static_cast<int>(text.size())) {
                    text.erase(cursor, 1);
                    render(prompt, text, cursor);
                }
                break;
            }

            case Key::ArrowLeft: {
                if (cursor > 0) {
                    cursor--;
                    render(prompt, text, cursor);
                }
                break;
            }

            case Key::ArrowRight: {
                if (cursor < static_cast<int>(text.size())) {
                    cursor++;
                    render(prompt, text, cursor);
                }
                break;
            }

            case Key::Home: {
                cursor = 0;
                render(prompt, text, cursor);
                break;
            }

            case Key::End: {
                cursor = static_cast<int>(text.size());
                render(prompt, text, cursor);
                break;
            }

            case Key::Tab: {
                int spaces = config_.tabWidth;
                text.insert(cursor, spaces, ' ');
                cursor += spaces;
                render(prompt, text, cursor);
                break;
            }

            case Key::Char: {
                text.insert(text.begin() + cursor, kp.ch);
                cursor++;

                // 括号匹配：输入 ) 时短暂闪烁匹配的 (
                if (kp.ch == ')' && useRawMode_ && config_.syntaxHighlight) {
                    int depth = 0;
                    int matchPos = -1;
                    for (int pos = cursor - 2; pos >= 0; --pos) {
                        if (text[pos] == ')')
                            depth++;
                        else if (text[pos] == '(') {
                            if (depth == 0) {
                                matchPos = pos;
                                break;
                            }
                            depth--;
                        }
                    }
                    if (matchPos >= 0) {
                        render(prompt, text, cursor);
                        int targetCol =
                            static_cast<int>(prompt.size()) + matchPos + 1;
                        std::cout << "\033[" << targetCol << "G";
                        std::cout.flush();
                        Sleep(150);
                        targetCol =
                            static_cast<int>(prompt.size()) + cursor + 1;
                        std::cout << "\033[" << targetCol << "G";
                        std::cout.flush();
                        break;
                    }
                }

                render(prompt, text, cursor);
                break;
            }

            default: break;
        }
    }
}
