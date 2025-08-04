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
				// check mem whether a const
				string baseReg = src.base_reg;
				long disp = src.displacement;
				if (mem_constants.find({baseReg, disp}) != mem_constants.end()) // mem is const
				{
					// propagate mem const to reg
					reg_constants[dest.reg] = mem_constants[{baseReg, disp}];
				}
			}
			// 4. reg to reg
			else if (src.type == OperandType::REGISTER &&
					dest.type == OperandType::REGISTER)
			{
				// ignore rax
				if (dest.reg == "rax") continue;
				if (reg_constants.count(src.reg))
				{
					// propagate reg const to reg
					reg_constants[dest.reg] = reg_constants[src.reg];
				}
			}
		}
		// process arithmetic inst
		else if (inst.opcode == "addl" || inst.opcode == "subl" ||
			inst.opcode == "imull" || inst.opcode == "idivl")
		{
			Operand src = inst.operands[0];
			Operand dest;

			optional<long> src_val = nullopt;
			optional<long> dest_val = nullopt;

			if (inst.opcode == "idivl")
			{
				if (src.type == OperandType::REGISTER)
				{
					src_val = (reg_constants.count(src.reg) ? reg_constants[src.reg] : -1);
				} else if (src.type == OperandType::MEMORY)
				{
					src_val = (mem_constants.count({src.base_reg, src.displacement}) ? mem_constants[{src.base_reg, src.displacement}] : -1);
				}

				dest_val = (reg_constants.count("eax") ? reg_constants["eax"] : -1);
				goto proc_op;
			} else
			{
				dest = inst.operands[1];
			}

			// propagate src and dest value
			if (src.type == OperandType::IMMEDIATE)
			{
				src_val = src.value;
			} else if (src.type == OperandType::REGISTER)
			{
				src_val = (reg_constants.count(src.reg) ? reg_constants[src.reg] : -1).value();
			} else if (src.type == OperandType::MEMORY)
			{
				src_val = (mem_constants.count({src.base_reg, src.displacement}) ? mem_constants[{src.base_reg, src.displacement}] : -1);
			}

			if (dest.type == OperandType::REGISTER)
			{
				dest_val = (reg_constants.count(dest.reg) ? reg_constants[dest.reg] : -1).value();
			} else if (dest.type == OperandType::MEMORY)
			{
				dest_val = (mem_constants.count({dest.base_reg, dest.displacement}) ? mem_constants[{dest.base_reg, dest.displacement}] : -1);
			}

			// if two operand const, folding calculate
proc_op:	if (src_val != nullopt && dest_val != nullopt)
			{
				long result = 0;
				if (inst.opcode == "addl")
				{
					result = dest_val.value() + src_val.value();
				} else if (inst.opcode == "subl")
				{
					result = dest_val.value() - src_val.value();
				} else if (inst.opcode == "imull")
				{
					result = dest_val.value() * src_val.value();
				} else if (inst.opcode == "idivl")
				{
					if (src_val != 0)
					{
						result = dest_val.value() / src_val.value();
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
	}
}