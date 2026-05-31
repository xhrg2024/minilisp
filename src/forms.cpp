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

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv&) {
    if (args.size() < 2) {
        throw LispError("Malformed lambda.");
    }
    auto params = args.front()->toVector();
    auto body = takeTail(args, 1);
    return std::make_shared<LambdaValue>(std::move(params), std::move(body));
}

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        throw LispError("Malformed define.");
    }

    if (auto name = args[0]->asSymbol()) {
        if (args.size() != 2) {
            throw LispError("Malformed define.");
        }
        env.addSymbol(*name, env.eval(args[1]));
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
    env.addSymbol(*name, std::make_shared<LambdaValue>(std::move(params), std::move(body)));
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

const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"quote", &quoteForm},
    {"define", &defineForm},
    {"if", &ifForm},
    {"and", &andForm},
    {"or", &orForm},
    {"lambda", &lambdaForm},
};