//
// Created by hemin on 25-7-21.
//

#include "Instruction.h"

string Instruction::to_string() const
{
	string s;
	if (!label.empty())
	{
		s += label + ":";
	}
	s += "\t" + opcode;
	for (size_t i = 0; i < operands.size(); ++i)
	{
		if (i == 0) s += "\t";
		else s += ", ";
		s += operands[i].to_string();
	}
	if (!comment.empty())
	{
		s += "/t# " + comment;
	}
	return s;
}

bool Instruction::is_store() const
{
	return opcode.substr(0, 3) == "mov" && !opcode.empty() &&
		operands[0].type == OperandType::IMMEDIATE &&
			operands.size() > 1 && operands[1].type == OperandType::MEMORY;
}

bool Instruction::is_load() const
{
	return opcode.substr(0, 3) == "mov" && !opcode.empty() &&
		operands[0].type == OperandType::MEMORY &&
			operands.size() > 1 && operands[1].type == OperandType::REGISTER;
}

bool Instruction::is_call() const
{
	return opcode == "call";
}

bool Instruction::is_exit() const
{
	return opcode == "call" && !opcode.empty() &&
		operands[0].type == OperandType::LABEL &&
			operands[0].label == "exit";
}

Operand Instruction::get_dest() const
{
	if (opcode.substr(0, 3) == "mov" && operands.size() > 1)
	{
		return operands[1];
	}
	return Operand();
}

Operand Instruction::get_src() const
{
	if (!operands.empty())
	{
		return operands[0];
	}
	return Operand();
}

