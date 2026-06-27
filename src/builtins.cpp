#include "./builtins.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "./error.h"
#include "./eval_env.h"

namespace {

void requireArgsSize(const std::vector<ValuePtr>& args, std::size_t expected, const char* name) {
    if (args.size() != expected) {
        throw LispError(std::string(name) + " expects " + std::to_string(expected) + " argument(s).");
    }
}

void requireAtLeast(const std::vector<ValuePtr>& args, std::size_t expected, const char* name) {
    if (args.size() < expected) {
        throw LispError(std::string(name) + " expects at least " + std::to_string(expected) + " argument(s).");
    }
}

double expectNumber(const ValuePtr& value, const char* message) {
    if (!value->isNumber()) {
        throw LispError(message);
    }
    return *value->asNumber();
}

bool isProperList(const ValuePtr& value) {
    if (value->isNil()) {
        return true;
    }
    auto pair = std::dynamic_pointer_cast<PairValue>(value);
    if (pair == nullptr) {
        return false;
    }
    return isProperList(pair->getRight());
}

ValuePtr makeBool(bool value) {
    return std::make_shared<BooleanValue>(value);
}

ValuePtr makeNil() {
    return std::make_shared<NilValue>();
}

ValuePtr makeListFromValues(const std::vector<ValuePtr>& values) {
    ValuePtr result = makeNil();
    for (auto iter = values.rbegin(); iter != values.rend(); ++iter) {
        result = std::make_shared<PairValue>(*iter, result);
    }
    return result;
}

}  // namespace

ValuePtr add(const std::vector<ValuePtr>& args, EvalEnv& env) {
    double result = 0.0;
    for (const auto& value : args) {
        result += expectNumber(value, "Cannot add a non-numeric value.");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr sub(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 1, "-");
    double result = expectNumber(args.front(), "Cannot subtract a non-numeric value.");
    if (args.size() == 1) {
        return std::make_shared<NumericValue>(-result);
    }
    for (std::size_t i = 1; i < args.size(); ++i) {
        result -= expectNumber(args[i], "Cannot subtract a non-numeric value.");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr mul(const std::vector<ValuePtr>& args, EvalEnv& env) {
    double result = 1.0;
    for (const auto& value : args) {
        result *= expectNumber(value, "Cannot multiply a non-numeric value.");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr div(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 1, "/");
    double result = expectNumber(args.front(), "Cannot divide a non-numeric value.");
    if (args.size() == 1) {
        return std::make_shared<NumericValue>(1.0 / result);
    }
    for (std::size_t i = 1; i < args.size(); ++i) {
        result /= expectNumber(args[i], "Cannot divide a non-numeric value.");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr absValue(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "abs");
    return std::make_shared<NumericValue>(std::abs(expectNumber(args.front(), "Cannot take abs of a non-numeric value.")));
}

ValuePtr expt(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "expt");
    return std::make_shared<NumericValue>(
        std::pow(expectNumber(args[0], "Cannot exponentiate a non-numeric value."),
                 expectNumber(args[1], "Cannot exponentiate a non-numeric value.")));
}

ValuePtr quotient(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "quotient");
    auto lhs = expectNumber(args[0], "Cannot quotient a non-numeric value.");
    auto rhs = expectNumber(args[1], "Cannot quotient a non-numeric value.");
    return std::make_shared<NumericValue>(std::trunc(lhs / rhs));
}

ValuePtr remainder(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "remainder");
    auto lhs = expectNumber(args[0], "Cannot remainder a non-numeric value.");
    auto rhs = expectNumber(args[1], "Cannot remainder a non-numeric value.");
    return std::make_shared<NumericValue>(std::fmod(lhs, rhs));
}
ValuePtr modulo(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "modulo");
    auto lhs = expectNumber(args[0], "Cannot modulo a non-numeric value.");
    auto rhs = expectNumber(args[1], "Cannot modulo a non-numeric value.");
    return std::make_shared<NumericValue>(lhs - rhs * std::floor(lhs / rhs));
}



ValuePtr print(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i != 0) {
            std::cout << ' ';
        }
        std::cout << args[i]->toString();
    }
    std::cout << std::endl;
    return makeNil();
}

ValuePtr display(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for (const auto& value : args) {
        std::cout << value->toString();
    }
    return makeNil();
}

ValuePtr newline(const std::vector<ValuePtr>&, EvalEnv& env) {
    std::cout << std::endl;
    return makeNil();
}
ValuePtr displayln(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "displayln");
    std::cout << args.front()->toString();
    std::cout << std::endl;
    return makeNil();
}

ValuePtr errorProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        throw LispError("error");
    }
    throw LispError(args.front()->toString());
}



ValuePtr exitProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    int code = 0;
    if (!args.empty() && args.front()->isNumber()) {
        code = static_cast<int>(*args.front()->asNumber());
    }
    std::exit(code);
    return makeNil();
}

ValuePtr atomp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "atom?");
    const auto& arg = args.front();
    return makeBool(arg->isBoolean() || arg->isNumber() || arg->isString() ||
                    arg->asSymbol().has_value() || arg->isNil());
}

ValuePtr booleanp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "boolean?");
    return makeBool(args.front()->isBoolean());
}

ValuePtr integerp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "integer?");
    if (!args.front()->isNumber()) {
        return makeBool(false);
    }
    auto value = *args.front()->asNumber();
    return makeBool(std::floor(value) == value);
}

ValuePtr listp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "list?");
    return makeBool(isProperList(args.front()));
}

ValuePtr numberp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "number?");
    return makeBool(args.front()->isNumber());
}

ValuePtr nullp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "null?");
    return makeBool(args.front()->isNil());
}

ValuePtr pairp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "pair?");
    return makeBool(args.front()->isPair());
}

ValuePtr procedurep(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "procedure?");
    return makeBool(args.front()->isProcedure());
}

ValuePtr stringp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "string?");
    return makeBool(args.front()->isString());
}

ValuePtr symbolp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "symbol?");
    return makeBool(args.front()->asSymbol().has_value());
}

ValuePtr car(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "car");
    auto pair = std::dynamic_pointer_cast<PairValue>(args.front());
    if (pair == nullptr) {
        throw LispError("car expects a pair.");
    }
    return pair->getLeft();
}

ValuePtr cdr(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "cdr");
    auto pair = std::dynamic_pointer_cast<PairValue>(args.front());
    if (pair == nullptr) {
        throw LispError("cdr expects a pair.");
    }
    return pair->getRight();
}

ValuePtr cons(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "cons");
    return std::make_shared<PairValue>(args[0], args[1]);
}

ValuePtr length(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "length");
    auto current = args.front();
    double count = 0.0;
    while (!current->isNil()) {
        auto pair = std::dynamic_pointer_cast<PairValue>(current);
        if (pair == nullptr) {
            throw LispError("length expects a proper list.");
        }
        ++count;
        current = pair->getRight();
    }
    return std::make_shared<NumericValue>(count);
}

ValuePtr list(const std::vector<ValuePtr>& args, EvalEnv& env) {
    return makeListFromValues(args);
}

ValuePtr append(const std::vector<ValuePtr>& args, EvalEnv& env) {
    std::vector<ValuePtr> result;
    for (const auto& listArg : args) {
        if (listArg->isNil()) continue;
        auto vec = listArg->toVector();
        result.insert(result.end(), vec.begin(), vec.end());
    }
    return makeListFromValues(result);
}

ValuePtr map(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "map");
    auto proc = args[0];
    auto vec = args[1]->toVector();
    std::vector<ValuePtr> results;
    for (const auto& elem : vec) {
        results.push_back(env.apply(proc, {elem}));
    }
    return makeListFromValues(results);
}

ValuePtr filter(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "filter");
    auto proc = args[0];
    auto vec = args[1]->toVector();
    std::vector<ValuePtr> results;
    for (const auto& elem : vec) {
        auto res = env.apply(proc, {elem});
        auto b = std::dynamic_pointer_cast<BooleanValue>(res);
        if (b == nullptr || b->getValue()) {
            results.push_back(elem);
        }
    }
    return makeListFromValues(results);
}

ValuePtr reduce(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "reduce");
    auto proc = args[0];
    auto vec = args[1]->toVector();
    if (vec.empty()) {
        throw LispError("reduce expects a non-empty list.");
    }
    auto result = vec[0];
    for (std::size_t i = 1; i < vec.size(); ++i) {
        result = env.apply(proc, {result, vec[i]});
    }
    return result;
}

ValuePtr applyProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "apply");
    auto proc = args[0];
    auto listVec = args[1]->toVector();
    return env.apply(proc, listVec);
}

ValuePtr evalProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "eval");
    return env.eval(args[0]);
}


ValuePtr equalNum(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 2, "=");
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (!args[i - 1]->isNumber() || !args[i]->isNumber() ||
            *args[i - 1]->asNumber() != *args[i]->asNumber()) {
            return makeBool(false);
        }
    }
    return makeBool(true);
}

ValuePtr less(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 2, "<");
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (!args[i - 1]->isNumber() || !args[i]->isNumber() ||
            *args[i - 1]->asNumber() >= *args[i]->asNumber()) {
            return makeBool(false);
        }
    }
    return makeBool(true);
}

ValuePtr greater(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 2, ">");
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (!args[i - 1]->isNumber() || !args[i]->isNumber() ||
            *args[i - 1]->asNumber() <= *args[i]->asNumber()) {
            return makeBool(false);
        }
    }
    return makeBool(true);
}

ValuePtr lessEqual(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 2, "<=");
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (!args[i - 1]->isNumber() || !args[i]->isNumber() ||
            *args[i - 1]->asNumber() > *args[i]->asNumber()) {
            return makeBool(false);
        }
    }
    return makeBool(true);
}

ValuePtr greaterEqual(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 2, ">=");
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (!args[i - 1]->isNumber() || !args[i]->isNumber() ||
            *args[i - 1]->asNumber() < *args[i]->asNumber()) {
            return makeBool(false);
        }
    }
    return makeBool(true);
}

ValuePtr evenp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "even?");
    if (!args.front()->isNumber()) {
        return makeBool(false);
    }
    return makeBool(static_cast<long long>(*args.front()->asNumber()) % 2 == 0);
}

ValuePtr oddp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "odd?");
    if (!args.front()->isNumber()) {
        return makeBool(false);
    }
    return makeBool(static_cast<long long>(*args.front()->asNumber()) % 2 != 0);
}

ValuePtr zerop(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "zero?");
    return makeBool(args.front()->isNumber() && *args.front()->asNumber() == 0.0);
}

namespace {

bool equalValues(const ValuePtr& a, const ValuePtr& b) {
    if (a->isNil() && b->isNil()) {
        return true;
    }
    if (a->isBoolean() && b->isBoolean()) {
        return dynamic_cast<const BooleanValue&>(*a).getValue() ==
               dynamic_cast<const BooleanValue&>(*b).getValue();
    }
    if (a->isNumber() && b->isNumber()) {
        return *a->asNumber() == *b->asNumber();
    }
    if (a->isString() && b->isString()) {
        return dynamic_cast<const StringValue&>(*a).getValue() ==
               dynamic_cast<const StringValue&>(*b).getValue();
    }
    if (a->asSymbol().has_value() && b->asSymbol().has_value()) {
        return *a->asSymbol() == *b->asSymbol();
    }
    if (a->isPair() && b->isPair()) {
        auto& pa = dynamic_cast<const PairValue&>(*a);
        auto& pb = dynamic_cast<const PairValue&>(*b);
        return equalValues(pa.getLeft(), pb.getLeft()) &&
               equalValues(pa.getRight(), pb.getRight());
    }
    return false;
}

}  // namespace

ValuePtr eqp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "eq?");
    const auto& a = args[0];
    const auto& b = args[1];
    if (a->isBoolean() || a->isNumber() || a->isNil() || a->asSymbol().has_value()) {
        return makeBool(equalValues(a, b));
    }
    return makeBool(a.get() == b.get());
}

ValuePtr equalp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "equal?");
    return makeBool(equalValues(args[0], args[1]));
}

ValuePtr notp(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "not");
    auto b = std::dynamic_pointer_cast<BooleanValue>(args.front());
    if (b != nullptr && !b->getValue()) {
        return makeBool(true);
    }
    return makeBool(false);
}


const std::unordered_map<std::string, BuiltinFuncType*> BUILTIN_FUNCTIONS{
    {"+", &add},
    {"-", &sub},
    {"*", &mul},
    {"/", &div},
    {"abs", &absValue},
    {"expt", &expt},
    {"quotient", &quotient},
    {"remainder", &remainder},
    {"modulo", &modulo},
    {"print", &print},
    {"display", &display},
    {"displayln", &displayln},

    {"newline", &newline},
    {"error", &errorProc},

    {"exit", &exitProc},
    {"atom?", &atomp},
    {"boolean?", &booleanp},
    {"integer?", &integerp},
    {"list?", &listp},
    {"number?", &numberp},
    {"null?", &nullp},
    {"pair?", &pairp},
    {"procedure?", &procedurep},
    {"string?", &stringp},
    {"symbol?", &symbolp},
    {"car", &car},
    {"cdr", &cdr},
    {"cons", &cons},
    {"length", &length},
    {"list", &list},
    {"append", &append},
    {"map", &map},
    {"filter", &filter},
    {"reduce", &reduce},
    {"apply", &applyProc},
    {"eval", &evalProc},
    {"=", &equalNum},
    {"<", &less},
    {">", &greater},
    {"<=", &lessEqual},
    {">=", &greaterEqual},
    {"even?", &evenp},
    {"odd?", &oddp},
    {"zero?", &zerop},
    {"eq?", &eqp},
    {"equal?", &equalp},
    {"not", &notp},

};