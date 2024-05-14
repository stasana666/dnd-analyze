#pragma once

#include <variant>
#include <string>
#include <sstream>
#include <optional>

namespace NToken {

enum class EOperator {
    Plus,
    Minus,
};

struct TNumber {
    int value;
};

struct TString {
    std::string value;
};

using TToken = std::variant<EOperator, TNumber, TString>;

}

class TTokenizer {
public:
    TTokenizer(const std::string& str)
        : input(str)
    {
        Skip();
    }

    NToken::TToken Next();
    NToken::TToken Peek();

    bool IsEnd() const;

private:
    bool IsOperator();
    bool IsNumber();
    bool IsString();

    NToken::EOperator ReadOperator();
    NToken::TNumber ReadNumber();
    NToken::TString ReadString();

    void Skip();

    std::stringstream input;
    std::optional<NToken::TToken> token;
};
