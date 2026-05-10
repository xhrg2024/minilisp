#include "./parser.h"

#include <utility>

#include "./error.h"

Parser::Parser(std::deque<TokenPtr> tokens) : tokens{std::move(tokens)} {}

TokenPtr& Parser::peek() {
    if (tokens.empty()) {
        throw SyntaxError("Unexpected end of input");
    }
    return tokens.front();
}

TokenPtr Parser::take() {
    auto token = std::move(peek());
    tokens.pop_front();
    return token;
}

ValuePtr Parser::makeList(const std::string& head, ValuePtr tail) {
    return std::make_shared<PairValue>(
        std::make_shared<SymbolValue>(head),
        std::make_shared<PairValue>(std::move(tail), std::make_shared<NilValue>())
    );
}

ValuePtr Parser::parseTails() {
    if (peek()->getType() == TokenType::RIGHT_PAREN) {
        take();
        return std::make_shared<NilValue>();
    }

    auto car = parse();
    if (peek()->getType() == TokenType::DOT) {
        take();
        auto cdr = parse();
        if (peek()->getType() != TokenType::RIGHT_PAREN) {
            throw SyntaxError("Expected ')'");
        }
        take();
        return std::make_shared<PairValue>(std::move(car), std::move(cdr));
    }

    auto cdr = parseTails();
    return std::make_shared<PairValue>(std::move(car), std::move(cdr));
}

ValuePtr Parser::parse() {
    auto token = take();
    switch (token->getType()) {
        case TokenType::NUMERIC_LITERAL:
            return std::make_shared<NumericValue>(static_cast<NumericLiteralToken&>(*token).getValue());
        case TokenType::BOOLEAN_LITERAL:
            return std::make_shared<BooleanValue>(static_cast<BooleanLiteralToken&>(*token).getValue());
        case TokenType::STRING_LITERAL:
            return std::make_shared<StringValue>(static_cast<StringLiteralToken&>(*token).getValue());
        case TokenType::IDENTIFIER:
            return std::make_shared<SymbolValue>(static_cast<IdentifierToken&>(*token).getName());
        case TokenType::LEFT_PAREN:
            return parseTails();
        case TokenType::QUOTE:
            return makeList("quote", parse());
        case TokenType::QUASIQUOTE:
            return makeList("quasiquote", parse());
        case TokenType::UNQUOTE:
            return makeList("unquote", parse());
        default:
            throw SyntaxError("Unimplemented");
    }
}