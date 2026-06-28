#include "./lisp_utils.h"

#include <cmath>
#include <string>

#include "./error.h"

/*
 * LispUtils 的具体实现。
 * 本文件承载跨模块共享的运行时规则，避免各个内置过程或特殊形式各自实现
 * 一套略有差异的参数检查、类型转换和构造逻辑。
 */
namespace LispUtils {

void requireArgsSize(const std::vector<ValuePtr>& args, std::size_t expected,
                     const char* name) {
    if (args.size() != expected) {
        throw LispError(std::string(name) + " expects " +
                        std::to_string(expected) + " argument(s).");
    }
}

void requireAtLeast(const std::vector<ValuePtr>& args, std::size_t expected,
                    const char* name) {
    if (args.size() < expected) {
        throw LispError(std::string(name) + " expects at least " +
                        std::to_string(expected) + " argument(s).");
    }
}

double expectNumber(const ValuePtr& value, const char* message) {
    auto number = value->asNumber();
    if (!number) {
        throw LispError(message);
    }
    return *number;
}

int expectInt(const ValuePtr& value, const char* message) {
    return static_cast<int>(std::lround(expectNumber(value, message)));
}

const std::string& expectString(const ValuePtr& value, const char* message) {
    auto string = std::dynamic_pointer_cast<const StringValue>(value);
    if (string == nullptr) {
        throw LispError(message);
    }
    return string->getValue();
}

bool isFalseValue(const ValuePtr& value) {
    auto boolean = std::dynamic_pointer_cast<BooleanValue>(value);
    return boolean != nullptr && !boolean->getValue();
}

bool isProperList(const ValuePtr& value) {
    if (value->isNil()) {
        return true;
    }
    auto pair = std::dynamic_pointer_cast<PairValue>(value);
    return pair != nullptr && isProperList(pair->getRight());
}

bool equalValues(const ValuePtr& lhs, const ValuePtr& rhs) {
    if (lhs->isNil() && rhs->isNil()) {
        return true;
    }
    if (lhs->isBoolean() && rhs->isBoolean()) {
        return dynamic_cast<const BooleanValue&>(*lhs).getValue() ==
               dynamic_cast<const BooleanValue&>(*rhs).getValue();
    }
    if (lhs->isNumber() && rhs->isNumber()) {
        return *lhs->asNumber() == *rhs->asNumber();
    }
    if (lhs->isString() && rhs->isString()) {
        return dynamic_cast<const StringValue&>(*lhs).getValue() ==
               dynamic_cast<const StringValue&>(*rhs).getValue();
    }
    if (lhs->asSymbol().has_value() && rhs->asSymbol().has_value()) {
        return *lhs->asSymbol() == *rhs->asSymbol();
    }
    if (lhs->isPair() && rhs->isPair()) {
        auto& lhsPair = dynamic_cast<const PairValue&>(*lhs);
        auto& rhsPair = dynamic_cast<const PairValue&>(*rhs);
        return equalValues(lhsPair.getLeft(), rhsPair.getLeft()) &&
               equalValues(lhsPair.getRight(), rhsPair.getRight());
    }
    return false;
}

ValuePtr makeNil() {
    return std::make_shared<NilValue>();
}

ValuePtr makeBool(bool value) {
    return std::make_shared<BooleanValue>(value);
}

ValuePtr makeNumber(double value) {
    return std::make_shared<NumericValue>(value);
}

ValuePtr makeSymbol(const std::string& name) {
    return std::make_shared<SymbolValue>(name);
}

ValuePtr makeList(const std::vector<ValuePtr>& values) {
    ValuePtr result = makeNil();
    for (auto iter = values.rbegin(); iter != values.rend(); ++iter) {
        result = std::make_shared<PairValue>(*iter, result);
    }
    return result;
}

}  // namespace LispUtils
