//
// Created by hemin on 25-7-21.
//

#ifndef BASICBLOCK_H
#define BASICBLOCK_H
#include "Instruction.h"

using namespace std;

class BasicBlock {
public:
	string name;
	vector<Instruction> instructions;

	BasicBlock(const string& n);
	void add_instruction(const Instruction& instruction);
	void print() const;
};



#endif //BASICBLOCK_H
