#include "./token.h"

#include <iomanip>
#include <sstream>

/*
 * token 对象的打印与工厂函数实现。
 * 本文件把单字符语法符号、布尔字面量以及各类 token 的调试输出集中处理，
 * 让 Tokenizer 只负责扫描流程。
 */

/*
 * 字符串字面量后缀工具。
 * 使用标准库字符串字面量后缀可以让 token 打印代码更简洁，作用范围仅限于
 * 本翻译单元。
 */
using namespace std::literals;

TokenPtr Token::fromChar(char c) {
    TokenType type;
    switch (c) {
        case '(': type = TokenType::LEFT_PAREN; break;
        case ')': type = TokenType::RIGHT_PAREN; break;
        case '\'': type = TokenType::QUOTE; break;
        case '`': type = TokenType::QUASIQUOTE; break;
        case ',': type = TokenType::UNQUOTE; break;
        // DOT not listed here, because it can be part of identifier/literal.
        default: return nullptr;
    }
    return TokenPtr(new Token(type));
}

TokenPtr Token::dot() {
    return TokenPtr(new Token(TokenType::DOT));
}

std::string Token::toString() const {
    switch (type) {
        case TokenType::LEFT_PAREN: return "(LEFT_PAREN)"; break;
        case TokenType::RIGHT_PAREN: return "(RIGHT_PAREN)"; break;
        case TokenType::QUOTE: return "(QUOTE)"; break;
        case TokenType::QUASIQUOTE: return "(QUASIQUOTE)"; break;
        case TokenType::UNQUOTE: return "(UNQUOTE)"; break;
        case TokenType::DOT: return "(DOT)"; break;
        default: return "(UNKNOWN)";
    }
}

std::unique_ptr<BooleanLiteralToken> BooleanLiteralToken::fromChar(char c) {
    if (c == 't') {
        return std::make_unique<BooleanLiteralToken>(true);
    } else if (c == 'f') {
        return std::make_unique<BooleanLiteralToken>(false);
    } else {
        return nullptr;
    }
}

std::string BooleanLiteralToken::toString() const {
    return "(BOOLEAN_LITERAL "s + (value ? "true" : "false") + ")";
}

std::string NumericLiteralToken::toString() const {
    return "(NUMERIC_LITERAL " + std::to_string(value) + ")";
}

std::string StringLiteralToken::toString() const {
    std::ostringstream ss;
    ss << "(STRING_LITERAL " << std::quoted(value) << ")";
    return ss.str();
}

std::string IdentifierToken::toString() const {
    return "(IDENTIFIER " + name + ")";
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << token.toString();
}
