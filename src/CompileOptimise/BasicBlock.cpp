//
// Created by hemin on 25-7-21.
//

#include "BasicBlock.h"

#include <iostream>

BasicBlock::BasicBlock(const string& n) : name(n) {}

void BasicBlock::add_instruction(const Instruction& instruction)
{
	instructions.push_back(instruction);
}

void BasicBlock::print() const
{
	if (!name.empty())
	{
		cout << name << ":" << endl;
	}
	for (const auto& inst : instructions)
	{
		cout << inst.to_string() << endl;
	}
}