#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <unordered_map>

#include "./value.h"
class EvalEnv;


ValuePtr add(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr sub(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr mul(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr div(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr absValue(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr expt(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr quotient(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr remainder(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr modulo(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr sqrtProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr sinProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr cosProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr tanProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr asinProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr acosProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr atanProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr logProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr expProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr floorProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr ceilProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr roundProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr maxProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr minProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr gcdProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr lcmProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringAppendProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringLengthProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringRefProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr substringProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringEqProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringLessProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringGreaterProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr numToStringProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringToNumProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringUpcaseProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringDowncaseProc(const std::vector<ValuePtr>& args, EvalEnv& env);

ValuePtr print(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr display(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr newline(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr displayln(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr errorProc(const std::vector<ValuePtr>& args, EvalEnv& env);

ValuePtr exitProc(const std::vector<ValuePtr>& args, EvalEnv& env);

ValuePtr atomp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr booleanp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr integerp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr listp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr numberp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr nullp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr pairp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr procedurep(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr stringp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr symbolp(const std::vector<ValuePtr>& args, EvalEnv& env);

ValuePtr car(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr cdr(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr cons(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr length(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr list(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr append(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr map(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr filter(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr reduce(const std::vector<ValuePtr>& args, EvalEnv& env);

ValuePtr applyProc(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr evalProc(const std::vector<ValuePtr>& args, EvalEnv& env);


ValuePtr equalNum(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr less(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr greater(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr lessEqual(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr greaterEqual(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr evenp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr oddp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr zerop(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr eqp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr equalp(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr notp(const std::vector<ValuePtr>& args, EvalEnv& env);


extern const std::unordered_map<std::string, BuiltinFuncType*> BUILTIN_FUNCTIONS;

#endif