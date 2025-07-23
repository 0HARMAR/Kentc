#include "TargetGenerator.h"

/**
 *
 * @param dividend e.g., "%t1", $3
 * @param divisor e.g., "%t2", $1
 * @param result result register
 */
void TargetGenerator::handleDivision(const string& dividend, const string& divisor, string& result)
{
	// force use %eax for dividend
	string eaxUser = registerAllocator.getVarInRegister(normalizeReg("%eax"));
	// %eax is used
	if (!eaxUser.empty())
	{
		if (eaxUser != dividend)
		{
			// make %eax free
			string new_reg = registerAllocator.spillRegister(eaxUser);
			addAsmLine("	movl	%eax, " + new_reg);

			// make dividend binding to %eax, free old reg
			if (dividend[0] == '$') // dividend is an immediate
			{
				addAsmLine("	movl	" + dividend + ", " + "%eax");
			} else // dividend is a temp var
			{
				string old_reg = registerAllocator.allocReg(dividend, normalizeReg("%eax"));
				addAsmLine("	movl	" + old_reg + ", %eax");
			}

		}
	} else // %eax is free
	{
		// make dividend binding to %eax, free old reg
		if (dividend[0] == '$') // dividend is an immediate
		{
			addAsmLine("	movl	" + dividend + ", " + "%eax");
		} else // dividend is a temp var
		{
			string old_reg = registerAllocator.allocReg(dividend, normalizeReg("%eax"));
			addAsmLine("	movl	" + old_reg + ", %eax");
		}
	}

	// %edx for result extend, force make %edx free
	string edxUser = registerAllocator.getVarInRegister(normalizeReg("%edx"));
	if (!edxUser.empty())
	{
		registerAllocator.spillRegister(edxUser);
	}

	// make sure divisor in register
	string divisorReg;
	if (divisor[0] == '%') {} // temp var
	else  // immediate value
	{
		divisorReg = registerAllocator.allocReg();
		addAsmLine("	movl	" + divisor + ", " + divisorReg);
	}

	// generate divide instruction
	addAsmLine("	cltd"); // sign extend %eax to %edx
	addAsmLine("	idivl	" + divisorReg);

	string resultReg = registerAllocator.allocReg(result);
	addAsmLine("	movl	%eax, " + resultReg); // move result to result reg

	// free op reg
	registerAllocator.freeReg(dividend);
	registerAllocator.freeReg(divisor);
}

string TargetGenerator::normalizeReg(string reg)
{
	string noBitPrefixReg = reg.substr(2);
	return "r" + noBitPrefixReg;
}
