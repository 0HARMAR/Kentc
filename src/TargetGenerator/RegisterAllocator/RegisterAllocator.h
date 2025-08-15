//
// Created by hemin on 25-7-19.
//

#ifndef REGISTERALLOCATOR_H
#define REGISTERALLOCATOR_H
#include <unordered_map>
#include "../AsmWriter/AsmWriter.h"
using namespace std;
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "../include/utils.h"
class RegisterAllocator {
public:
	RegisterAllocator(AsmWriter& asmWriter, int& stackSize, map<string, int>& stackOffset, string& currentFunction);
	void allocMemoryAddr(string addrVar, int size=4);
	string getMemLocation(string addrVar);
	string allocReg(string var, string type, vector<string>exclude = {});
	string allocFixReg(string var, string reg);
	string allocReg(vector<string> exclude = {});
	string allocaArguments(string var, int index);
	string spillVar(string var, string type);
	string spillVar();
	string selectSpillCandidate();
	void freeReg(string var);
	string getTempVarLocation(string operand);
	void handleCall();
	string getVarInRegister(string reg);
	string spillRegister(string var, string type);
	void extandStack(bool inMain);
	bool haveFreeReg();
	string getRandomTransitReg(string = "");
	vector<string> allocaHistory;

private:
	AsmWriter& asmWriter;
	int& stackSize;
	map<string, int>& stackOffset; // current stack offset
	string& currentFunction;
	vector<string> freeRegs = {"%rdi", "%rsi", "%rax", "%rbx", "%rcx", "%rdx",
		"%r8","%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
	unordered_map<string, string> varToReg; // variable to register mapping
	unordered_map<string, int> addrToStack; // address variable to stack offset mapping
	unordered_map<string, int> spilledVars; // spilled variables to stack offset mapping
	unordered_map<string, string> varToArgumentReg; // argument variable to argument reg
};



#endif //REGISTERALLOCATOR_H
