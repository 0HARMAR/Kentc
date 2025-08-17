//
// Created by hemin on 25-8-11.
//
#include "../TargetGenerator.h"

string TargetGenerator::processBinaryOp(const vector<string>& tokens)
{
	if (tokens.size() != 5) return "";

	string result = tokens[0];
	string op = tokens[1];
	string type = tokens[2];
	string op1 = tokens[3];
	op1.pop_back();
	string op2 = tokens[4];

	// process operand
	string src1,src2;
	if (op1.find('%') != string::npos)
	{
		src1 = registerAllocator.getTempVarLocation(op1);
		src1 = denormalizeReg(src1, 32);
	} else
	{
		src1 = "$" + op1;
	}

	if (op2.find('%') != string::npos)
	{
		src2 = registerAllocator.getTempVarLocation(op2);
		src2 = denormalizeReg(src2, 32);
	} else
	{
		src2 = "$" + op2;
	}

	// process div
	if (op == "sdiv")
	{
		handleDivision(src1, src2, result);
	}

	// process other ops
	if (opMap.find(op) != opMap.end() && op != "sdiv")
	{
		string resultReg = registerAllocator.allocReg(result, "i32");
		if (src1[0] != '$')
		{
			if (isMemory(src1) && isMemory(resultReg))
			{
				asmWriter.movMemToMem(src1, denormalizeReg(resultReg, 32),
					denormalizeReg(registerAllocator.getRandomTransitReg(), 32), 32);
			} else
			{
				addAsmLine("	movl	" + src1 + ", " + denormalizeReg(resultReg, 32));
			}
		}
		else addAsmLine("	movl	" + src1 + ", " + denormalizeReg(resultReg, 32));
		if (op == "mul" and isMemory(resultReg)) // mul dest could not be memory
		{
			string spilledTransitReg = registerAllocator.getRandomTransitReg();
			asmWriter.push(spilledTransitReg);
			asmWriter.mov(denormalizeReg(resultReg, 32), denormalizeReg(spilledTransitReg, 32), "l");
			asmWriter.mul(src2, denormalizeReg(spilledTransitReg, 32), "l");
			asmWriter.mov(denormalizeReg(spilledTransitReg, 32), denormalizeReg(resultReg, 32), "l");
			asmWriter.pop(spilledTransitReg);
			return "";
		}
		if (isMemory(src2) && isMemory(resultReg)) // two operand both mem? no,no
			// let us make fuck transit
		{
			string spilledTransitReg = registerAllocator.getRandomTransitReg();
			asmWriter.push(spilledTransitReg);
			asmWriter.mov(denormalizeReg(resultReg, 32), denormalizeReg(spilledTransitReg, 32), "l");
			if (op == "add") asmWriter.add(src2, denormalizeReg(spilledTransitReg, 32),  "l");
			else if (op == "sub") asmWriter.sub(src2, denormalizeReg(spilledTransitReg, 32), "l");
			asmWriter.mov(denormalizeReg(spilledTransitReg, 32), denormalizeReg(resultReg, 32), "l");
			asmWriter.pop(spilledTransitReg);
			return "";
		}
		addAsmLine("\t" + opMap[op] + "	" + src2 + ", " + denormalizeReg(resultReg, 32));
	}

	return "";
}
