//
// Created by hemin on 25-8-7.
//
#include <format>

#include "TargetGenerator.h"

void TargetGenerator::processGetElementPtr(const std::vector<std::string>& args)
{
	string resultTempReg = args[0];
	string stringName = args[1];

	string resultRealReg = registerAllocator.allocReg(resultTempReg);
	asmWriter.lea(stringName + "(%rip)", resultRealReg, "q");
}
