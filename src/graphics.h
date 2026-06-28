#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>

#include "./value.h"

class EvalEnv;

ValuePtr graphicsOpen(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsClose(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsClear(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsColor(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsLine(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsRect(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsCircle(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsText(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsRefresh(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsPollEvent(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsWaitEvent(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr graphicsSleep(const std::vector<ValuePtr>& args, EvalEnv& env);

#endif
