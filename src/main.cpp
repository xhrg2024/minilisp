#include <string>

#include "./parser.h"
#include "./tokenizer.h"
#include "./rjsj_test.hpp"

struct TestCtx {
    std::string eval(std::string input) {
        auto tokens = Tokenizer::tokenize(input);
        Parser parser(std::move(tokens));
        auto value = parser.parse();
        return value->toString();
    }
};

int main() {
    RJSJ_TEST(TestCtx, Lv2, Lv2Only);
}