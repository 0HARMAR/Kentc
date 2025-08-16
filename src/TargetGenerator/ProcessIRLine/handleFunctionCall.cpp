#include "../TargetGenerator.h"

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

void TargetGenerator::handlePrintHex(vector<string> tokens)
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
	string realReg = registerAllocator.allocReg(returnReg, "i64", exclude);
	addAsmLine("	movq	%rax, " + realReg);
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

void TargetGenerator::handleCall(Function_ function, string resultReg)
{
	string name = function.name;
	string returnType = function.returnType;
	vector<Variable> arguments = function.parameters;

	vector<string> realArgs;
	for (auto arg : arguments)
	{
		if (arg.name[0] == '%')
		{
			string realReg = registerAllocator.getTempVarLocation(arg.name);
			realArgs.push_back(realReg);
		} else
		{
			realArgs.push_back("$" + arg.name);
		}
	}

	addAsmLine(CallingConvention::addCallerSave());
	for (int i = 0; i < arguments.size(); i++)
	{
		string type = arguments[i].type;
		int typeSize = variableMap[type];
		if (realArgs[i][0] == '%')
		{
			asmWriter.mov(formatReg(realArgs[i], typeSize), formatReg('%' + CallingConvention::parameters[i], typeSize),
				bitWideToMovSubfix[typeSize]);
		} else
		{
			asmWriter.mov(realArgs[i], formatReg('%' + CallingConvention::parameters[i], typeSize),
				bitWideToMovSubfix[typeSize]);
		}
	}
	asmWriter.call(name);

	vector<string> exclude;

	// save return value, but not the caller save register,
	// because they will be restore then.
	for (auto& ex: CallingConvention::callerSaved)
	{
		exclude.push_back("%" + ex);
	}
	string returnReg = registerAllocator.allocReg(resultReg, "i32", exclude);
	asmWriter.mov("%eax", formatReg(returnReg, 32), "l");
	addAsmLine(CallingConvention::restoreCallerSave());
}


