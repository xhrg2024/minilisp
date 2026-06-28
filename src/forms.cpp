#include "./forms.h"

#include <utility>

#include "./error.h"
#include "./eval_env.h"
#include "./lisp_utils.h"

namespace {

/*
 * 特殊形式辅助实现区。
 * 特殊形式拿到的是未求值表达式，因此这里集中处理真假判断、表达式序列求值
 * 和参数结构检查，避免把这些语法规则分散到各个表项中。
 */

using LispUtils::isFalseValue;
using LispUtils::makeBool;
using LispUtils::makeNil;

std::vector<ValuePtr> takeTail(const std::vector<ValuePtr>& args,
                               std::size_t start) {
    return std::vector<ValuePtr>(
        args.begin() + static_cast<std::ptrdiff_t>(start), args.end());
}

ValuePtr evalSequence(const std::vector<ValuePtr>& expressions, EvalEnv& env) {
    ValuePtr result = makeNil();
    for (const auto& expr : expressions) {
        result = env.eval(expr);
    }
    return result;
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
    return std::make_shared<LambdaValue>(std::move(params), std::move(body),
                                         env.shared_from_this());
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
        return makeNil();
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
    env.defineBinding(
        *name, std::make_shared<LambdaValue>(std::move(params), std::move(body),
                                             env.shared_from_this()));
    return makeNil();
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
        return makeBool(true);
    }
    ValuePtr last = makeBool(true);
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
        return makeBool(false);
    }
    for (const auto& expr : args) {
        auto value = env.eval(expr);
        if (!isFalseValue(value)) {
            return value;
        }
    }
    return makeBool(false);
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
        auto condition = isElse ? makeBool(true) : env.eval(test);
        if (!isFalseValue(condition)) {
            if (vec.size() == 1) {
                return condition;
            }
            return evalSequence(takeTail(vec, 1), env);
        }
    }
    return makeNil();
}

ValuePtr beginForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        return makeNil();
    }
    return evalSequence(args, env);
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
    auto lambda = std::make_shared<LambdaValue>(
        std::move(params), std::move(body), env.shared_from_this());
    return env.apply(lambda, std::move(paramValues));
}

namespace {

/*
 * quasiquote 递归展开辅助区。
 * 这里专门处理 quasiquote 内部遇到 unquote 时的局部求值，其余 pair 会递归
 * 保持数据结构形状。
 */

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
    return std::make_shared<PairValue>(quasiquoteWalk(car, env),
                                       quasiquoteWalk(pair->getRight(), env));
}

}  // namespace

ValuePtr quasiquoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1) {
        throw LispError("Malformed quasiquote.");
    }
    return quasiquoteWalk(args[0], env);
}

/*
 * 特殊形式名称到 C++ 实现函数的映射表。
 * EvalEnv 在普通过程调用前查询该表，以决定当前列表是否需要特殊求值规则。
 */
const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"quote", &quoteForm}, {"define", &defineForm},
    {"if", &ifForm},       {"and", &andForm},
    {"or", &orForm},       {"lambda", &lambdaForm},
    {"cond", &condForm},   {"begin", &beginForm},
    {"let", &letForm},     {"quasiquote", &quasiquoteForm},
};
