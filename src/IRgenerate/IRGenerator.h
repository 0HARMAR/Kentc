//
// Created by hemin on 25-7-15.
//

#ifndef IRGENERATOR_H
#define IRGENERATOR_H
#include "../../include/json.hpp"
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
	std::string generateExpr(const json &expr,int &tempRegCount, std::string &ir);
	// func name -> variables
	std::map<string, vector<variable>> functionVariables;
	// func name -> arguments
	map<string, vector<variable>> functionArguments;
	std::vector<variable>& variables = functionVariables["main"];
	void parseStatements(const json &statement, std::string &outputIR);

	int index = 0;

	map<string,string> typeToIRtype = {
		{"int", "i32"}
	};
};



#endif //IRGENERATOR_H
