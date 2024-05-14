#include "resources.h"

#include <iostream>

namespace {

std::map<std::string, int> resourceIdByName;
std::map<int, std::string> resourceNameById;

}

std::string GetResourceName(TResourceId id)
{
    return resourceNameById.at(id.Id);
}

TResourceId GetResourceId(const std::string& str)
{
    auto it = resourceIdByName.find(str);
    if (it != resourceIdByName.end()) {
        return TResourceId{it->second};
    }
    TResourceId id{static_cast<int>(resourceIdByName.size())};
    resourceNameById[id.Id] = str;
    resourceIdByName[str] = id.Id;
    return id;
}

TResource::TResource(const rapidjson::Value& json)
    : id(GetResourceId(json["name"].GetString()))
    , count(json["count"].GetInt())
{
}

TResources::TResources(const rapidjson::Value& json)
{
    for (auto& resJson : json.GetArray()) {
        TResource res(resJson);
        std::string recovery = resJson.HasMember("recovery") ? resJson["recovery"].GetString() : "";
        resources[res.id.Id] = res.count;
        if (recovery == "on_turn_start") {
            recoverOnTurnStart.emplace_back(res.id);
        }
        if (recovery == "recovery 5-6") {
            recover56.emplace_back(res.id);
        }
    }
}

void TResources::OnTurnStart()
{
    for (TResourceId id : recoverOnTurnStart) {
        resources[id.Id] = 1;
    }
}

void TResources::OnTurnEnd()
{
    tmpResources.clear();
}

bool TResources::Check(TResource resource) const
{
    auto it = resources.find(resource.id.Id);
    auto tmpIt = tmpResources.find(resource.id.Id);
    int count = 0;
    if (it != resources.end()) {
        count += it->second;
    }
    if (tmpIt != tmpResources.end()) {
        count += tmpIt->second;
    }
    return count >= resource.count;
}

void TResources::Consume(TResource resource)
{
    int& tmp = tmpResources[resource.id.Id];
    if (tmp >= resource.count) {
        tmp -= resource.count;
        return;
    }
    resource.count -= tmp;
    tmp = 0;
    resources[resource.id.Id] -= resource.count;
}

void TResources::Add(TResource resource)
{
    resources[resource.id.Id] += resource.count;
}

void TResources::AddTmpResource(TResource resource)
{
    tmpResources[resource.id.Id] += resource.count;
}
