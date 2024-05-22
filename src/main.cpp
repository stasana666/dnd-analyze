#include <iostream>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

#include "statblock.h"
#include "statblock_json_schema.h"
#include "creature.h"
#include "battlefield.h"

using namespace std;
using namespace rapidjson;

TStatblock GetStatblock(SchemaValidator& validator, std::string path)
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

    return TStatblock(d);
}

int main(int argc, char** argv) {
    Document sd;
    if (sd.Parse(jsonSchema).HasParseError()) {
        throw std::logic_error("json schema is incorrect");
    }

    SchemaDocument schema(sd);

    SchemaValidator validator(schema);

    std::cerr << "schema create" << std::endl;
    TStatblock cleric = GetStatblock(validator, argv[1]);
    std::cerr << argv[1] << std::endl;
    TStatblock chimera = GetStatblock(validator, argv[2]);
    std::cerr << argv[2] << std::endl;

    TEncounter encounter(2, 2, 0);

    encounter.AddCreature(cleric, 0, 0, 0);
    encounter.AddCreature(chimera, 1, 0, 1);

    double res = encounter.GetWinProbability();
    std::cout << res << std::endl;
}
