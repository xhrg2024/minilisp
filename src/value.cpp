#include "./value.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "./error.h"
#include "./eval_env.h"

namespace {

std::string valueToString(const Value& value);

std::vector<ValuePtr> pairToVector(const PairValue& value) {
    std::vector<ValuePtr> result{value.getLeft()};
    auto tail = value.getRight();
    if (auto pair = dynamic_cast<const PairValue*>(tail.get()); pair != nullptr) {
        auto more = pairToVector(*pair);
        result.insert(result.end(), more.begin(), more.end());
    } else if (dynamic_cast<const NilValue*>(tail.get()) == nullptr) {
        throw LispError("Expected a proper list.");
    }
    return result;
}

std::string formatPairTail(const Value& value) {
    if (dynamic_cast<const NilValue*>(&value) != nullptr) {
        return ")";
    }

    if (auto pair = dynamic_cast<const PairValue*>(&value); pair != nullptr) {
        return " " + valueToString(*pair->getLeft()) + formatPairTail(*pair->getRight());
    }

    return " . " + valueToString(value) + ")";
}

std::string valueToString(const Value& value) {
    if (auto booleanValue = dynamic_cast<const BooleanValue*>(&value); booleanValue != nullptr) {
        return booleanValue->toString();
    }
    if (auto numericValue = dynamic_cast<const NumericValue*>(&value); numericValue != nullptr) {
        return numericValue->toString();
    }
    if (auto stringValue = dynamic_cast<const StringValue*>(&value); stringValue != nullptr) {
        return stringValue->toString();
    }
    if (auto symbolValue = dynamic_cast<const SymbolValue*>(&value); symbolValue != nullptr) {
        return symbolValue->toString();
    }
    if (auto nilValue = dynamic_cast<const NilValue*>(&value); nilValue != nullptr) {
        return nilValue->toString();
    }
    if (auto pairValue = dynamic_cast<const PairValue*>(&value); pairValue != nullptr) {
        std::ostringstream ss;
        ss << '(' << valueToString(*pairValue->getLeft()) << formatPairTail(*pairValue->getRight());
        return ss.str();
    }
    return "<unknown>";
}

}  // namespace

bool Value::isNil() const {
    return false;
}

bool Value::isSelfEvaluating() const {
    return false;
}

bool Value::isNumber() const {
    return false;
}

bool Value::isBoolean() const {
    return false;
}

bool Value::isString() const {
    return false;
}

bool Value::isPair() const {
    return false;
}

std::optional<double> Value::asNumber() const {
    return std::nullopt;
}

std::optional<std::string> Value::asSymbol() const {
    return std::nullopt;
}

std::vector<ValuePtr> Value::toVector() const {
    throw LispError("Expected a list.");
}

bool Value::isProcedure() const {
    return false;
}

BooleanValue::BooleanValue(bool value) : value{value} {}

bool BooleanValue::getValue() const {
    return value;
}

bool BooleanValue::isBoolean() const {
    return true;
}

bool BooleanValue::isSelfEvaluating() const {
    return true;
}

std::string BooleanValue::toString() const {
    return value ? "#t" : "#f";
}

NumericValue::NumericValue(double value) : value{value} {}

double NumericValue::getValue() const {
    return value;
}

bool NumericValue::isNumber() const {
    return true;
}

std::optional<double> NumericValue::asNumber() const {
    return value;
}

bool NumericValue::isSelfEvaluating() const {
    return true;
}

std::string NumericValue::toString() const {
    std::ostringstream ss;
    if (std::floor(value) == value) {
        ss << static_cast<long long>(value);
    } else {
        ss << std::setprecision(15) << value;
    }
    return ss.str();
}

StringValue::StringValue(const std::string& value) : value{value} {}

const std::string& StringValue::getValue() const {
    return value;
}

bool StringValue::isString() const {
    return true;
}

bool StringValue::isSelfEvaluating() const {
    return true;
}

std::string StringValue::toString() const {
    std::ostringstream ss;
    ss << std::quoted(value);
    return ss.str();
}

NilValue::NilValue() = default;

bool NilValue::isNil() const {
    return true;
}

std::string NilValue::toString() const {
    return "()";
}

SymbolValue::SymbolValue(const std::string& name) : name{name} {}

const std::string& SymbolValue::getName() const {
    return name;
}

std::optional<std::string> SymbolValue::asSymbol() const {
    return name;
}

std::string SymbolValue::toString() const {
    return name;
}

PairValue::PairValue(std::shared_ptr<Value> left, std::shared_ptr<Value> right)
    : left{std::move(left)}, right{std::move(right)} {}

const std::shared_ptr<Value>& PairValue::getLeft() const {
    return left;
}

const std::shared_ptr<Value>& PairValue::getRight() const {
    return right;
}

bool PairValue::isPair() const {
    return true;
}

std::vector<ValuePtr> PairValue::toVector() const {
    return pairToVector(*this);
}

std::string PairValue::toString() const {
    return valueToString(*this);
}

BuiltinProcValue::BuiltinProcValue(BuiltinFuncType* func) : func{func} {}

bool BuiltinProcValue::isSelfEvaluating() const {
    return true;
}

bool BuiltinProcValue::isProcedure() const {
    return true;
}

ValuePtr BuiltinProcValue::call(const std::vector<ValuePtr>& args) const {
    return (*func)(args);
}

std::string BuiltinProcValue::toString() const {
    return "#<procedure>";
}

LambdaValue::LambdaValue(std::vector<ValuePtr> params, std::vector<ValuePtr> body,
                         std::shared_ptr<EvalEnv> parent)
    : params{std::move(params)}, body{std::move(body)}, parent{std::move(parent)} {}

bool LambdaValue::isSelfEvaluating() const {
    return true;
}

bool LambdaValue::isProcedure() const {
    return true;
}

const std::vector<ValuePtr>& LambdaValue::getParams() const {
    return params;
}

const std::vector<ValuePtr>& LambdaValue::getBody() const {
    return body;
}

std::shared_ptr<EvalEnv> LambdaValue::getEnv() const {
    return parent;
}

std::string LambdaValue::toString() const {
    return "#<procedure>";
}

std::ostream& operator<<(std::ostream& os, const Value& value) {
    return os << value.toString();
}