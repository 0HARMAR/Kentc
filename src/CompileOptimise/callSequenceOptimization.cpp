#include "Optimizer.h"

void Optimizer::call_sequence_optimization(BasicBlock& block)
{
	vector<Instruction> new_instructions;
	vector<Instruction>& insts = block.instructions;

	for (size_t i = 0; i < insts.size(); ++i)
	{
		// check register push sequence
		if (i + 10 < insts.size() && insts[i].opcode == "push")
		{
			bool is_call_seq = true;
			size_t start = i;
			size_t push_count = 0;

			// check continuous push instructions
			while (i < insts.size() && insts[i].opcode == "push")
			{
				push_count++;
				i++;
			}

			// check subq $8, %rsp
			if (i < insts.size() && insts[i].opcode == "subq" &&
				insts[i].operands.size() == 2 &&
				insts[i].operands[0].type == OperandType::IMMEDIATE &&
				insts[i].operands[0].value == 8 &&
				insts[i].operands[1].type == OperandType::REGISTER &&
				insts[i].operands[1].reg == "rsp")
			{
				i++;
			} else
			{
				is_call_seq = false;
			}

			// check call instruction
			if (i < insts.size() && insts[i].is_call())
			{
				// save call instruction
				Instruction call_inst = insts[i];
				i++;

				// check addq $8, %rsp
				if (i < insts.size() && insts[i].opcode == "addq" &&
				insts[i].operands.size() == 2 &&
				insts[i].operands[0].type == OperandType::IMMEDIATE &&
				insts[i].operands[0].value == 8 &&
				insts[i].operands[1].type == OperandType::REGISTER &&
				insts[i].operands[1].reg == "rsp")
				{
					i++;
				}

				// check continuous pop instructions
				bool pop_match = true;
				size_t pop_count = 0;
				for (size_t j = 0; j < push_count; ++j)
				{
					if (i >= insts.size() || insts[i].opcode != "pop")
					{
						pop_match = false;
						break;
					}
					i++;
					pop_count++;
				}

				// if all match, then replace
				if (pop_match)
				{
					// remove push/pop seq, retain call instruction
					new_instructions.push_back(call_inst);
					continue;
				}
			} else
			{
				is_call_seq = false;
			}

			// if not call sequence, then back process
			if (!is_call_seq)
			{
				i = start; // back to push start
			}
		}

		// process normal instruction
		if (i < insts.size())
		{
			new_instructions.push_back(insts[i]);
		}
	}

	block.instructions = new_instructions;
}
