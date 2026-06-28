#include "./builtins.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>

#include "./error.h"
#include "./eval_env.h"
#include "./graphics.h"

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

ValuePtr sqrtProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "sqrt");
    return std::make_shared<NumericValue>(std::sqrt(expectNumber(args[0], "sqrt expects a number.")));
}

ValuePtr sinProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "sin");
    return std::make_shared<NumericValue>(std::sin(expectNumber(args[0], "sin expects a number.")));
}

ValuePtr cosProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "cos");
    return std::make_shared<NumericValue>(std::cos(expectNumber(args[0], "cos expects a number.")));
}

ValuePtr tanProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "tan");
    return std::make_shared<NumericValue>(std::tan(expectNumber(args[0], "tan expects a number.")));
}

ValuePtr asinProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "asin");
    return std::make_shared<NumericValue>(std::asin(expectNumber(args[0], "asin expects a number.")));
}

ValuePtr acosProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "acos");
    return std::make_shared<NumericValue>(std::acos(expectNumber(args[0], "acos expects a number.")));
}

ValuePtr atanProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 1, "atan");
    if (args.size() == 2)
        return std::make_shared<NumericValue>(std::atan2(
            expectNumber(args[0], "atan expects numbers."),
            expectNumber(args[1], "atan expects numbers.")));
    return std::make_shared<NumericValue>(std::atan(expectNumber(args[0], "atan expects a number.")));
}

ValuePtr logProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 1, "log");
    if (args.size() == 2)
        return std::make_shared<NumericValue>(
            std::log(expectNumber(args[1], "log expects numbers.")) /
            std::log(expectNumber(args[0], "log expects numbers.")));
    return std::make_shared<NumericValue>(std::log(expectNumber(args[0], "log expects a number.")));
}

ValuePtr expProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "exp");
    return std::make_shared<NumericValue>(std::exp(expectNumber(args[0], "exp expects a number.")));
}

ValuePtr floorProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "floor");
    return std::make_shared<NumericValue>(std::floor(expectNumber(args[0], "floor expects a number.")));
}

ValuePtr ceilProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "ceil");
    return std::make_shared<NumericValue>(std::ceil(expectNumber(args[0], "ceil expects a number.")));
}

ValuePtr roundProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "round");
    return std::make_shared<NumericValue>(std::round(expectNumber(args[0], "round expects a number.")));
}

ValuePtr maxProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 1, "max");
    double result = expectNumber(args[0], "max expects numbers.");
    for (std::size_t i = 1; i < args.size(); ++i)
        result = std::max(result, expectNumber(args[i], "max expects numbers."));
    return std::make_shared<NumericValue>(result);
}

ValuePtr minProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireAtLeast(args, 1, "min");
    double result = expectNumber(args[0], "min expects numbers.");
    for (std::size_t i = 1; i < args.size(); ++i)
        result = std::min(result, expectNumber(args[i], "min expects numbers."));
    return std::make_shared<NumericValue>(result);
}

ValuePtr gcdProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "gcd");
    int a = static_cast<int>(expectNumber(args[0], "gcd expects integers."));
    int b = static_cast<int>(expectNumber(args[1], "gcd expects integers."));
    return std::make_shared<NumericValue>(static_cast<double>(std::gcd(a, b)));
}

ValuePtr lcmProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "lcm");
    int a = static_cast<int>(expectNumber(args[0], "lcm expects integers."));
    int b = static_cast<int>(expectNumber(args[1], "lcm expects integers."));
    return std::make_shared<NumericValue>(static_cast<double>(std::lcm(a, b)));
}

namespace {

const std::string& expectString(const ValuePtr& value, const char* message) {
    auto str = std::dynamic_pointer_cast<const StringValue>(value);
    if (!str) throw LispError(message);
    return str->getValue();
}

}  // namespace

ValuePtr stringAppendProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    std::string result;
    for (const auto& arg : args)
        result += expectString(arg, "string-append expects string arguments.");
    return std::make_shared<StringValue>(result);
}

ValuePtr stringLengthProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "string-length");
    return std::make_shared<NumericValue>(
        static_cast<double>(expectString(args[0], "string-length expects a string.").size()));
}

ValuePtr stringRefProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "string-ref");
    const auto& s = expectString(args[0], "string-ref expects a string.");
    auto idx = static_cast<std::size_t>(expectNumber(args[1], "string-ref expects an index."));
    if (idx >= s.size()) throw LispError("string-ref: index out of range.");
    return std::make_shared<StringValue>(std::string(1, s[idx]));
}

ValuePtr substringProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 3, "substring");
    const auto& s = expectString(args[0], "substring expects a string.");
    auto start = static_cast<std::size_t>(expectNumber(args[1], "substring expects a start index."));
    auto end = static_cast<std::size_t>(expectNumber(args[2], "substring expects an end index."));
    if (start > s.size() || end > s.size() || start > end)
        throw LispError("substring: index out of range.");
    return std::make_shared<StringValue>(s.substr(start, end - start));
}

ValuePtr stringEqProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "string=?");
    return makeBool(expectString(args[0], "string=? expects strings.") ==
                    expectString(args[1], "string=? expects strings."));
}

ValuePtr stringLessProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "string<?");
    return makeBool(expectString(args[0], "string<? expects strings.") <
                    expectString(args[1], "string<? expects strings."));
}

ValuePtr stringGreaterProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 2, "string>?");
    return makeBool(expectString(args[0], "string>? expects strings.") >
                    expectString(args[1], "string>? expects strings."));
}

ValuePtr numToStringProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "number->string");
    double val = expectNumber(args[0], "number->string expects a number.");
    std::string s = std::to_string(val);
    // 去掉多余尾部零
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') s.pop_back();
    return std::make_shared<StringValue>(s);
}

ValuePtr stringToNumProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "string->number");
    const auto& s = expectString(args[0], "string->number expects a string.");
    try {
        std::size_t pos = 0;
        double val = std::stod(s, &pos);
        if (pos != s.size()) return makeBool(false);
        return std::make_shared<NumericValue>(val);
    } catch (...) {
        return makeBool(false);
    }
}

ValuePtr stringUpcaseProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "string-upcase");
    const auto& s = expectString(args[0], "string-upcase expects a string.");
    std::string result = s;
    for (auto& c : result)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return std::make_shared<StringValue>(result);
}

ValuePtr stringDowncaseProc(const std::vector<ValuePtr>& args, EvalEnv& env) {
    requireArgsSize(args, 1, "string-downcase");
    const auto& s = expectString(args[0], "string-downcase expects a string.");
    std::string result = s;
    for (auto& c : result)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return std::make_shared<StringValue>(result);
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
    {"graphics-open", &graphicsOpen},
    {"graphics-close", &graphicsClose},
    {"graphics-clear", &graphicsClear},
    {"graphics-color", &graphicsColor},
    {"graphics-line", &graphicsLine},
    {"graphics-rect", &graphicsRect},
    {"graphics-circle", &graphicsCircle},
    {"graphics-text", &graphicsText},
    {"graphics-refresh", &graphicsRefresh},
    {"graphics-poll-event", &graphicsPollEvent},
    {"graphics-wait-event", &graphicsWaitEvent},
    {"graphics-sleep", &graphicsSleep},

    {"+", &add},
    {"-", &sub},
    {"*", &mul},
    {"/", &div},
    {"abs", &absValue},
    {"expt", &expt},
    {"quotient", &quotient},
    {"remainder", &remainder},
    {"modulo", &modulo},
    {"sqrt", &sqrtProc},
    {"sin", &sinProc},
    {"cos", &cosProc},
    {"tan", &tanProc},
    {"asin", &asinProc},
    {"acos", &acosProc},
    {"atan", &atanProc},
    {"log", &logProc},
    {"exp", &expProc},
    {"floor", &floorProc},
    {"ceil", &ceilProc},
    {"round", &roundProc},
    {"max", &maxProc},
    {"min", &minProc},
    {"gcd", &gcdProc},
    {"lcm", &lcmProc},
    {"string-append", &stringAppendProc},
    {"string-length", &stringLengthProc},
    {"string-ref", &stringRefProc},
    {"substring", &substringProc},
    {"string=?", &stringEqProc},
    {"string<?", &stringLessProc},
    {"string>?", &stringGreaterProc},
    {"number->string", &numToStringProc},
    {"string->number", &stringToNumProc},
    {"string-upcase", &stringUpcaseProc},
    {"string-downcase", &stringDowncaseProc},
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
