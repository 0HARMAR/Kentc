//
// Created by hemin on 25-7-19.
//

#ifndef REGISTERALLOCATOR_H
#define REGISTERALLOCATOR_H
#include <unordered_map>
using namespace std;
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
class RegisterAllocator {
public:
	void allocMemoryAddr(string addrVar, int size=4);
	string getMemLocation(string addrVar);
	string allocReg(string var, vector<string>exclude = {});
	string allocReg(string var, string reg);
	string allocReg();
	string spillVar(string var);
	string selectSpillCandidate();
	void freeReg(string var, bool lastUse=true);
	string getTempVarLocation(string operand);
	void handleCall();
	string getVarInRegister(string reg);
	string spillRegister(string var);
private:
	vector<string> freeRegs = {"%rdi", "%rsi", "%rax", "%rbx", "%rcx", "%rdx",
		"%r8","%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
	unordered_map<string, string> varToReg; // variable to register mapping
	unordered_map<string, int> addrToStack; // address variable to stack offset mapping
	unordered_map<string, int> spilledVars; // spilled variables to stack offset mapping
	int stackOffset = -8; // current stack offset
};



#endif //REGISTERALLOCATOR_H
