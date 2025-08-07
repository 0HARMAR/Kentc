//
// Created by hemin on 25-7-16.
// im tired..., i want die
// hi,intel, i want you launch a new chip that
// support mem to mem mov, no some reason, because i
// actually need it... i must own it... it is my love...
//

#include "TargetGenerator.h"
#include <complex>

TargetGenerator::TargetGenerator() : asmLines(), asmWriter(asmLines), registerAllocator(this->asmWriter){}

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
	string type_str = trim(tokens[1]);
	int typeSize = stoi(type_str.substr(1, type_str.size() - 1));
	string destVar = trim(tokens[2]);

	// e.g. storeAddr : (%rax), (-8(%rbp))(not allow in syntax)
	string storeAddr;
	string spilledTransitReg = "";
	if (destVar[0] == '%') // addr store in a temp register
	{
		storeAddr = registerAllocator.getTempVarLocation(destVar);
		// memory access addr must 32 bit least
		if (isMemory(storeAddr))
		{
			spilledTransitReg = registerAllocator.getRandomTransitReg();
			asmWriter.push(spilledTransitReg);
			asmWriter.mov(storeAddr, denormalizeReg(spilledTransitReg, 32), "l");
			storeAddr = "(" + denormalizeReg(spilledTransitReg, 32) + ")";
		} else storeAddr = "(" + denormalizeReg(storeAddr, 32) + ")"; // 32 bit addr is enough
	}

	// e.g. storeValue : $0x5, %rax, -8(%rbp)
	string storeValue;
	if (value[0] == '%') // value is a temp register
	{
		storeValue = registerAllocator.getTempVarLocation(value);
		if (storeValue[0] == '%') storeValue = denormalizeReg(storeValue, typeSize);
	} else // value is an immediate
	{
		storeValue = "$" + value;
	}

	// not allow mov mem to mem
	if (isMemory(storeValue))
	{
		string spilledTransitReg_ = registerAllocator.getRandomTransitReg();
		spilledTransitReg_ = denormalizeReg(spilledTransitReg_, 32);
		asmWriter.push(normalizeReg(spilledTransitReg_)); // must push 64 bit for align
		asmWriter.mov(storeValue, formatReg(spilledTransitReg_, typeSize), bitWideToMovSubfix[typeSize]);
		asmWriter.mov(spilledTransitReg_, storeAddr, "l");
		asmWriter.pop(normalizeReg(spilledTransitReg_)); // must pop 64 bit
		if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
		return;
	}

	asmWriter.mov(storeValue, storeAddr, "l");
	if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
}

string TargetGenerator::processLoad(const vector<string>& tokens)
{
	if (tokens.size() != 3) return "";

	// pattern : [loadValue] = load i32, i32* [loadAddr]
	// e.g. %t1 = load i32, i32* %b
	string destVar = trim(tokens[0]);
	int typeSize = stoi(trim(tokens[1]).substr(1, 2));
	string srcAddr = trim(tokens[2]);

	// e.g. loadValue : %rbx, -8(%rbp)
	// first process loadValue, because it may
	// involve stack extend and sub rsp
	// thus influence push/pop sequence
	string loadValue = registerAllocator.allocReg(destVar);
	if (loadValue[0] == '%') loadValue = denormalizeReg(loadValue, typeSize);

	// e.g. loadAddr : (%rax), (-8(%rbp))(not allow in syntax)
	string loadAddr = registerAllocator.getTempVarLocation(srcAddr);
	string spilledTransitReg = "";
	if (isMemory(loadAddr))
	{
		spilledTransitReg = registerAllocator.getRandomTransitReg();
		asmWriter.push(spilledTransitReg);
		asmWriter.mov(loadAddr, denormalizeReg(spilledTransitReg, 32), "l");
		loadAddr = "(" + denormalizeReg(spilledTransitReg, 32) + ")";
	} else loadAddr = "(" + denormalizeReg(loadAddr, 32) + ")";

	// not allow mov mem to mem
	if (isMemory(loadValue))
	{
		string spilledTransitReg_ = registerAllocator.getRandomTransitReg();
		asmWriter.push(spilledTransitReg_);
		asmWriter.mov(loadAddr, denormalizeReg(spilledTransitReg_, 32), "l");
		asmWriter.mov(denormalizeReg(spilledTransitReg_, typeSize), loadValue, bitWideToMovSubfix[typeSize]);
		asmWriter.pop(spilledTransitReg_);
		if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
		return "";
	}

	asmWriter.mov(loadAddr, formatReg(loadValue, typeSize), bitWideToMovSubfix[typeSize]);
	if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
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
				asmWriter.movMemToMem(src1, denormalizeReg(resultReg, 32),
					denormalizeReg(registerAllocator.getRandomTransitReg(), 32), "l");
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
	} else if (funcName == "in")
	{
		handleIn(tokens);
	} else if (funcName == "print_string")
	{
		handlePrintString(tokens);
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

void TargetGenerator::processIcmp(const vector<string>& tokens)
{
	string resultReg = tokens[0];
	string op = tokens[1];
	string op1Type = tokens[2];
	string op1 = tokens[3];
	string op2 = tokens[4];

	string op1Reg = registerAllocator.getTempVarLocation(op1);
	asmWriter.cmp("$" + op2, denormalizeReg(op1Reg, 32), "l");
	string cmpResult = registerAllocator.allocReg(resultReg);
	asmWriter.set(denormalizeReg(cmpResult, 8), "e");
}

void TargetGenerator::processBr(const vector<string>& tokens)
{
	string cmpResult = tokens[0];
	string cmpResultReg = registerAllocator.getTempVarLocation(cmpResult);
	string trueLabel = tokens[1];
	string continueLabel = tokens[2];
	asmWriter.cmp("$1", denormalizeReg(cmpResultReg, 8), "b");
	asmWriter.jmp(continueLabel, "ne");
}


