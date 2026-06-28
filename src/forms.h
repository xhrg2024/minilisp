#ifndef FORMS_H
#define FORMS_H

#include <string>
#include <unordered_map>
#include <vector>

#include "./value.h"

class EvalEnv;

/*
 * 特殊形式的 C++ 实现函数签名。
 * 与普通内置过程不同，特殊形式接收到的参数仍是未求值的语法树，因此 if、
 * define、lambda、quote 等形式可以自行控制求值时机与求值顺序。
 */
using SpecialFormType = ValuePtr(const std::vector<ValuePtr>&, EvalEnv&);

/*
 * 特殊形式注册表。
 * EvalEnv 在普通函数调用前先查询该表，从而把语法级求值规则与普通内置过程
 * 区分开，避免把非标准求值顺序混入通用调用逻辑。
 */
extern const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS;

#endif
