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
class RegisterAllocator {
public:
	void allocMemoryAddr(string addrVar, int size=4);
	string getMemLocation(string addrVar);
	string allocReg(string var);
	string spillVar(string var);
	string selectSpillCandidate();
	void freeReg(string var, bool lastUse=true);
	string getTempVarLocation(string operand);
	void handleCall();
	string getVarInRegister(string reg);
	void spillRegister(string var);
private:
	vector<string> freeRegs = {"%r8d","r9d", "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"};
	unordered_map<string, string> varToReg; // variable to register mapping
	unordered_map<string, int> addrToStack; // address variable to stack offset mapping
	unordered_map<string, int> spilledVars; // spilled variables to stack offset mapping
	int stackOffset = -8; // current stack offset
};



#endif //REGISTERALLOCATOR_H
