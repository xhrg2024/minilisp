#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

class Value;
class EvalEnv;

/*
 * 所有运行时值的共享句柄。
 * Value
 * 对象本身按不可变方式使用，列表、闭包和环境都可能引用同一个值，
 * 因此使用
 * shared_ptr 表达共享所有权。
 */
using ValuePtr = std::shared_ptr<Value>;

/*
 * Lisp 内置过程的 C++ 函数签名。
 *
 * 调用时参数已经完成求值，同时传入当前环境，使 apply、map、filter、eval
 *
 * 等高阶过程能够在必要时重新进入求值器。
 */
using BuiltinFuncType = ValuePtr(const std::vector<ValuePtr>&, EvalEnv&);

/*
 * 所有 Lisp 运行时值的抽象基类。
 *
 * 虚函数查询接口构成一套轻量类型协议，解析器、求值器、打印器和内置过程
 *
 * 可以通过该协议使用值，而不需要依赖具体派生类实现。

 */
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

/*
 * Scheme 布尔值的运行时表示。
 * 布尔值是自求值对象，并按 #t/#f
 * 打印；真假值规则不写死在该类中，而是由
 * 公共工具函数统一判断。

 */
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

/*
 * 数字字面量的运行时表示。
 * 当前解释器统一用 double 保存数值；integer?
 * 等谓词在需要更严格语义时
 * 再检查数字是否具有整数形态。

 */
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

/*
 * 字符串字面量的运行时表示。
 * 在当前解释器中字符串对 Lisp
 * 代码不可变，内部保存原始 C++ 字符串，
 * toString() 则输出带引号、可读的 Lisp
 * 表示。
 */
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

/*
 * 空列表的运行时表示。
 * Nil 既是一个普通值，也是正规列表的结束标记；按照
 * Scheme 风格，它不会被
 * 求值器当作假值处理。
 */
class NilValue : public Value {
public:
    NilValue();

    bool isNil() const override;
    std::vector<ValuePtr> toVector() const override;

    std::string toString() const override;
};

/*
 * 解析后标识符的运行时表示。
 * SymbolValue 在求值阶段会由 EvalEnv
 * 查找绑定；如果处于 quote 结果中，
 * 它则作为普通符号数据保留下来。

 */
class SymbolValue : public Value {
private:
    const std::string name;

public:
    explicit SymbolValue(const std::string& name);

    const std::string& getName() const;
    std::optional<std::string> asSymbol() const override;
    std::string toString() const override;
};

/*
 * cons 单元的运行时表示。
 * PairValue
 * 同时用于正规列表和点对；当调用者需要把它转成 vector 时，必须
 *
 * 确认尾部是正规列表，否则会抛出运行时错误。
 */
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

/*
 * C++ 原生内置过程的运行时包装。
 *
 * 它是自求值对象，并把调用委托给保存的函数指针，同时对求值器保持统一的
 * Value
 * 接口。
 */
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

/*
 * 用户自定义 lambda 的运行时表示。
 *
 * 它保存形参列表、函数体表达式和定义时所在的父环境，从而让闭包在之后调用
 *
 * 时仍能访问定义处的词法绑定。
 */
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
