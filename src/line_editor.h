#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#include <optional>
#include <string>

// 交互式行编辑器：支持代码高亮、自动缩进、光标定位
// Windows 实现，使用 _getch() 和 ANSI 转义序列
class LineEditor {
public:
    struct Config {
        bool syntaxHighlight = true;
        bool autoIndent = true;
        int tabWidth = 2;
        Config() = default;
    };

    LineEditor();
    explicit LineEditor(Config config);
    ~LineEditor();

    LineEditor(const LineEditor&) = delete;
    LineEditor& operator=(const LineEditor&) = delete;

    // 读取一行输入。indentHint 预填充空格（用于续行模式下的自动缩进）。
    // 返回 nullopt 表示 EOF。
    std::optional<std::string> readLine(const std::string& prompt,
                                        int indentHint = 0);

    // 计算文本中的未闭合左括号数（用于自动缩进）
    static int countOpenParens(const std::string& text);

private:
    Config config_;
    bool useRawMode_;
    bool rawModeSaved_ = false;
    void* savedInputMode_ = nullptr;   // DWORD 原始模式
    void* savedOutputMode_ = nullptr;  // DWORD 原始模式

    void enableRawMode();
    void disableRawMode();

    // 按键类型
    enum class Key {
        Char,
        Enter,
        Backspace,
        Delete,
        Tab,
        ArrowLeft,
        ArrowRight,
        Home,
        End,
        CtrlC,
        CtrlD,
        Unknown
    };
    struct KeyPress {
        Key key;
        char ch = '\0';
    };
    KeyPress readKey();

    // 渲染当前行（含语法高亮和光标定位）
    void render(const std::string& prompt, const std::string& text,
                int cursorPos);

    // 语法高亮：返回带 ANSI 颜色码的字符串
    std::string highlight(const std::string& text);
};

#endif  // LINE_EDITOR_H
