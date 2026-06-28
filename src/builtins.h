#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <unordered_map>

#include "./value.h"

extern const std::unordered_map<std::string, BuiltinFuncType*>
    BUILTIN_FUNCTIONS;

#endif
