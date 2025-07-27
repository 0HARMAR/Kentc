//
// Created by hemin on 25-7-16.
//

#include "TargetGenerator.h"

#include <complex>

TargetGenerator::TargetGenerator() : asmLines(), asmWriter(asmLines), registerAllocator(this->asmWriter){}

string TargetGenerator::trim(const string& str)
{
	size_t first = str.find_first_not_of(' \t');
	if (string::npos == first) return "";
	size_t last = str.find_last_not_of(' \t');
	return str.substr(first, (last - first + 1));
}

void TargetGenerator::addAsmLine(const string& line)
{
	asmLines.push_back(line);
}

void TargetGenerator::addAsmLine(const vector<string>& lines)
{
	for (const auto& line : lines)
	{
		addAsmLine(line);
	}
}


string TargetGenerator::createTempVar()
{
	return "%t" + to_string(nextTempVar++);
}

void TargetGenerator::processAlloca(const vector<string>& tokens)
{
	if (tokens.size() != 2) return ;

	string varName = trim(tokens[0]);
	string type = trim(tokens[1]);

	// allocate stack space for the variable
	int size = variableMap.count(type) ? variableMap[type] : 4;
	stackSize += size;
	maxStackOffset = stackSize;

	// record var info
	variables[varName] = {varName, type, -stackSize};
}

void TargetGenerator::processStore(const vector<string>& tokens)
{
	if (tokens.size() != 3) return ;

	// pattern : store i32 [storeValue], i32* [storeAddr]
	// e.g. store i32 5, i32* %t1
	string value = trim(tokens[0]);
	value.pop_back();
	string type = trim(tokens[1]);
	string destVar = trim(tokens[2]);

	// e.g. storeAddr : (%rax), (-8(%rbp))(not allow in syntax)
	string storeAddr;
	string spilledTransitReg = "";
	if (destVar[0] == '%') // addr store in a temp register
	{
		storeAddr = registerAllocator.getTempVarLocation(destVar);
		if (isMemory(storeAddr))
		{
			spilledTransitReg = registerAllocator.getRandomTransitReg();
			asmWriter.push(spilledTransitReg);
			asmWriter.mov(storeAddr, spilledTransitReg, "q");
			storeAddr = "(" + spilledTransitReg + ")";
		} else storeAddr = "(" + storeAddr + ")";
	}

	// e.g. storeValue : $0x5, %rax, -8(%rbp)
	string storeValue;
	if (value[0] == '%') // value is a temp register
	{
		storeValue = registerAllocator.getTempVarLocation(value);
	} else // value is an immediate
	{
		storeValue = "$0x" + value;
	}

	// not allow mov mem to mem
	if (isMemory(storeValue))
	{
		string spilledTransitReg = registerAllocator.getRandomTransitReg();
		asmWriter.push(spilledTransitReg);
		asmWriter.mov(storeValue, spilledTransitReg, "q");
		asmWriter.mov(spilledTransitReg, storeAddr, "q");
		asmWriter.pop(spilledTransitReg);
		return;
	}

	asmWriter.mov(storeValue, storeAddr, "q");
	if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
}

string TargetGenerator::processLoad(const vector<string>& tokens)
{
	if (tokens.size() != 2) return "";

	// pattern : [loadValue] = load i32, i32* [loadAddr]
	// e.g. %t1 = load i32, i32* %b
	string destVar = trim(tokens[0]);
	string srcAddr = trim(tokens[1]);

	// e.g. loadAddr : (%rax), (-8(%rbp))(not allow in syntax)
	string loadAddr = registerAllocator.getTempVarLocation(srcAddr);
	loadAddr = "(" + loadAddr + ")";

	// e.g. loadValue : %rbx
	string loadValue = registerAllocator.allocReg(destVar);

	// not allow mov mem to mem
	if (isMemory(loadValue))
	{
		string spilledTransitReg = registerAllocator.getRandomTransitReg();
		asmWriter.push(spilledTransitReg);
		asmWriter.mov(loadAddr, spilledTransitReg, "q");
		asmWriter.mov(spilledTransitReg, loadValue, "q");
		asmWriter.pop(spilledTransitReg);
		return "";
	}

	asmWriter.mov(loadAddr, loadValue, "q");
	return "";
}

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
		string resultReg = registerAllocator.allocReg(result);
		if (src1[0] != '$')
		{
			if (isMemory(src1) && isMemory(resultReg))
			{
				asmWriter.movMemToMem(src1, resultReg, registerAllocator.getRandomTransitReg(), "q");
			} else
			{
				addAsmLine("	movq	" + normalizeReg(src1) + ", " + resultReg);
			}
		}
		else addAsmLine("	movq	" + src1 + ", " + resultReg);
		addAsmLine("	" + opMap[op] + "	" + src2 + ", " + denormalizeReg(resultReg, 32));
	}

	return "";
}

void TargetGenerator::processCall(const vector<string>& tokens)
{
	if (tokens.empty()) return;

	string funcName = tokens[0];
	string returnType = tokens[1];
	string returnReg = tokens[2];
	map<string, string> argList; // name -> type mapping

	for (int i = 3; i < tokens.size(); i += 2)
	{
		if (i + 1 >= tokens.size()) break;
		string argType = tokens[i];
		string argName = tokens[i + 1];
		argList[argName] = argType;
	}

	if (funcName == "print_int") {
		handlePrintInt(tokens);
	} else if (funcName == "exit") {
		handleExit(tokens);
	} else if (funcName == "malloc") {
	} else if (funcName == "free") {
	} else if (funcName == "malloc_at") {
		handleMallocAt(tokens);
	}

}

void TargetGenerator::processInttoptr(const vector<string>& tokens)
{
	string convertToReg = tokens[0];
	string convertValue = tokens[1];

	stringstream ss;
	void* addr = malloc_at(4, stoul(convertValue));
	uintptr_t intAddr = reinterpret_cast<uintptr_t>(addr);
	ss << hex << intAddr;
	string realAddr = ss.str();

	string Reg = registerAllocator.allocReg(convertToReg);
	addAsmLine("	movl	$0x" + realAddr + ", " + denormalizeReg(Reg, 32));
}


