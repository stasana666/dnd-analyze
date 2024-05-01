#include "tokenizer.h"

bool TTokenizer::IsOperator()
{
    return input.peek() == '+' || input.peek() == '-';
}

bool TTokenizer::IsNumber()
{
    return isdigit(input.peek());
}

bool TTokenizer::IsString()
{
    return isalpha(input.peek());
}

NToken::EOperator TTokenizer::ReadOperator()
{
    char op = input.get();
    NToken::EOperator result = (op == '+' ? NToken::EOperator::Plus : NToken::EOperator::Minus);
    Skip();
    return result;
}

NToken::TNumber TTokenizer::ReadNumber()
{
    NToken::TNumber result;
    while (!input.eof() && isdigit(input.peek())) {
        result.value = result.value * 10 + input.get() - '0';
    }
    Skip();
    return result;
}

NToken::TString TTokenizer::ReadString()
{
    NToken::TString result;
    while (!input.eof() && (isalpha(input.peek()) || input.peek() == '_')) {
        result.value.push_back(input.get());
    }
    Skip();
    return result;
}

NToken::TToken TTokenizer::Next()
{
    if (IsOperator()) {
        return ReadOperator();
    }
    if (IsNumber()) {
        return ReadNumber();
    }
    if (IsString()) {
        return ReadString();
    }
    throw std::logic_error("TTokenizer::Next()");
}

bool TTokenizer::IsEnd() const
{
    return input.eof();
}

void TTokenizer::Skip()
{
    while (!input.eof() && input.peek() == ' ') {
        input.get();
    }
}
