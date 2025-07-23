//
// Created by hemin on 25-7-21.
//

#include "Function.h"

Function::Function(const string& n) : name(n) {}

shared_ptr<BasicBlock> Function::add_block(const string& name)
{
	auto block = make_shared<BasicBlock>(name);
	blocks.push_back(block);
	return block;
}

void Function::print() const
{
	for (const auto& block : blocks)
	{
		block->print();
	}
}
