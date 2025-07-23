#include "Optimizer.h"

void Optimizer::constant_propagation(BasicBlock& block)
{
	for (Instruction& inst : block.instructions)
	{
		// process move inst
		if (inst.opcode.substr(0, 3) == "mov")
		{
			Operand src = inst.get_src();
			Operand dest = inst.get_dest();

			// 1. immediate to reg
			if (src.type == OperandType::IMMEDIATE &&
				dest.type == OperandType::REGISTER)
			{
				reg_constants[dest.reg] = src.value;
			}
			// 2. immediate to mem
			else if (src.type == OperandType::IMMEDIATE &&
				dest.type == OperandType::MEMORY)
			{
				mem_constants[{dest.base_reg, dest.displacement}] = src.value;
			}
			// 3. mem to reg
			else if (src.type == OperandType::MEMORY &&
				dest.type == OperandType::REGISTER)
			{
				auto key = make_pair(src.base_reg, src.displacement);
				if (mem_constants.find(key) != mem_constants.end())
				{
					// replace to immediate
					inst.opcode = "movl";
					inst.operands[0] = Operand::create_immediate(mem_constants[key]);
					reg_constants[dest.reg] = mem_constants[key];
				} else
				{
					reg_constants.erase(dest.reg);
				}
			}
			// 4. reg to reg
			else if (src.type == OperandType::REGISTER &&
					dest.type == OperandType::REGISTER)
			{
				if (reg_constants.find(src.reg) != reg_constants.end()) {
					reg_constants[dest.reg] = reg_constants[src.reg];
				} else {
					reg_constants.erase(dest.reg);
				}
			}
		}
		// process arithmetic inst
		else if (inst.opcode == "addl" || inst.opcode == "subl" ||
			inst.opcode == "imull" || inst.opcode == "idivl")
		{
			if (inst.operands.size() < 2) continue;

			Operand src = inst.operands[0];
			Operand dest = inst.operands[1];

			// only dest is reg
			if (dest.type != OperandType::REGISTER) continue;

			long src_val = -1;
			long dest_val = reg_constants.count(dest.reg) ? reg_constants[dest.reg] : -1;

			// get src value
			if (src.type == OperandType::IMMEDIATE)
			{
				src_val = src.value;
			} else if (src.type == OperandType::REGISTER &&
				reg_constants.count(src.reg))
			{
				src_val = reg_constants[src.reg];
			}

			// if two operand const, folding calculate
			if (src_val != -1 && dest_val != -1)
			{
				long result = 0;
				if (inst.opcode == "addl")
				{
					result = dest_val + src_val;
				} else if (inst.opcode == "subl")
				{
					result = dest_val - src_val;
				} else if (inst.opcode == "imull")
				{
					result = dest_val * src_val;
				} else if (inst.opcode == "idivl")
				{
					if (src_val != 0)
					{
						result = dest_val / src_val;
					}
				}

				// replace to immediate move inst
				inst.opcode = "movl";
				inst.operands.clear();
				inst.operands.push_back(Operand::create_immediate(result));
				inst.operands.push_back(dest);

				reg_constants[dest.reg] = result;
			} else
			{
				reg_constants.erase(dest.reg);
			}
		}
		// function call remove register const
		else if (inst.is_call())
		{
			// remove caller save registers
			vector<string> caller_saved = {
				"rax", "eax", "rdi", "edi", "rsi", "esi",
					"rdx", "edx", "rcx", "ecx", "r8", "r8d",
					"r9", "r9d", "r10", "r10d", "r11", "r11d"
			};

			for (const string& reg : caller_saved)
			{
				reg_constants.erase(reg);
			}

			// assume call may change memory
			mem_constants.clear();
		}
	}
}