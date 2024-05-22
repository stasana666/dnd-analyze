#include "parser.h"

#include "tokenizer.h"

#include <iostream>

namespace {

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}

TParser::TParser(const std::map<std::string, int>& variables)
    : variables(variables)
{
}

void PrintToken(NToken::TToken token) {
    std::visit(overloaded{
        [&](NToken::EOperator op) {
            std::cerr << "operator: " << (op == NToken::EOperator::Plus ? "+" : "-") << std::endl;
        },
        [&](NToken::TNumber number) {
            std::cerr << "number: " << number.value << std::endl;
        },
        [&](NToken::TString str) {
            std::cerr << "string: " << "\"" << str.value << "\"" << std::endl;
        }
    }, token);
}

TDistribution TParser::ParseDistribution(const std::string& rawStr) const
{
    TTokenizer tokenizer(rawStr);
    TDistribution result = ReadSummand(tokenizer);

    while (!tokenizer.IsEnd()) {
        NToken::TToken token = tokenizer.Next();
        // PrintToken(token);

        if (!std::holds_alternative<NToken::EOperator>(token)) {
            throw std::logic_error("can\'t parse: " + rawStr);
        }
        
        NToken::EOperator op = std::get<NToken::EOperator>(token);
        switch (op) {
            case NToken::EOperator::Plus: {
                result += ReadSummand(tokenizer);
                break;
            }
            case NToken::EOperator::Minus: {
                result -= ReadSummand(tokenizer);
                break;
            }
        }
    }
    // result.Print();
    return result;
}

TDistribution TParser::ReadSummand(TTokenizer& tokenizer) const
{
    NToken::TToken token = tokenizer.Next();
    //PrintToken(token);
    TDistribution res(0);

    std::visit(overloaded{
        [&](NToken::EOperator op) {
            throw std::logic_error("unexpected operator");
        },
        [&](NToken::TNumber number) {
            if (tokenizer.IsEnd()) {
                res = TDistribution(number.value);
            }
            if (!tokenizer.IsEnd()) {
                NToken::TToken token = tokenizer.Peek();
                //PrintToken(token);
                if (std::holds_alternative<NToken::TString>(token)) {
                    NToken::TString str = std::get<NToken::TString>(token);
                    if (str.value != "d") {
                        throw std::logic_error("expect \'d\', but found " + str.value);
                    }
                    tokenizer.Next();

                    token = tokenizer.Next();
                    //PrintToken(token);
                    if (std::holds_alternative<NToken::TNumber>(token)) {
                        NToken::TNumber dice_size = std::get<NToken::TNumber>(token);
                        res = TDistribution(TDice{dice_size.value}) * number.value;
                    }
                } else {
                    res = TDistribution(number.value);
                }
            } else {
                res = TDistribution(number.value);
            }
            
        },
        [&](NToken::TString str) {
            if (str.value == "d") {
                NToken::TToken token = tokenizer.Next();
                if (std::holds_alternative<NToken::TNumber>(token)) {
                    NToken::TNumber dice_size = std::get<NToken::TNumber>(token);
                    res = TDistribution(TDice{dice_size.value});
                } else {
                    throw std::logic_error("expected number after \'d\'");
                }
            } else {
                res = TDistribution(variables.at(str.value));
            }
        }
    }, token);

    return res;
}

int TParser::Parse(const std::string& rawStr) const
{
    TDistribution distribution = ParseDistribution(rawStr);
    std::mt19937 rng;
    return distribution.GetValue(rng);
}
