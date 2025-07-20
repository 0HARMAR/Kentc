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

	// load dividend to %eax
	string dividendLoc = registerAllocator.getTempVarLocation(dividend);
	if (dividendLoc != "%eax")
	{
		// TODO generate asm code to load dividend to %eax
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
	// TODO generate asm code to divide

	// free op reg
	registerAllocator.freeReg(dividend);
	registerAllocator.freeReg(divisor);
}
