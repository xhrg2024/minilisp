#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

/*
 * 语法错误异常。
 * 当输入源码无法被正确分词、解析，或无法构造成合法表达式树时抛出该异常。
 */
class SyntaxError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/*
 * Lisp 运行时错误异常。
 * 当程序语法合法，但在求值、类型检查、参数数量检查或内置过程执行时失败，
 * 会通过该异常向 REPL 或文件执行入口报告错误。
 */
class LispError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

#endif
