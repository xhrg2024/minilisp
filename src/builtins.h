#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <unordered_map>

#include "./value.h"

/*
 * 普通内置过程注册表。
 * 表中的过程会被加入全局环境，调用时接收到的参数已经完成求值，因此它们
 * 与用户通过 lambda 定义的过程共享同一套 apply 调用路径。
 */
extern const std::unordered_map<std::string, BuiltinFuncType*>
    BUILTIN_FUNCTIONS;

#endif
