#include "status.h"

#include <map>

namespace {

std::map<std::string, EStatus> statusByName = {
    {"fallen", EStatus::Fallen},
    {"poison", EStatus::Poison}
};

}

EStatus GetStatusByString(const std::string& name)
{
    return statusByName.at(name);
}
