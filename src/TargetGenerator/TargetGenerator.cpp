//
// Created by hemin on 25-7-16.
//

#include "TargetGenerator.h"

#include <complex>

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

	// analyze : store i32 5, i32* %mem512
	string value = trim(tokens[0]);
	value.pop_back();
	string type = trim(tokens[1]);
	string destVar = trim(tokens[2]);

	string varAddrReg;
	if (destVar[0] == '%') // var addr store in a register
	{
		varAddrReg = registerAllocator.getTempVarLocation(destVar);
	}

	// if prefix no %, it`s an immediate operand
	if (value.find('%') == string::npos)
	{
			addAsmLine("	movq	$" + value + ", (" + varAddrReg + ")");
	}
	// temp var
	else
	{
		if (variables.find(destVar) != variables.end())
		{
			string varToReg = registerAllocator.getTempVarLocation(value);
			addAsmLine("	movq	" + varToReg + ", " + to_string(variables[destVar].stackOffset) + "(%rbp)");
		}
	}
}

string TargetGenerator::processLoad(const vector<string>& tokens)
{
	if (tokens.size() != 2) return "";

	// analyze : %t1 = load i32, i32* %mem512
	string destVar = trim(tokens[0]);
	string srcAddr = trim(tokens[1]);

	string srcAddrReg = registerAllocator.getTempVarLocation(srcAddr);
	string destVarReg = registerAllocator.allocReg(destVar);
	addAsmLine("	movq	(" + srcAddrReg + "), " + destVarReg);
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
		if (src1[0] != '$') addAsmLine("	movq	" + normalizeReg(src1) + ", " + resultReg);
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

	string Reg = registerAllocator.allocReg(convertToReg);
	addAsmLine("	movl	$" + convertValue + ", " + denormalizeReg(Reg, 32));
}


