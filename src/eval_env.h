#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "./builtins.h"
#include "./forms.h"
#include "./value.h"

class EvalEnv : public std::enable_shared_from_this<EvalEnv> {
private:
    std::unordered_map<std::string, ValuePtr> symbols;
    std::shared_ptr<EvalEnv> parent;

    std::vector<ValuePtr> evalList(ValuePtr expr);

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
