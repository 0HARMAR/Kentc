#include "TargetGenerator.h"

void TargetGenerator::handleDivision(const string& dividend, const string& divisor, string& result)
{
	// force use %eax for dividend
	string eaxUser = registerAllocator.getVarInRegister("%eax");
	if (!eaxUser.empty())
	{
		if (eaxUser != dividend) registerAllocator.spillRegister(eaxUser);
	}

	// force use %edx for result extend
	string edxUser = registerAllocator.getVarInRegister("%edx");
	if (!edxUser.empty())
	{
		registerAllocator.spillRegister(edxUser);
	}

	// load divisor to another reg(not %edx）
	string divisorRegOrNum;
	if (divisor[0] == '%')
	{
		divisorRegOrNum = registerAllocator.getTempVarLocation(divisor);
		// divisor in eax or edx and be spilled, realloca it
		if (divisorRegOrNum == "")
		{
			registerAllocator.allocReg(divisor);
		}
	} else
	{
		divisorRegOrNum = "$" + divisor;
	}

	// generate divide instruction
	addAsmLine("	cltd"); // sign extend %eax to %edx
	addAsmLine("	idivl " + divisorRegOrNum);

	string resultReg = registerAllocator.allocReg(result);
	addAsmLine("	movl %eax, " + resultReg); // move result to result reg

	// free op reg
	registerAllocator.freeReg(dividend);
	registerAllocator.freeReg(divisor);
}
