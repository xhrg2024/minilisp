#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "./eval_env.h"
#include "./parser.h"
#include "./repl.h"
#include "./rjsj_test.hpp"
#include "./tokenizer.h"

struct TestCtx {
    std::shared_ptr<EvalEnv> env = EvalEnv::createGlobal();

    std::string eval(std::string input) {
        auto tokens = Tokenizer::tokenize(input);
        Parser parser(std::move(tokens));
        auto value = parser.parse();
        auto result = env->eval(std::move(value));
        return result->toString();
    }
};

void evalFile(Parser& parser, std::shared_ptr<EvalEnv>& env) {
    while (parser.hasMore()) {
        auto expr = parser.parse();
        env->eval(std::move(expr));
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        if (std::strcmp(argv[1], "--test") == 0) {
            RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv5Extra, Lv6, Lv7, Lv7Lib,
                      Sicp);
            return 0;
        }
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file: " << argv[1] << std::endl;
            return 1;
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        auto tokens = Tokenizer::tokenize(content);
        Parser parser(std::move(tokens));
        auto env = EvalEnv::createGlobal();
        try {
            evalFile(parser, env);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }

    auto env = EvalEnv::createGlobal();
    runRepl(std::move(env));
}
