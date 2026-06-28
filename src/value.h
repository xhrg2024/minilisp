#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

class Value;
class EvalEnv;

using ValuePtr = std::shared_ptr<Value>;
using BuiltinFuncType = ValuePtr(const std::vector<ValuePtr>&, EvalEnv&);

class Value {
protected:
    Value() = default;

public:
    virtual ~Value() = default;
    virtual std::string toString() const = 0;
    virtual bool isNil() const;
    virtual bool isSelfEvaluating() const;
    virtual bool isNumber() const;
    virtual bool isBoolean() const;
    virtual bool isString() const;
    virtual bool isPair() const;
    virtual std::optional<std::string> asSymbol() const;
    virtual std::optional<double> asNumber() const;
    virtual std::vector<ValuePtr> toVector() const;
    virtual bool isProcedure() const;
};

class BooleanValue : public Value {
private:
    const bool value;

public:
    explicit BooleanValue(bool value);

    bool getValue() const;
    bool isBoolean() const override;
    bool isSelfEvaluating() const override;
    std::string toString() const override;
};

class NumericValue : public Value {
private:
    const double value;

public:
    explicit NumericValue(double value);

    double getValue() const;
    bool isNumber() const override;
    std::optional<double> asNumber() const override;
    bool isSelfEvaluating() const override;
    std::string toString() const override;
};

class StringValue : public Value {
private:
    const std::string value;

public:
    explicit StringValue(const std::string& value);

    const std::string& getValue() const;
    bool isString() const override;
    bool isSelfEvaluating() const override;
    std::string toString() const override;
};

class NilValue : public Value {
public:
    NilValue();

    bool isNil() const override;
    std::vector<ValuePtr> toVector() const override;

    std::string toString() const override;
};

class SymbolValue : public Value {
private:
    const std::string name;

public:
    explicit SymbolValue(const std::string& name);

    const std::string& getName() const;
    std::optional<std::string> asSymbol() const override;
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
    bool isPair() const override;
    std::vector<ValuePtr> toVector() const override;
    std::string toString() const override;
};

class BuiltinProcValue : public Value {
private:
    BuiltinFuncType* func;

public:
    explicit BuiltinProcValue(BuiltinFuncType* func);

    bool isSelfEvaluating() const override;
    bool isProcedure() const override;
    ValuePtr call(const std::vector<ValuePtr>& args, EvalEnv& env) const;
    std::string toString() const override;
};

class EvalEnv;

class LambdaValue : public Value {
private:
    std::vector<ValuePtr> params;
    std::vector<ValuePtr> body;
    std::shared_ptr<EvalEnv> parent;

public:
    LambdaValue(std::vector<ValuePtr> params, std::vector<ValuePtr> body,
                std::shared_ptr<EvalEnv> parent);

    bool isSelfEvaluating() const override;
    bool isProcedure() const override;
    const std::vector<ValuePtr>& getParams() const;
    const std::vector<ValuePtr>& getBody() const;
    std::shared_ptr<EvalEnv> getEnv() const;
    std::string toString() const override;
};

std::ostream& operator<<(std::ostream& os, const Value& value);

#endif