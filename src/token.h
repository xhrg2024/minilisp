#ifndef TOKEN_H
#define TOKEN_H

#include <memory>
#include <optional>
#include <ostream>
#include <string>

/*
 * 词法分析器能够识别的完整 token 类型集合。
 * Parser 依赖这些类别而不是原始源码字符来做语法判断，从而将字符扫描与
 * 语法构造解耦。
 */
enum class TokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    QUOTE,
    QUASIQUOTE,
    UNQUOTE,
    DOT,
    BOOLEAN_LITERAL,
    NUMERIC_LITERAL,
    STRING_LITERAL,
    IDENTIFIER,
};

class Token;

/*
 * token 的独占所有权句柄。
 * token 会从分词器移动到解析器，使用 unique_ptr 可以明确表达所有权转移，
 * 并避免不同阶段意外共享可变解析状态。
 */
using TokenPtr = std::unique_ptr<Token>;

/*
 * token 基类。
 * 简单标点 token 直接使用该类，带值的字面量和标识符 token 通过派生类保存
 * 解析后的具体数据。
 */
class Token {
private:
    TokenType type;

protected:
    Token(TokenType type) : type{type} {}

public:
    virtual ~Token() = default;

    static TokenPtr fromChar(char c);
    static TokenPtr dot();

    TokenType getType() const {
        return type;
    }
    virtual std::string toString() const;
};

/*
 * 布尔字面量 token，对应 #t 和 #f。
 * 它把源码中的布尔标记转换为 C++ bool，解析阶段再据此构造 BooleanValue。
 */
class BooleanLiteralToken : public Token {
private:
    bool value;

public:
    BooleanLiteralToken(bool value)
        : Token(TokenType::BOOLEAN_LITERAL), value{value} {}

    static std::unique_ptr<BooleanLiteralToken> fromChar(char c);

    bool getValue() const {
        return value;
    }
    std::string toString() const override;
};

/*
 * 数字字面量 token。
 * 创建该对象前，分词器会确认整个词素都能解析为数字，避免出现 "12abc" 这类
 * 只被部分解析的非法数字。
 */
class NumericLiteralToken : public Token {
private:
    double value;

public:
    NumericLiteralToken(double value)
        : Token(TokenType::NUMERIC_LITERAL), value{value} {}

    double getValue() const {
        return value;
    }
    std::string toString() const override;
};

/*
 * 字符串字面量 token。
 * 转义字符会在分词阶段处理完成，因此这里保存的是实际字符串内容，而不是
 * 带引号的原始源码片段。
 */
class StringLiteralToken : public Token {
private:
    std::string value;

public:
    StringLiteralToken(const std::string& value)
        : Token(TokenType::STRING_LITERAL), value{value} {}

    const std::string& getValue() const {
        return value;
    }
    std::string toString() const override;
};

/*
 * 标识符 token，也用于承载特殊形式名称。
 * Parser 会先把标识符构造成 SymbolValue，之后由求值器决定它是环境中的绑定
 * 名称，还是 quote 后保留下来的普通数据。
 */
class IdentifierToken : public Token {
private:
    std::string name;

public:
    IdentifierToken(const std::string& name)
        : Token(TokenType::IDENTIFIER), name{name} {}

    const std::string& getName() const {
        return name;
    }
    std::string toString() const override;
};

std::ostream& operator<<(std::ostream& os, const Token& token);

#endif
