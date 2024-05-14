#include <iostream>
#include <fstream>

#include <rapidjson/document.h>

#include "statblock.h"
#include "creature.h"
#include "battlefield.h"

using namespace std;
using namespace rapidjson;

TStatblock GetStatblock(std::string path)
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

    return TStatblock(d);
}

int main(int argc, char** argv) {
    TStatblock warrior = GetStatblock(argv[2]);

    TEncounter encounter(2, 2, 0);

    encounter.AddCreature(warrior, 0, 0, 0);
    //encounter.AddCreature(warrior, 0, 0, 0);
    encounter.AddCreature(warrior, 1, 0, 1);
    //encounter.AddCreature(warrior, 1, 0, 1);

    double res = encounter.GetWinProbability();
    std::cout << res << std::endl;
}
