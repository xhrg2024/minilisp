#include "./value.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

std::string valueToString(const Value& value);

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

BooleanValue::BooleanValue(bool value) : value{value} {}

bool BooleanValue::getValue() const {
    return value;
}

std::string BooleanValue::toString() const {
    return value ? "#t" : "#f";
}

NumericValue::NumericValue(double value) : value{value} {}

double NumericValue::getValue() const {
    return value;
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

std::string StringValue::toString() const {
    std::ostringstream ss;
    ss << std::quoted(value);
    return ss.str();
}

NilValue::NilValue() = default;

std::string NilValue::toString() const {
    return "()";
}

SymbolValue::SymbolValue(const std::string& name) : name{name} {}

const std::string& SymbolValue::getName() const {
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

std::string PairValue::toString() const {
    return valueToString(*this);
}

std::ostream& operator<<(std::ostream& os, const Value& value) {
    return os << value.toString();
}