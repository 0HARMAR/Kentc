//
// Created by hemin on 25-7-19.
//

#include "RegisterAllocator.h"

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
		if (find(callerSaved.begin(), callerSaved.end(), pair.second) != callerSaved.end())
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
	return operand;
}

void RegisterAllocator::handleCall()
{
	for (auto& pair : varToReg)
	{
		if (find(callerSaved.begin(), callerSaved.end(), pair.second) != callerSaved.end())
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

void RegisterAllocator::handleDivision(const string& dividend, const string& divisor, string& result)
{

}