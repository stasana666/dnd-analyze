#pragma once

#include <rapidjson/document.h>

#include <vector>
#include <map>

struct TResourceId {
    int Id;
};

std::string GetResourceName(TResourceId id);

class TResource {
public:
    TResource(const rapidjson::Value& json);

    TResourceId id;
    int count;
};

class TResources {
public:
    TResources(const rapidjson::Value& json);

    bool Check(TResource resource) const;
    void Consume(TResource resource);
    void Add(TResource resource);

    void OnTurnStart();
    void OnTurnEnd();
    void Recover(TResource resource);
    void AddTmpResource(TResource resource);

    // При наличии ресурсов, восстанавливающихся с определенной вероятностью
    // нужно рассматривать два варианта развития событий - где ресурс восстановился и где не восстановился
    void NeedDivisionWorld() const;

private:
    std::map<int, int> resources;
    std::map<int, int> tmpResources;
    std::vector<TResourceId> recoverOnTurnStart;
    std::vector<TResourceId> recover56;
};
