//
// Created by hemin on 25-7-21.
//

#ifndef OPTIMIZER_H
#define OPTIMIZER_H
#include <map>

#include "Function.h"

using namespace std;

class Optimizer {
private:
	Function function;

	// trace the const
	map<pair<string, int>, long> mem_constants; // (base, offset) -> value
	map<string, long> reg_constants; // reg -> value

	// remove dead code
	void dead_code_elimination(BasicBlock& block);

	// constant propagation
	void constant_propagation(BasicBlock& block);

	// call sequence optimization
	void call_sequence_optimization(BasicBlock& block);

	// peephole optimization
	void peephole_optimization(BasicBlock& block);

	// stack optimization
	void stack_optimization(BasicBlock& block);

public:
	Optimizer(const Function& func);

	Function optimize();
};



#endif //OPTIMIZER_H
