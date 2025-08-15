//
// Created by hemin on 25-8-11.
//

#ifndef UTILS_H
#define UTILS_H
#include <map>
#include <string>

using namespace std;
extern map<int, string> bitWideToMovSubfix;
extern map<string, int> variableMap;

string formatReg(string reg, int bitWide);
#endif //UTILS_H
