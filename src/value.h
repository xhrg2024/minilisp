#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <ostream>
#include <string>

class Value {
protected:
    Value() = default;

public:
    virtual ~Value() = default;
    virtual std::string toString() const = 0;
};

using ValuePtr = std::shared_ptr<Value>;

class BooleanValue : public Value {
private:
    const bool value;

public:
    explicit BooleanValue(bool value);

    bool getValue() const;
    std::string toString() const override;
};

class NumericValue : public Value {
private:
    const double value;

public:
    explicit NumericValue(double value);

    double getValue() const;
    std::string toString() const override;
};

class StringValue : public Value {
private:
    const std::string value;

public:
    explicit StringValue(const std::string& value);

    const std::string& getValue() const;
    std::string toString() const override;
};

class NilValue : public Value {
public:
    NilValue();

    std::string toString() const override;
};

class SymbolValue : public Value {
private:
    const std::string name;

public:
    explicit SymbolValue(const std::string& name);

    const std::string& getName() const;
    std::string toString() const override;
};

class PairValue : public Value {
private:
    const std::shared_ptr<Value> left;
    const std::shared_ptr<Value> right;

public:
    PairValue(std::shared_ptr<Value> left, std::shared_ptr<Value> right);

    const std::shared_ptr<Value>& getLeft() const;
    const std::shared_ptr<Value>& getRight() const;
    std::string toString() const override;
};

std::ostream& operator<<(std::ostream& os, const Value& value);

#endif