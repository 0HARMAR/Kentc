//
// Created by hemin on 25-7-15.
//

#ifndef IRGENERATOR_H
#define IRGENERATOR_H
#include "../../include/json.hpp"
using json = nlohmann::json;

class IRGenerator {
public:
	void generateIR(const json &program, std::string &outputIR);

private:
	std::string generateExpr(const json &expr, std::map<std::string,int> &varAddrMap,
		int &tempRegCount, std::string &ir);
};



#endif //IRGENERATOR_H
