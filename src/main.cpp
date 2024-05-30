#include <iostream>
#include <fstream>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

#include "statblock.h"
#include "json_schema.h"
#include "creature.h"
#include "battlefield.h"

using namespace std;
using namespace rapidjson;

int playersCount = 0;

TEncounter EncounterFromJson(SchemaValidator& validator, std::string path)
{
    std::string json;
    ifstream in(path);
    while (!in.eof()) {
        std::string s;
        in >> s;
        json += s;
    }
    Document d;
    d.Parse(json.c_str());

    if (!d.Accept(validator)) {
        StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        throw std::logic_error(path + " schema is invalid: " + sb.GetString() + ", " + validator.GetInvalidSchemaKeyword());
    }

    std::unordered_map<std::string, std::shared_ptr<const TStatblock>> statblockByName;
    for (auto& statblockJson : d["statblocks"].GetArray()) {
        TStatblock statblock(statblockJson);
        statblockByName[statblockJson["name"].GetString()] = std::make_shared<const TStatblock>(statblockJson);
    }
    TEncounter encounter(d["width"].GetInt(), d["height"].GetInt());
    for (auto& creatureJson : d["creatures"].GetArray()) {
        if (creatureJson["team"].GetInt() == 0) {
            ++playersCount;
        }
        encounter.AddCreature(statblockByName.at(creatureJson["name"].GetString()), creatureJson["x"].GetInt(), creatureJson["y"].GetInt(), creatureJson["team"].GetInt());
    }
    return encounter;
}

int main(int argc, char** argv) {
    Document sd;
    if (sd.Parse(jsonSchema).HasParseError()) {
        throw std::logic_error("json schema is incorrect");
    }

    SchemaDocument schema(sd);

    SchemaValidator validator(schema);

    TEncounter encounter = EncounterFromJson(validator, argv[1]);

    TResult res = encounter.GetWinProbability();
    std::cout << "prob players win: " << res.probWin << std::endl;
    for (int i = 0; i <= playersCount; ++i) {
        std::cerr << "prob that " << i << " players alive: " << res.alivePlayers[i] << std::endl;
    }
}
