#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <string>
#include <unordered_map>
#include <vector>

#include "./builtins.h"
#include "./value.h"

class EvalEnv {
private:
    std::unordered_map<std::string, ValuePtr> symbols;

    std::vector<ValuePtr> evalList(ValuePtr expr);

public:
    EvalEnv();

    ValuePtr eval(ValuePtr expr);
    ValuePtr apply(ValuePtr proc, std::vector<ValuePtr> args);
};

#endif