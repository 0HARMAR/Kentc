//
// Created by hemin on 25-7-20.
//

#ifndef CALLINGCONVENTION_H
#define CALLINGCONVENTION_H
#include <vector>
#include <string>
using namespace std;
class CallingConvention {
public:
	static vector<string> callerSaved;
	static vector<string> calleeSaved;
	static vector<string> parameters;
	static string returnReg;
	static int alignment;

	static vector<string> addCallerSave();
	static vector<string> restoreCallerSave();
	static vector<string> addCalleeSave();
	static vector<string> restoreCalleeSave();
};



#endif //CALLINGCONVENTION_H
