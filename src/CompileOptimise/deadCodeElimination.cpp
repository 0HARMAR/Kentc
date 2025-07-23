#include "Optimizer.h"
#include <set>
void Optimizer::dead_code_elimination(BasicBlock& block)
{
	vector<Instruction> new_instructions;
	vector<Instruction>& insts = block.instructions;
	bool exit_found = false;

	// store instruction cover analyze
	map<pair<string, int>, int> last_store;
	set<pair<string,int>> used_memory;

	for (int i = 0; i < insts.size(); ++i)
	{
		Instruction& inst = insts[i];

		// 1. remove code after exit
		if (exit_found)
		{
			continue;
		}
		if (inst.is_exit())
		{
			exit_found = true;
			new_instructions.push_back(inst);
			continue;
		}

		// 2. remove memory cover store
		if (inst.is_store())
		{
			Operand dest = inst.get_dest();
			if (dest.type == OperandType::MEMORY)
			{
				pair<string, int> loc = {dest.base_reg, dest.displacement};

				// check before store whether used
				if (last_store.find(loc) == last_store.end() &&
					used_memory.find(loc) == used_memory.end())
				{
					// remove covered store
					int prev_idx = last_store[loc];
					new_instructions.erase(new_instructions.begin() + prev_idx);
				}

				last_store[loc] = new_instructions.size();
				// erase the memory loc used mark
				used_memory.erase(loc);
			}
		}

		// 3. mark memory used
		if (inst.is_load())
		{
			Operand src = inst.get_src();
			if (src.type == OperandType::MEMORY)
			{
				pair<string, int> loc = {src.base_reg, src.displacement};
				used_memory.insert(loc);
			}
		}

		// 4. remove unused store
		bool keep = true;
		if (inst.is_store())
		{
			Operand dest = inst.get_dest();
			if (dest.type == OperandType::MEMORY)
			{
				pair<string, int> loc = {dest.base_reg, dest.displacement};
				if (used_memory.find(loc) == used_memory.end())
				{
					// this store not be used, remove
					keep = false;
				}
			}
		}
		if (keep)
		{
			new_instructions.push_back(inst);
		}
	}
	block.instructions = new_instructions;
}
