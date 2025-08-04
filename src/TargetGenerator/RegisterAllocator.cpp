//
// Created by hemin on 25-7-19.
//

#include "RegisterAllocator.h"
#include "CallingConvention.h"

RegisterAllocator::RegisterAllocator(AsmWriter& asmWriter) : asmWriter(asmWriter) {}

// alloc register for immediate
string RegisterAllocator::allocReg(vector<string> exclude)
{
	string reg = freeRegs.back();
	auto it = find_if(freeRegs.begin(), freeRegs.end(), [&](const auto& r)
			{
				return find(exclude.begin(), exclude.end(), r) == exclude.end();
			});

	if (it != freeRegs.end())
	{
		reg = *it;
		freeRegs.erase(it);
	} else
	{
		reg = "";
	}
	if (reg != "")freeRegs.pop_back();
	return spillVar();
}

string RegisterAllocator::allocReg(string var, vector<string> exclude)
{
	if (!freeRegs.empty())
	{
		string reg = freeRegs.back();
		// no exclude or not exclude register, use it directly
		if (exclude.empty() || find(exclude.begin(), exclude.end(), reg) == exclude.end())
		{
			freeRegs.pop_back();
		} else
		{
			auto it = find_if(freeRegs.begin(), freeRegs.end(), [&](const auto& r)
			{
				return find(exclude.begin(), exclude.end(), r) == exclude.end();
			});

			if (it != freeRegs.end())
			{
				reg = *it;
				freeRegs.erase(it);
			} else
			{
				reg = "";
			}
		}
		if (reg != "")
		{
			varToReg[var] = reg;
			return reg;
		}
	}
	return spillVar(var);
}

// if free register null, put temp var to stack
string RegisterAllocator::spillVar(string var)
{
	int spillSlot = stackOffset - 4;
	if (spillSlot < stackSize) extandStack();
	stackOffset -= 4;
	spilledVars[var] = spillSlot;

	return std::to_string(stackOffset) + "(%rbp)";
}

// if free register null, put immediate to stack
string RegisterAllocator::spillVar()
{
	int spillSlot = stackOffset - 4;
	if (spillSlot < stackSize) extandStack();
	stackOffset -= 4;

	return std::to_string(stackOffset) + "(%rbp)";
}
// choose first var
string RegisterAllocator::selectSpillCandidate()
{
	for (auto& pair : varToReg)
	{
		if (find(CallingConvention::callerSaved.begin(), CallingConvention::callerSaved.end(),
			pair.second) != CallingConvention::callerSaved.end())
		{
			return pair.first;
		}
	}
	return varToReg.begin()->first;
}

string RegisterAllocator::getTempVarLocation(string operand)
{
	if (operand[0] == '%')
	{
		if (varToReg.count(operand)) return varToReg[operand];
		if (spilledVars.count(operand)) return to_string(spilledVars[operand]) + "(%rbp)";

	}
	return "";
}

void RegisterAllocator::handleCall()
{
	for (auto& pair : varToReg)
	{
		if (find(CallingConvention::callerSaved.begin(), CallingConvention::callerSaved.end(),
			pair.second) != CallingConvention::callerSaved.end())
		{
			// TODO generate asm code to save the register

			spilledVars[pair.first] = stackOffset - 4;
			stackOffset -= 4;
		}
	}
}

void RegisterAllocator::freeReg(string var)
{
	if (varToReg.count(var))
	{
		string reg = varToReg[var];
		freeRegs.push_back(reg);
		varToReg.erase(var);
	}
}

string RegisterAllocator::getVarInRegister(string reg)
{
	for (const auto& pair : varToReg)
	{
		if (pair.second == reg) return pair.first;
	}
	return "";
}

// if var bind a reg, cancel the bind, and alloca a new reg for it,
// return the new reg
string RegisterAllocator::spillRegister(string var)
{
	vector<string> exclude;
	if (varToReg.count(var))
	{
		string reg = varToReg[var];
		freeRegs.push_back(reg);
		varToReg.erase(var);
		exclude.push_back(reg);
	}

	return allocReg(var, exclude);
}
// binding a var to reg, if var is binding, return old reg
string RegisterAllocator::allocReg(string var, string reg)
{
	if (varToReg.count(var))
	{
		string old_reg = getTempVarLocation(var);
		// free var binding reg
		freeReg(var);

		// remove reg form freeRegs
		freeRegs.erase(std::remove(freeRegs.begin(), freeRegs.end(), reg), freeRegs.end());

		varToReg[var] = reg;
		return old_reg;

	} else
	{
		// remove reg form freeRegs
		freeRegs.erase(remove(freeRegs.begin(), freeRegs.end(), reg), freeRegs.end());

		varToReg[var] = reg;
		return "";
	}
}

void RegisterAllocator::extandStack()
{
	// TODO calculate need stack size
	stackSize -= 16;
	asmWriter.sub("$16", "%rsp", "q");
}

bool RegisterAllocator::haveFreeReg()
{
	if (freeRegs.size() == 0) return false;
	return true;
}

string RegisterAllocator::getRandomTransitReg()
{
	static vector<string> Regs = {"%rdi", "%rsi", "%rax", "%rbx", "%rcx", "%rdx",
		"%r8","%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
	static string lastReg;

	static bool seedInitialized = false;
	if (!seedInitialized)
	{
		srand(time(nullptr));
		seedInitialized = true;
	}

	string newReg;
	do
	{
		string key = "harmar";
		int salt = rand();
		string salted = key + to_string(salt);
		size_t index = hash<string>{}(salted) % Regs.size();
		newReg = Regs[index];

		if (lastReg.empty() || Regs.size() == 1) break;
	} while (newReg == lastReg);

	lastReg = newReg;
	return newReg;

}

