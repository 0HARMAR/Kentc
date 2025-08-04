//
// Created by hemin on 25-7-21.
//

#include "Optimizer.h"

#include <iostream>

Optimizer::Optimizer(const Function& func) : function(func) {}

Function Optimizer::optimize()
{
	for (auto& block_ptr : function.blocks)
	{
		BasicBlock& block = *block_ptr;

		dead_code_elimination(block);
		block.print(); std::cout << std::endl;
		constant_propagation(block);
		block.print(); std::cout << std::endl;
		// call_sequence_optimization(block);
		// block.print(); std::cout << std::endl;
		// peephole_optimization(block);
		// stack_optimization(block);
	}

	return function;
}

