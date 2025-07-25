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
	addAsmLine("	movl	$" + addr + ", %esi");
	addAsmLine("	call	" + tokens[0]);
	addAsmLine(CallingConvention::restoreCallerSave());

	// process return value
	string returnReg = tokens[2];
	vector<string> exclude;
	exclude.push_back("%rax");
	string realReg = registerAllocator.allocReg(returnReg, exclude);
	addAsmLine("	movl	%eax, " + denormalizeReg(realReg, 32));
}
