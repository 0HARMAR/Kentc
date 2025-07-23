//
// Created by hemin on 25-7-19.
//

#include "RegisterAllocator.h"
#include "CallingConvention.h"

// alloc register for immediate
string RegisterAllocator::allocReg()
{
	string reg = freeRegs.back();
	freeRegs.pop_back();
	return reg;
}

string RegisterAllocator::allocReg(string var)
{
	if (!freeRegs.empty())
	{
		string reg = freeRegs.back();
		freeRegs.pop_back();
		varToReg[var] = reg;
		return reg;
	}
	return spillVar(var);
}

string RegisterAllocator::spillVar(string var)
{
	string spillTarget = selectSpillCandidate();
	int spillSlot = stackOffset - 4;
	stackOffset -= 4;
	spilledVars[spillTarget] = spillSlot;

	// TODO generate asm code to spill the variable

	freeReg(spillTarget); // free the register used by the spilled variable
	return allocReg(var); // realloc
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

void RegisterAllocator::freeReg(string var, bool lastUse)
{
	if (varToReg.count(var))
	{
		string reg = varToReg[var];
		freeRegs.push_back(reg);
		varToReg.erase(var);
		//
		if (!lastUse && spilledVars.count(var))
		{
			// TODO generate asm code to reload the variable
		}
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
	if (varToReg.count(var))
	{
		string reg = varToReg[var];
		freeRegs.push_back(reg);
	}

	return allocReg(var);
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
