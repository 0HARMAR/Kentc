//
// Created by hemin on 25-8-8.
//
#include "../TargetGenerator.h"

void TargetGenerator::processXor(const vector<string>& tokens)
{
	string resultTempReg = tokens[0];
	string op1 = tokens[1];
	string op2 = tokens[2];

	string resultRealReg = registerAllocator.allocReg(resultTempReg, "i64");
	if (op1 + op2 == "00") // reset a register
	asmWriter.Xor(resultRealReg, resultRealReg, "q");
}
