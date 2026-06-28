#include "./tokenizer.h"

#include <cctype>
#include <set>
#include <stdexcept>

#include "./error.h"

const std::set<char> TOKEN_END{'(', ')', '\'', '`', ',', '"'};

TokenPtr Tokenizer::nextToken(int& pos) {
    while (pos < input.size()) {
        auto c = input[pos];
        if (c == ';') {
            while (pos < input.size() && input[pos] != '\n') {
                pos++;
            }
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            pos++;
        } else if (auto token = Token::fromChar(c)) {
            pos++;
            return token;
        } else if (c == '#') {
            if (pos + 1 >= input.size()) {
                throw SyntaxError("Unexpected end after #");
            }
            if (auto result = BooleanLiteralToken::fromChar(input[pos + 1])) {
                pos += 2;
                return result;
            } else {
                throw SyntaxError("Unexpected character after #");
            }
        } else if (c == '"') {
            std::string string;
            pos++;
            while (pos < input.size()) {
                if (input[pos] == '"') {
                    pos++;
                    return std::make_unique<StringLiteralToken>(string);
                } else if (input[pos] == '\\') {
                    if (pos + 1 >= input.size()) {
                        throw SyntaxError("Unexpected end of string literal");
                    }
                    auto next = input[pos + 1];
                    if (next == 'n') {
                        string += '\n';
                    } else {
                        string += next;
                    }
                    pos += 2;
                } else {
                    string += input[pos];
                    pos++;
                }
            }
            throw SyntaxError("Unexpected end of string literal");
        } else {
            int start = pos;
            do {
                pos++;
            } while (pos < input.size() &&
                     !std::isspace(static_cast<unsigned char>(input[pos])) &&
                     !TOKEN_END.contains(input[pos]));
            auto text = input.substr(start, pos - start);
            if (text == ".") {
                return Token::dot();
            }
            if (std::isdigit(static_cast<unsigned char>(text[0])) ||
                text[0] == '+' || text[0] == '-' || text[0] == '.') {
                try {
                    std::size_t parsed = 0;
                    auto number = std::stod(text, &parsed);
                    if (parsed == text.size()) {
                        return std::make_unique<NumericLiteralToken>(number);
                    }
                } catch (const std::exception&) {
                }
            }
            return std::make_unique<IdentifierToken>(text);
        }
    }
    return nullptr;
}

std::deque<TokenPtr> Tokenizer::tokenize() {
    std::deque<TokenPtr> tokens;
    int pos = 0;
    while (true) {
        auto token = nextToken(pos);
        if (!token) {
            break;
        }
        tokens.push_back(std::move(token));
    }
    return tokens;
}

std::deque<TokenPtr> Tokenizer::tokenize(const std::string& input) {
    return Tokenizer(input).tokenize();
}
