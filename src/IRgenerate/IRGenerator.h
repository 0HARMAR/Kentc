#ifndef IRGENERATOR_H
#define IRGENERATOR_H
#include "../../include/json.hpp"
#include "IRWriter.h"
#include <map>
#include <set>

using json = nlohmann::json;
using namespace std;

struct variable
{
    std::string name;
    std::string type;
};

class IRGenerator {
public:
    IRGenerator();
    void generateIR(const json &program, std::string &outputIR);

private:
    IRWriter irWriter;
    std::string generateExpr(const json &expr);
    std::map<string, vector<variable>> functionVariables;
    std::map<string, vector<variable>> functionArguments;
    std::vector<variable>& variables = functionVariables["main"];
    void parseStatements(const json &statement);
    string getVarTypeByName(string name);
    string currentFunction = "main";
    int looperLabelNum = 0;

    map<string,string> typeToIRtype = {
        {"int", "i32"}
    };
};

#endif
