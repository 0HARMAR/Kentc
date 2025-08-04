//
// Created by hemin on 25-7-19.
//

#ifndef REGISTERALLOCATOR_H
#define REGISTERALLOCATOR_H
#include <unordered_map>
#include "AsmWriter.h"
using namespace std;
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
class RegisterAllocator {
public:
	RegisterAllocator(AsmWriter& asmWriter);
	void allocMemoryAddr(string addrVar, int size=4);
	string getMemLocation(string addrVar);
	string allocReg(string var, vector<string>exclude = {});
	string allocReg(string var, string reg);
	string allocReg(vector<string> exclude = {});
	string spillVar(string var);
	string spillVar();
	string selectSpillCandidate();
	void freeReg(string var);
	string getTempVarLocation(string operand);
	void handleCall();
	string getVarInRegister(string reg);
	string spillRegister(string var);
	void extandStack();
	bool haveFreeReg();
	string getRandomTransitReg();
private:
	AsmWriter& asmWriter;
	vector<string> freeRegs = {"%rdi", "%rsi", "%rax", "%rbx", "%rcx", "%rdx",
		"%r8","%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
	unordered_map<string, string> varToReg; // variable to register mapping
	unordered_map<string, int> addrToStack; // address variable to stack offset mapping
	unordered_map<string, int> spilledVars; // spilled variables to stack offset mapping
	int stackOffset = -0; // current stack offset
	int stackSize = -16;
};



#endif //REGISTERALLOCATOR_H
