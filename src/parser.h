#ifndef PARSER_H
#define PARSER_H

#include <deque>

#include "./token.h"
#include "./value.h"

class Parser {
private:
    std::deque<TokenPtr> tokens;

    TokenPtr& peek();
    TokenPtr take();
    ValuePtr parseTails();
    ValuePtr makeList(const std::string& head, ValuePtr tail);

public:
    explicit Parser(std::deque<TokenPtr> tokens);

    ValuePtr parse();
};

#endif