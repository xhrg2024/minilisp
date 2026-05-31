#include "./eval_env.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "./error.h"
#include "./forms.h"

EvalEnv::EvalEnv() {
    for (const auto& [name, func] : BUILTIN_FUNCTIONS) {
        symbols.emplace(name, std::make_shared<BuiltinProcValue>(func));
    }
}

void EvalEnv::addSymbol(const std::string& name, ValuePtr value) {
    symbols[name] = std::move(value);
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
        return builtin->call(args);
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
        auto iter = symbols.find(*name);
        if (iter != symbols.end()) {
            return iter->second;
        }
        throw LispError("Variable " + *name + " not defined.");
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