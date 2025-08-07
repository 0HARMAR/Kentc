#include "TargetGenerator.h"

void TargetGenerator::handlePrintInt(vector<string> tokens)
{
	// first arg
	string arg = tokens[4];

	addAsmLine(CallingConvention::addCallerSave());
	if (arg.find('%') != string::npos) // temp register
	{
		string reg = registerAllocator.getTempVarLocation(arg);
		addAsmLine("	movl	" + denormalizeReg(reg, 32) + ", %edi");
	}
	addAsmLine("	call	" + tokens[0]);
	addAsmLine(CallingConvention::restoreCallerSave());
}

void TargetGenerator::handlePrintString(vector<string> tokens)
{
	string stringAddressTempReg = tokens[4];
	string stringLength = tokens[6];

	addAsmLine(CallingConvention::addCallerSave());
	string stringAddressRealReg = registerAllocator.getTempVarLocation(stringAddressTempReg);
	asmWriter.mov(stringAddressRealReg, "%rdi", "q");
	asmWriter.mov("$" + stringLength, "%esi", "l");
	asmWriter.call(tokens[0]);
	addAsmLine(CallingConvention::restoreCallerSave());
}

void TargetGenerator::handleExit(vector<string> tokens)
{
	// first arg
	string arg = tokens[4];
	addAsmLine(CallingConvention::addCallerSave());
	if (arg.find('%') != string::npos) // temp register
	{
		string reg = registerAllocator.getTempVarLocation(arg);
		addAsmLine("	movl	" + denormalizeReg(reg, 32) + ", %edi");
	} else // immediate value
	{
		addAsmLine("	movl	$" + arg + ", %edi");
	}
	addAsmLine("	call	" + tokens[0]);
	addAsmLine(CallingConvention::restoreCallerSave());
}

void TargetGenerator::handleMallocAt(vector<string> tokens)
{
	string size = tokens[4];
	string addr = tokens[6];

	addAsmLine(CallingConvention::addCallerSave());
	addAsmLine("	movl	$" + size + ", %edi");
	addAsmLine("	movl	$0x" + addr + ", %esi");
	addAsmLine("	call	" + tokens[0]);

	// process return value
	string returnReg = tokens[2];
	vector<string> exclude;

	// save return value, but not the caller save register,
	// because they will be restore then.
	for (auto& ex: CallingConvention::callerSaved)
	{
		exclude.push_back("%" + ex);
	}
	string realReg = registerAllocator.allocReg(returnReg, exclude);
	addAsmLine("	movl	%eax, " + denormalizeReg(realReg, 32));
	addAsmLine(CallingConvention::restoreCallerSave());
}

void TargetGenerator::handleIn(vector<string> tokens)
{
	string inBytesNum = tokens[4];
	string inAddress = tokens[6];
	int inAddressInt = stoi(inAddress);
	string realAddress = std::to_string(reinterpret_cast<uintptr_t>(malloc_at(1, inAddressInt)));

	addAsmLine(CallingConvention::addCallerSave());
	asmWriter.mov("$" + inBytesNum, "%edi", "l");
	asmWriter.mov("$" + realAddress, "%esi", "l");
	asmWriter.call(tokens[0]);
	addAsmLine(CallingConvention::restoreCallerSave());
}

