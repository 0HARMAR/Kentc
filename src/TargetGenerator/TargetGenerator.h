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
#include "AsmWriter.h"
#include <stdint.h>
#include "../../include/stringUtils.h"
#include "../StaticProgramAnalysis/IRliveAnalyzor.h"
#include <regex>
using namespace std;
struct Variable
{
	string name;
	string type;
	int stackOffset;
};

extern "C" void* malloc_at(size_t size, size_t offset);

class TargetGenerator
{
public:
	TargetGenerator();
	void addAsmLine(const string& line);
	void addAsmLine(const vector<string>& lines);
	string createTempVar();
	void processAlloca(const vector<string>& tokens);
	void processStore(const vector<string>& tokens);
	string processLoad(const vector<string>& tokens);
	string processBinaryOp(const vector<string>& tokens);
	void processCall(const vector<string>& tokens);
	void processInttoptr(const vector<string>& tokens);
	void processIcmp(const vector<string>& tokens);
	void processBr(const vector<string>& tokens);
	void processString(const vector<string>& tokens);
	void processGetElementPtr(const vector<string>& tokens);
	vector<string> convertIRToASM(const vector<string>& irLines);
	void handleDivision(const string& dividend, const string& divisor, string& result);
	void handlePrintInt(vector<string>);
	void handleExit(vector<string>);
	void handleMallocAt(vector<string>);
	void handleIn(vector<string>);
	void handlePrintString(vector<string>);
private:
	// program state
	vector<string> asmLines;
	map<string, Variable> variables;
	int stackSize = 0;
	int maxStackOffset = 0;
	int nextTempVar = 0;

	// rodata section index
	int sectionIndex = 2;

	// bit wide -> mov subfix
	map<int, string> bitWideToMovSubfix = {
		{8, "b"},
		{16, "w"},
		{32, "l"},
		{64, "q"}
	};

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

	// string table
	// name -> (length, content)
	map<string, pair<int, string>> stringTable;

	// register allocator
	RegisterAllocator registerAllocator;

	// static program analyzer
	StaticProgramAnalyzer staticProgramAnalyzer;

	// asm writer
	AsmWriter asmWriter;

	// ir live analyse
	IRliveAnalyzor irLiveAnalyzer;

	// assist func
	string normalizeReg(string reg);
	string denormalizeReg(string reg, int bitWide);
	string formatReg(string reg, int bitWide);
	vector<string> parseCall(string callStr);
	vector<string> parseString(string str);
	bool isMemory(string op);
};

#endif //TARGETGENERATOR_H
