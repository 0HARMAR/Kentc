#include "Optimizer.h"

void Optimizer::stack_optimization(BasicBlock& block)
{
	// calculate needed stack space
	int stack_space_needed = 0;
	for (auto& inst : block.instructions)
	{
		if (inst.is_store() &&
			inst.operands[1].type == OperandType::MEMORY &&
			inst.operands[1].base_reg == "rbp")
		{
			int offset = inst.operands[1].displacement;
			if (offset < 0) // negative offset means local variable
			{
				stack_space_needed = max(stack_space_needed, -offset);
			}
		}
	}

	// make sure stack space is aligned to 16 bytes
	stack_space_needed = (stack_space_needed + 15) & ~15;

	// update stack alloca inst
	for (auto& inst : block.instructions)
	{
		if (inst.opcode == "subq" && inst.operands.size() == 2 &&
			inst.operands[0].type == OperandType::IMMEDIATE &&
			inst.operands[1].type == OperandType::REGISTER &&
			inst.operands[1].reg == "rsp")
		{
			inst.operands[0] = Operand::create_immediate(stack_space_needed);
			break;
		}
	}
}
