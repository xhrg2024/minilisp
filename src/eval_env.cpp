#include "./eval_env.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "./error.h"
#include "./forms.h"

EvalEnv::EvalEnv() : parent{nullptr} {}

std::shared_ptr<EvalEnv> EvalEnv::createGlobal() {
    auto env = std::shared_ptr<EvalEnv>(new EvalEnv());
    for (const auto& [name, func] : BUILTIN_FUNCTIONS) {
        env->symbols.emplace(name, std::make_shared<BuiltinProcValue>(func));
    }
    return env;
}

ValuePtr EvalEnv::lookupBinding(const std::string& name) const {
    auto iter = symbols.find(name);
    if (iter != symbols.end()) {
        return iter->second;
    }
    if (parent != nullptr) {
        return parent->lookupBinding(name);
    }
    throw LispError("Variable " + name + " not defined.");
}

void EvalEnv::defineBinding(const std::string& name, ValuePtr value) {
    symbols[name] = std::move(value);
}

std::shared_ptr<EvalEnv> EvalEnv::createChild(const std::vector<std::string>& params,
                                               const std::vector<ValuePtr>& args) {
    auto child = std::shared_ptr<EvalEnv>(new EvalEnv());
    child->parent = shared_from_this();
    if (params.size() != args.size()) {
        throw LispError("Lambda parameter count mismatch.");
    }
    for (std::size_t i = 0; i < params.size(); ++i) {
        child->symbols[params[i]] = args[i];
    }
    return child;
}

std::vector<ValuePtr> EvalEnv::evalList(ValuePtr expr) {
    std::vector<ValuePtr> result;
    std::ranges::transform(expr->toVector(), std::back_inserter(result), [this](ValuePtr value) {
        return this->eval(std::move(value));
    });
    return result;
}

ValuePtr EvalEnv::apply(ValuePtr proc, std::vector<ValuePtr> args) {
    if (auto builtin = dynamic_cast<BuiltinProcValue*>(proc.get()); builtin != nullptr) {
        return builtin->call(args, *this);
    }
    if (auto lambda = dynamic_cast<LambdaValue*>(proc.get()); lambda != nullptr) {
        std::vector<std::string> paramNames;
        for (const auto& param : lambda->getParams()) {
            auto sym = param->asSymbol();
            if (!sym) {
                throw LispError("Lambda parameter must be a symbol.");
            }
            paramNames.push_back(*sym);
        }
        auto child = lambda->getEnv()->createChild(paramNames, args);
        const auto& body = lambda->getBody();
        ValuePtr result = std::make_shared<NilValue>();
        for (const auto& expr : body) {
            result = child->eval(expr);
        }
        return result;
    }
    throw LispError("Unimplemented");
}

ValuePtr EvalEnv::eval(ValuePtr expr) {
    if (expr->isSelfEvaluating()) {
        return expr;
    }

    if (expr->isNil()) {
        throw LispError("Evaluating nil is prohibited.");
    }

    if (auto name = expr->asSymbol()) {
        return lookupBinding(*name);
    }

    auto values = expr->toVector();
    if (values.empty()) {
        throw LispError("Evaluating nil is prohibited.");
    }

    if (auto head = values.front()->asSymbol()) {
        auto iter = SPECIAL_FORMS.find(*head);
        if (iter != SPECIAL_FORMS.end()) {
            return iter->second(std::vector<ValuePtr>(values.begin() + 1, values.end()), *this);
        }
    }

    auto proc = eval(values.front());
    std::vector<ValuePtr> args;
    if (values.size() > 1) {
        std::ranges::transform(values.begin() + 1, values.end(), std::back_inserter(args), [this](ValuePtr value) {
            return this->eval(std::move(value));
        });
    }
    return apply(std::move(proc), std::move(args));
}
