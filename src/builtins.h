#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <unordered_map>

#include "./value.h"

ValuePtr add(const std::vector<ValuePtr>& args);
ValuePtr sub(const std::vector<ValuePtr>& args);
ValuePtr mul(const std::vector<ValuePtr>& args);
ValuePtr div(const std::vector<ValuePtr>& args);
ValuePtr absValue(const std::vector<ValuePtr>& args);
ValuePtr expt(const std::vector<ValuePtr>& args);
ValuePtr quotient(const std::vector<ValuePtr>& args);
ValuePtr remainder(const std::vector<ValuePtr>& args);
ValuePtr print(const std::vector<ValuePtr>& args);
ValuePtr display(const std::vector<ValuePtr>& args);
ValuePtr newline(const std::vector<ValuePtr>& args);
ValuePtr exitProc(const std::vector<ValuePtr>& args);

ValuePtr atomp(const std::vector<ValuePtr>& args);
ValuePtr booleanp(const std::vector<ValuePtr>& args);
ValuePtr integerp(const std::vector<ValuePtr>& args);
ValuePtr listp(const std::vector<ValuePtr>& args);
ValuePtr numberp(const std::vector<ValuePtr>& args);
ValuePtr nullp(const std::vector<ValuePtr>& args);
ValuePtr pairp(const std::vector<ValuePtr>& args);
ValuePtr procedurep(const std::vector<ValuePtr>& args);
ValuePtr stringp(const std::vector<ValuePtr>& args);
ValuePtr symbolp(const std::vector<ValuePtr>& args);

ValuePtr car(const std::vector<ValuePtr>& args);
ValuePtr cdr(const std::vector<ValuePtr>& args);
ValuePtr cons(const std::vector<ValuePtr>& args);
ValuePtr length(const std::vector<ValuePtr>& args);
ValuePtr list(const std::vector<ValuePtr>& args);

ValuePtr equal(const std::vector<ValuePtr>& args);
ValuePtr less(const std::vector<ValuePtr>& args);
ValuePtr greater(const std::vector<ValuePtr>& args);
ValuePtr lessEqual(const std::vector<ValuePtr>& args);
ValuePtr greaterEqual(const std::vector<ValuePtr>& args);
ValuePtr evenp(const std::vector<ValuePtr>& args);
ValuePtr oddp(const std::vector<ValuePtr>& args);
ValuePtr zerop(const std::vector<ValuePtr>& args);

extern const std::unordered_map<std::string, BuiltinFuncType*> BUILTIN_FUNCTIONS;

#endif