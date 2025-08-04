//
// Created by hemin on 25-7-22.
//

#ifndef ASSEMBLYPARSER_H
#define ASSEMBLYPARSER_H
#include "Function.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include "../../include/stringUtils.h"
using namespace std;

class AssemblyParser {
public:
	static Function parse(const vector<string>& lines);
};



#endif //ASSEMBLYPARSER_H
