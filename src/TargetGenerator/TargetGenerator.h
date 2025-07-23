//
// Created by hemin on 25-7-16.
//

#ifndef TARGETGENERATOR_H
#define TARGETGENERATOR_H
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <cctype>
#include "RegisterAllocator.h"
#include "../StaticProgramAnalysis/StaticProgramAnalyzer.h"
#include "CallingConvention.h"
using namespace std;
struct Variable
{
	string name;
	string type;
	int stackOffset;
};

struct Instruction
{
	string op;
	vector<string> operands;
	string result;
};

class TargetGenerator
{
public:
	void addAsmLine(const string& line);
	void addAsmLine(const vector<string>& lines);
	string createTempVar();
	void processAlloca(const vector<string>& tokens);
	void processStore(const vector<string>& tokens);
	string processLoad(const vector<string>& tokens);
	string processBinaryOp(const vector<string>& tokens);
	void processCall(const vector<string>& tokens);
	vector<string> convertIRToASM(const vector<string>& irLines);
	void handleDivision(const string& dividend, const string& divisor, string& result);

private:
	// program state
	vector<string> asmLines;
	map<string, Variable> variables;
	int stackSize = 0;
	int maxStackOffset = 0;
	int nextTempVar = 0;

	// var map
	map<string, int> variableMap = {
		{"i32", 4},
	};

	// operator map
	map<string, string> opMap = {
		{"add", "addl"},
		{"sub", "subl"},
		{"mul", "imull"},
		{"sdiv", "idivl"},
	};

	// register allocator
	RegisterAllocator registerAllocator;

	// static program analyzer
	StaticProgramAnalyzer staticProgramAnalyzer;

	// assist func
	string trim(const string& str);
	string normalizeReg(string reg);
};

#endif //TARGETGENERATOR_H
