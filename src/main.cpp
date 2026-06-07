#include <string>

#include "./eval_env.h"
#include "./parser.h"
#include "./tokenizer.h"
#include "./rjsj_test.hpp"

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

int main() {
    RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv6);
}