#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <deque>
#include <string>

#include "./token.h"

/*
 * Mini-Lisp 源码字符扫描器。
 * Tokenizer 只负责把原始文本切分为注释、字符串、布尔值、数字、标识符和
 * 括号等 token，不处理求值语义，从而让分词、解析和执行各自保持单一职责。
 */
class Tokenizer {
private:
    TokenPtr nextToken(int& pos);
    std::deque<TokenPtr> tokenize();

    std::string input;
    Tokenizer(const std::string& input) : input{input} {}

public:
    static std::deque<TokenPtr> tokenize(const std::string& input);
};

#endif
