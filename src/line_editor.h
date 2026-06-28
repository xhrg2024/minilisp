#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#include <optional>
#include <string>

/*
 * REPL 使用的交互式行编辑器。
 * 该类负责终端原始模式、按键编辑、语法高亮以及基于括号数量的续行输入；
 * 对外只暴露 readLine()，让 REPL 不需要关心底层控制台细节。
 */
class LineEditor {
public:
    /*
     * 行编辑器配置项。
     * 将语法高亮、自动缩进、制表宽度等开关集中在一个结构中，避免构造函数
     * 参数列表随着功能增长而变长。
     */
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

    /*
     * 从终端读取一条逻辑输入行。
     * indentHint 用于续行提示时预填缩进空格；返回 nullopt 表示 EOF，
     * 调用方可以据此正常结束 REPL。
     */
    std::optional<std::string> readLine(const std::string& prompt,
                                        int indentHint = 0);

    /*
     * 统计源码片段中尚未闭合的左括号数量。
     * 该函数会跳过字符串和注释中的括号，使结果反映 Lisp 语法结构，而不是
     * 单纯的字符计数。
     */
    static int countOpenParens(const std::string& text);

private:
    Config config_;
    bool useRawMode_;
    bool rawModeSaved_ = false;
    void* savedInputMode_ = nullptr;
    void* savedOutputMode_ = nullptr;

    void enableRawMode();
    void disableRawMode();

    /*
     * readKey() 返回的标准化按键类型。
     * 平台相关的方向键、控制键和转义序列会先转换为该枚举，让编辑逻辑不必
     * 直接依赖控制台 API。
     */
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

    /*
     * 解码后的单次按键。
     * 可打印字符通过 ch 保存字符本身，移动光标和编辑命令则只使用 key 字段。
     */
    struct KeyPress {
        Key key;
        char ch = '\0';
    };
    KeyPress readKey();

    /*
     * 重绘当前编辑行，并把光标移动到逻辑编辑位置。
     * 将渲染集中在这里，可以让输入处理只维护文本状态，而不重复书写终端
     * 转义序列。
     */
    void render(const std::string& prompt, const std::string& text,
                int cursorPos);

    /*
     * 对 Lisp 源码做轻量级语法高亮。
     * 返回值包含 ANSI 颜色转义序列，只用于终端显示，不参与解析或持久化。
     */
    std::string highlight(const std::string& text);
};

#endif  // LINE_EDITOR_H
