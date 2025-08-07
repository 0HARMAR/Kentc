//
// Created by hemin on 25-7-15.
//

#ifndef IRGENERATOR_H
#define IRGENERATOR_H
#include "../../include/json.hpp"
#include <map>
#include <set>

using json = nlohmann::json;

struct variable
{
	std::string name;
	std::string type;
};

class IRGenerator {
public:
	void generateIR(const json &program, std::string &outputIR);

private:
	std::string generateExpr(const json &expr,int &tempRegCount, std::string &ir);
	std::vector<variable> variables;
	void parseStatements(const json &statement, std::string &outputIR);

	int index = 0;
};



#endif //IRGENERATOR_H
