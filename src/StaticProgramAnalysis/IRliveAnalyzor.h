//
// Created by hemin on 25-7-30.
//

#ifndef IRLIVEANALYZOR_H
#define IRLIVEANALYZOR_H
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
#include <regex>
#include <sstream>
#include "../../include/stringUtils.h"

using namespace std;

class IRliveAnalyzor {
public:
	map<string, pair<int, int>> calculateLiveRanges(const string& irCode);
};



#endif //IRLIVEANALYZOR_H
