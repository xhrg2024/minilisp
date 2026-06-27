#include "./repl.h"

#include <iostream>
#include <string>

#include "./eval_env.h"
#include "./parser.h"
#include "./tokenizer.h"

void runRepl(std::shared_ptr<EvalEnv> env) {
    std::string line;
    const char* prompt = "mini-lisp> ";
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        try {
            auto tokens = Tokenizer::tokenize(line);
            Parser parser(std::move(tokens));
            if (!parser.hasMore()) {
                continue;
            }
            auto expr = parser.parse();
            auto result = env->eval(std::move(expr));
            std::cout << result->toString() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}