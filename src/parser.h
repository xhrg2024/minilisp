#ifndef PARSER_H
#define PARSER_H

#include <deque>

#include "./token.h"
#include "./value.h"

/*
 * Mini-Lisp 表达式的递归下降解析器。
 * Parser 消耗 Tokenizer 生成的 token
 * 流，并构造用于后续求值的 Value 树，
 * 覆盖字面量、符号、普通列表、点对以及
 * quote 相关简写语法。
 */
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
    bool hasMore() const;
};

#endif
