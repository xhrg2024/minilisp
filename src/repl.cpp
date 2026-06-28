#include "./repl.h"

#include <cctype>
#include <iostream>
#include <string>

#include "./error.h"
#include "./eval_env.h"
#include "./line_editor.h"
#include "./parser.h"
#include "./tokenizer.h"

/*
 * REPL 交互循环实现。
 * 这里负责把多行输入拼接成完整表达式，调用分词、解析和求值流程，并把错误
 * 信息以适合交互使用的形式输出。
 */
void runRepl(std::shared_ptr<EvalEnv> env) {
    LineEditor editor;
    std::string input;
    const char* promptMain = "mini-lisp> ";
    const char* promptCont = "... ";

    while (true) {
        // 根据上下文选择提示符和缩进
        const char* prompt = input.empty() ? promptMain : promptCont;
        int indentHint = 0;
        if (!input.empty() && editor.countOpenParens(input) > 0) {
            // 自动缩进 = 未闭合括号数 × 2 空格
            indentHint = editor.countOpenParens(input) * 2;
        }

        auto optLine = editor.readLine(prompt, indentHint);
        if (!optLine) break;  // EOF

        std::string line = std::move(*optLine);

        // 将新行追加到累积缓冲区
        if (!input.empty()) {
            input += '\n';
        }
        input += line;

        // 跳过纯空白输入
        bool onlyWhitespace = true;
        for (char c : input) {
            if (!std::isspace(static_cast<unsigned char>(c))) {
                onlyWhitespace = false;
                break;
            }
        }
        if (onlyWhitespace) {
            input.clear();
            continue;
        }

        try {
            auto tokens = Tokenizer::tokenize(input);
            Parser parser(std::move(tokens));

            if (!parser.hasMore()) {
                input.clear();
                continue;
            }

            // 依次求值所有表达式
            while (parser.hasMore()) {
                auto expr = parser.parse();
                auto result = env->eval(std::move(expr));
                std::cout << result->toString() << std::endl;
            }

            input.clear();

        } catch (const SyntaxError& e) {
            std::string errMsg = e.what();
            if (errMsg.find("Unexpected end of") != std::string::npos) {
                // 输入不完整，缓冲区保持不变，进入续行模式
            } else {
                std::cout << "Error: " << errMsg << std::endl;
                input.clear();
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
            input.clear();
        }
    }
}
