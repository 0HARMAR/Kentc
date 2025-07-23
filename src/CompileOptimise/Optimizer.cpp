//
// Created by hemin on 25-7-21.
//

#include "Optimizer.h"

Optimizer::Optimizer(const Function& func) : function(func) {}

Function Optimizer::optimize()
{
	for (auto& block_ptr : function.blocks)
	{
		BasicBlock& block = *block_ptr;

		dead_code_elimination(block);
		constant_propagation(block);
		call_sequence_optimization(block);
		peephole_optimization(block);
		stack_optimization(block);
	}

	return function;
}

