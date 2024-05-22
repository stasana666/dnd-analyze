#include "tokenizer.h"

#include <iostream>

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
    NToken::TNumber result{0};
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
    if (token.has_value()) {
        NToken::TToken res = token.value();
        token.reset();
        return res;
    }
    if (IsOperator()) {
        return ReadOperator();
    }
    if (IsNumber()) {
        return ReadNumber();
    }
    if (IsString()) {
        return ReadString();
    }
    std::cerr << "\"" << input.str() << "\"" << std::endl;
    throw std::logic_error("TTokenizer::Next()");
}

NToken::TToken TTokenizer::Peek()
{
    if (token.has_value()) {
        return token.value();
    }
    token = Next();
    return token.value();
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
