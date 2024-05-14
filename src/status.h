#pragma once

#include <string>

enum class EStatus {
    Fallen,
    Poison,
};

EStatus GetStatusByString(const std::string& name);
