//
// Created by hemin on 25-7-16.
// im tired..., i want die
// hi,intel, i want you launch a new chip that
// support mem to mem mov, no some reason, because i
// actually need it... i must own it... it is my love...
//

#include "TargetGenerator.h"
#include <complex>
#include <bits/atomic_base.h>

extern bool isMain;
TargetGenerator::TargetGenerator()
: stackSize(-256),
stackOffset({}),
asmLines(), asmWriter(asmLines),
registerAllocator(asmWriter, stackSize, stackOffset, currentFunction){}

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

	string allocaTempReg = trim(tokens[0]);
	string type = trim(tokens[1]);

	string allocaRealReg = registerAllocator.allocReg(allocaTempReg, "i64", vector<string>{});
	string size = std::to_string(variableMap[type]);
	if (stackOffset[currentFunction] - stoi(size) < stackSize)
	{
		registerAllocator.extandStack(isMain);
	}
	if (isMemory(allocaRealReg))
	{
		string transitReg = registerAllocator.getRandomTransitReg();
		asmWriter.push(transitReg);
		asmWriter.lea( std::to_string((stackOffset[currentFunction] - stoi(size))) + "(%rbp)", transitReg, "q");
		asmWriter.mov(transitReg, allocaRealReg, "q");
		asmWriter.pop(transitReg);
	}
	else asmWriter.lea( std::to_string((stackOffset[currentFunction] - stoi(size))) + "(%rbp)", allocaRealReg, "q");
	stackOffset[currentFunction] -= stoi(size);

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
		// memory access addr must 64 bit
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
		if (storeValue[0] == '%') storeValue = denormalizeReg(storeValue, typeSize);
	} else // value is an immediate
	{
		storeValue = "$" + value;
	}

	// not allow mov mem to mem
	if (isMemory(storeValue))
	{
		string spilledTransitReg_ = registerAllocator.getRandomTransitReg
		(storeAddr.substr(1, storeAddr.size() - 2)); // remove '(' and ')'
		asmWriter.push(spilledTransitReg_); // must push 64 bit for align
		asmWriter.mov(storeValue, formatReg(spilledTransitReg_, typeSize), bitWideToMovSubfix[typeSize]);
		asmWriter.mov(formatReg(spilledTransitReg_, typeSize), storeAddr, bitWideToMovSubfix[typeSize]);
		asmWriter.pop(spilledTransitReg_); // must pop 64 bit
		if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
		return;
	}

	asmWriter.mov(storeValue, storeAddr, bitWideToMovSubfix[typeSize]);
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
	string loadValue = registerAllocator.allocReg(destVar, "i" + std::to_string((typeSize)), vector<string>{});
	if (loadValue[0] == '%') loadValue = denormalizeReg(loadValue, typeSize);

	// e.g. loadAddr : (%rax), (-8(%rbp))(not allow in syntax)
	string loadAddr = registerAllocator.getTempVarLocation(srcAddr);
	string spilledTransitReg = "";
	if (isMemory(loadAddr))
	{
		spilledTransitReg = registerAllocator.getRandomTransitReg();
		asmWriter.push(spilledTransitReg);
		asmWriter.mov(loadAddr, spilledTransitReg, "q");
		loadAddr = "(" + spilledTransitReg + ")";
	} else loadAddr = "(" + loadAddr + ")";

	// not allow mov mem to mem
	if (isMemory(loadValue))
	{
		string spilledTransitReg_ = registerAllocator.getRandomTransitReg();
		asmWriter.push(spilledTransitReg_);
		asmWriter.mov(loadAddr, formatReg(spilledTransitReg_, typeSize), bitWideToMovSubfix[typeSize]);
		asmWriter.mov(denormalizeReg(spilledTransitReg_, typeSize), loadValue, bitWideToMovSubfix[typeSize]);
		asmWriter.pop(spilledTransitReg_);
		if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
		return "";
	}

	asmWriter.mov(loadAddr, formatReg(loadValue, typeSize), bitWideToMovSubfix[typeSize]);
	if (!spilledTransitReg.empty()) asmWriter.pop(spilledTransitReg);
	return "";
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

	string Reg = registerAllocator.allocReg(convertToReg, "i32", vector<string>{});
	addAsmLine("	movl	$0x" + realAddr + ", " + denormalizeReg(Reg, 32));
}

void TargetGenerator::processIcmp(const vector<string>& tokens)
{
	string resultReg = tokens[0];
	string op = tokens[1];
	string op1Type = tokens[2];
	int typeSize = variableMap[op1Type];
	string typeSuffix = bitWideToMovSubfix[typeSize * 8];
	string op1 = tokens[3];
	string op2 = tokens[4];

	string op1Reg = registerAllocator.getTempVarLocation(op1);
	asmWriter.cmp("$" + op2, denormalizeReg(op1Reg, typeSize * 8), typeSuffix);
	string cmpResult = registerAllocator.allocReg(resultReg, "i1", vector<string>{});
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


