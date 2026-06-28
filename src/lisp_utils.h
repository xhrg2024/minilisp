#ifndef LISP_UTILS_H
#define LISP_UTILS_H

#include <cstddef>
#include <string>
#include <vector>

#include "./value.h"

namespace LispUtils {

void requireArgsSize(const std::vector<ValuePtr>& args, std::size_t expected,
                     const char* name);
void requireAtLeast(const std::vector<ValuePtr>& args, std::size_t expected,
                    const char* name);

double expectNumber(const ValuePtr& value, const char* message);
int expectInt(const ValuePtr& value, const char* message);
const std::string& expectString(const ValuePtr& value, const char* message);

bool isFalseValue(const ValuePtr& value);
bool isProperList(const ValuePtr& value);
bool equalValues(const ValuePtr& lhs, const ValuePtr& rhs);

ValuePtr makeNil();
ValuePtr makeBool(bool value);
ValuePtr makeNumber(double value);
ValuePtr makeSymbol(const std::string& name);
ValuePtr makeList(const std::vector<ValuePtr>& values);

}  // namespace LispUtils

#endif
