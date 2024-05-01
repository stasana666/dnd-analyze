#include <iostream>
#include <fstream>

#include <rapidjson/document.h>

#include "statblock.h"

using namespace std;
using namespace rapidjson;

int main(int argc, char** argv) {
    std::string json;
    ifstream in(argv[2]);
    while (!in.eof()) {
        std::string s;
        in >> s;
        json += s;
    }
    Document d;
    d.Parse(json.c_str());

    // Воин 1 уровня
    TStatblock warrior(d);
}
