//
// Created by hemin on 25-8-7.
//
#include <format>

#include "../TargetGenerator.h"

void TargetGenerator::processGetElementPtr(const std::vector<std::string>& args)
{
	string resultTempReg = args[0];
	string stringName = args[1];

	string resultRealReg = registerAllocator.allocReg(resultTempReg, "i64");
	if (isMemory(resultRealReg))
	{
		string transitTempReg = registerAllocator.getRandomTransitReg();
		asmWriter.push(transitTempReg);
		asmWriter.lea(stringName + "(%rip)", transitTempReg, "q");
		asmWriter.mov(transitTempReg, resultRealReg, "q");
		asmWriter.pop(transitTempReg);
	} else
	asmWriter.lea(stringName + "(%rip)", resultRealReg, "q");
}
