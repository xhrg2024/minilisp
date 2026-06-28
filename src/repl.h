#ifndef REPL_H
#define REPL_H

#include <memory>

class EvalEnv;

/*
 * 启动交互式读取-求值-打印循环。
 * 函数会持续读取完整表达式，在传入环境中求值，并打印非空结果，直到遇到
 * EOF 或 Lisp 程序显式退出。
 */
void runRepl(std::shared_ptr<EvalEnv> env);

#endif
