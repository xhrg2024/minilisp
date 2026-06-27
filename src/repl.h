#ifndef REPL_H
#define REPL_H

#include <memory>

class EvalEnv;

void runRepl(std::shared_ptr<EvalEnv> env);

#endif