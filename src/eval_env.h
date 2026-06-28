#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "./builtins.h"
#include "./forms.h"
#include "./value.h"

/*
 * 词法环境与求值器入口。
 * 每个 EvalEnv
 * 表示一层符号绑定帧，并可指向父环境；表达式求值、函数应用
 *
 * 和闭包调用都依赖这条词法作用域链，因此统一封装在该类中。

 */
class EvalEnv : public std::enable_shared_from_this<EvalEnv> {
private:
    std::unordered_map<std::string, ValuePtr> symbols;
    std::shared_ptr<EvalEnv> parent;

    std::vector<ValuePtr> evalArguments(
        const std::vector<ValuePtr>& expressions);
    ValuePtr evalSequence(const std::vector<ValuePtr>& expressions);
    ValuePtr evalCombination(const std::vector<ValuePtr>& values);
    ValuePtr applyLambda(LambdaValue& lambda,
                         const std::vector<ValuePtr>& args);

    EvalEnv();

public:
    static std::shared_ptr<EvalEnv> createGlobal();

    ValuePtr lookupBinding(const std::string& name) const;
    void defineBinding(const std::string& name, ValuePtr value);

    std::shared_ptr<EvalEnv> createChild(const std::vector<std::string>& params,
                                         const std::vector<ValuePtr>& args);

    ValuePtr eval(ValuePtr expr);
    ValuePtr apply(ValuePtr proc, std::vector<ValuePtr> args);
};

#endif
