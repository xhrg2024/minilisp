#include "./forms.h"

#include <utility>

#include "./error.h"
#include "./eval_env.h"

namespace {

bool isFalseValue(const ValuePtr& value) {
    auto boolean = std::dynamic_pointer_cast<BooleanValue>(value);
    return boolean != nullptr && !boolean->getValue();
}

std::vector<ValuePtr> takeTail(const std::vector<ValuePtr>& args, std::size_t start) {
    return std::vector<ValuePtr>(args.begin() + static_cast<std::ptrdiff_t>(start), args.end());
}

}  // namespace

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv&) {
    if (args.size() != 1) {
        throw LispError("Malformed quote.");
    }
    return args.front();
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() < 2) {
        throw LispError("Malformed lambda.");
    }
    auto params = args.front()->toVector();
    auto body = takeTail(args, 1);
    return std::make_shared<LambdaValue>(std::move(params), std::move(body), env.shared_from_this());
}

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        throw LispError("Malformed define.");
    }

    if (auto name = args[0]->asSymbol()) {
        if (args.size() != 2) {
            throw LispError("Malformed define.");
        }
        env.defineBinding(*name, env.eval(args[1]));
        return std::make_shared<NilValue>();
    }

    if (args.size() < 2) {
        throw LispError("Malformed define.");
    }

    auto head = args[0]->toVector();
    if (head.empty()) {
        throw LispError("Malformed define.");
    }
    auto name = head.front()->asSymbol();
    if (!name) {
        throw LispError("Malformed define.");
    }

    std::vector<ValuePtr> params(head.begin() + 1, head.end());
    std::vector<ValuePtr> body(args.begin() + 1, args.end());
    env.defineBinding(*name, std::make_shared<LambdaValue>(std::move(params), std::move(body), env.shared_from_this()));
    return std::make_shared<NilValue>();
}

ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 3) {
        throw LispError("Malformed if.");
    }
    auto condition = env.eval(args[0]);
    if (isFalseValue(condition)) {
        return env.eval(args[2]);
    }
    return env.eval(args[1]);
}

ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        return std::make_shared<BooleanValue>(true);
    }
    ValuePtr last = std::make_shared<BooleanValue>(true);
    for (const auto& expr : args) {
        last = env.eval(expr);
        if (isFalseValue(last)) {
            return last;
        }
    }
    return last;
}

ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        return std::make_shared<BooleanValue>(false);
    }
    for (const auto& expr : args) {
        auto value = env.eval(expr);
        if (!isFalseValue(value)) {
            return value;
        }
    }
    return std::make_shared<BooleanValue>(false);
}

ValuePtr condForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for (const auto& clause : args) {
        auto vec = clause->toVector();
        if (vec.empty()) {
            throw LispError("Malformed cond clause.");
        }
        auto test = vec[0];
        bool isElse = false;
        if (auto sym = test->asSymbol()) {
            if (*sym == "else") {
                isElse = true;
            }
        }
        auto condition = isElse ? std::make_shared<BooleanValue>(true) : env.eval(test);
        if (!isFalseValue(condition)) {
            if (vec.size() == 1) {
                return condition;
            }
            ValuePtr result = std::make_shared<NilValue>();
            for (std::size_t i = 1; i < vec.size(); ++i) {
                result = env.eval(vec[i]);
            }
            return result;
        }
    }
    return std::make_shared<NilValue>();
}

ValuePtr beginForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        return std::make_shared<NilValue>();
    }
    ValuePtr result = std::make_shared<NilValue>();
    for (const auto& expr : args) {
        result = env.eval(expr);
    }
    return result;
}

ValuePtr letForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() < 2) {
        throw LispError("Malformed let.");
    }
    auto bindings = args[0]->toVector();
    std::vector<std::string> paramNames;
    std::vector<ValuePtr> paramValues;
    for (const auto& binding : bindings) {
        auto bv = binding->toVector();
        if (bv.size() != 2) {
            throw LispError("Malformed let binding.");
        }
        auto name = bv[0]->asSymbol();
        if (!name) {
            throw LispError("Malformed let binding: expected symbol.");
        }
        paramNames.push_back(*name);
        paramValues.push_back(env.eval(bv[1]));
    }
    std::vector<ValuePtr> params;
    for (const auto& name : paramNames) {
        params.push_back(std::make_shared<SymbolValue>(name));
    }
    std::vector<ValuePtr> body(args.begin() + 1, args.end());
    auto lambda = std::make_shared<LambdaValue>(std::move(params), std::move(body), env.shared_from_this());
    return env.apply(lambda, std::move(paramValues));
}

namespace {

ValuePtr quasiquoteWalk(const ValuePtr& expr, EvalEnv& env) {
    if (!expr->isPair()) {
        return expr;
    }
    auto pair = std::dynamic_pointer_cast<PairValue>(expr);
    auto car = pair->getLeft();
    if (auto sym = car->asSymbol()) {
        if (*sym == "unquote") {
            auto cdr = pair->getRight();
            auto vec = cdr->toVector();
            if (vec.size() != 1) {
                throw LispError("Malformed unquote.");
            }
            return env.eval(vec[0]);
        }
    }
    return std::make_shared<PairValue>(
        quasiquoteWalk(car, env),
        quasiquoteWalk(pair->getRight(), env)
    );
}

}  // namespace

ValuePtr quasiquoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1) {
        throw LispError("Malformed quasiquote.");
    }
    return quasiquoteWalk(args[0], env);
}


const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"quote", &quoteForm},
    {"define", &defineForm},
    {"if", &ifForm},
    {"and", &andForm},
    {"or", &orForm},
    {"lambda", &lambdaForm},
    {"cond", &condForm},
    {"begin", &beginForm},
    {"let", &letForm},
    {"quasiquote", &quasiquoteForm},
};