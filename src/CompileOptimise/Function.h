//
// Created by hemin on 25-7-21.
//

#ifndef FUNCTION_H
#define FUNCTION_H
#include <memory>

#include "BasicBlock.h"

using namespace std;

class Function {
public:
	string name;
	vector<shared_ptr<BasicBlock>> blocks;

	Function(const string& n);
	shared_ptr<BasicBlock> add_block(const string& name);
	void print() const;
};



#endif //FUNCTION_H
