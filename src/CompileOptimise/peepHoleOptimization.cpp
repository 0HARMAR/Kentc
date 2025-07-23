#include "Optimizer.h"

void Optimizer::peephole_optimization(BasicBlock& block)
{
	vector<Instruction> new_instructions;

	for (auto& inst : block.instructions)
	{
		// simplify register move seq
		if (inst.opcode == "movl" && inst.operands.size() == 2 &&
			inst.operands[0].type == OperandType::REGISTER &&
			inst.operands[1].type == OperandType::REGISTER)
		{
			// movl %eax, %eax is a no-op
			if (inst.operands[0].reg != inst.operands[1].reg)
			{
				new_instructions.push_back(inst);
			}
			continue;
		}
		new_instructions.push_back(inst);
	}

	block.instructions = new_instructions;
}
