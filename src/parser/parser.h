#pragma once

#include "tokenizer.h"
#include "../distribution.h"
#include <map>

class TParser {
public:
    TParser(const std::map<std::string, int>& variables);

    TDistribution ParseDistribution(const std::string& rawStr) const;
    int Parse(const std::string& rawStr) const;

private:
    TDistribution ReadSummand(TTokenizer& tokenizer) const;

    const std::map<std::string, int>& variables;
};
