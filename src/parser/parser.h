#pragma once

#include "tokenizer.h"
#include "distribution.h"

class TParser {
public:
    TParser(const std::map<std::string, int>& variables);

    TDistribution ParseDistribution(const std::string& rawStr);
    int Parse(const std::string& rawStr);

private:

};
