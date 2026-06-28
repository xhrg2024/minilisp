#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>

#include "./value.h"

class EvalEnv;

/*
 * 图形过程声明。
 * 这些函数会注册为 Lisp 内置过程，组成一个小型绘图与事件 API：脚本可以
 * 打开窗口、绘制到双缓冲、刷新画面，并以 Lisp 列表形式读取输入事件。
 */
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
