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

	// if prefix no %, it`s an immediate operand
	if (value.find('%') == string::npos)
	{
		if (variables.find(destVar) != variables.end())
		{
			addAsmLine("	movl	$" + value + ", " +
				to_string(variables[destVar].stackOffset) + "(%rbp)");
		}
	}
	// temp var
	else
	{
		if (variables.find(value.substr(1)) != variables.end() &&
			variables.find(destVar) != variables.end())
		{
			string varToReg = registerAllocator.getTempVarLocation(value);
			addAsmLine("	movl	" + varToReg + ", " + to_string(variables[destVar].stackOffset) + "(%rbp)");
		}
	}
}

string TargetGenerator::processLoad(const vector<string>& tokens)
{
	if (tokens.size() != 2) return "";

	// analyze : %t1 = load i32, i32* %mem512
	string destVar = trim(tokens[0]);
	string srcAddr = trim(tokens[1]);

	if (variables.find(srcAddr) != variables.end())
	{
		string varToReg = registerAllocator.allocReg(destVar);
		addAsmLine("	movl	" + to_string(variables[srcAddr].stackOffset) + "(%rbp)" + ", " + varToReg);
	}
	return "";
}

string TargetGenerator::processBinaryOp(const vector<string>& tokens)
{
	if (tokens.size() != 4) return "";

	string result = tokens[0];
	string op = tokens[1];
	string type = tokens[2];
	string op1 = tokens[3];
	string op2 = tokens[4];

	// process operand
	string src1,src2;
	if (op1.find('%') == string::npos)
	{
		string var = op1.substr(1);
		if (variables.find(var) != variables.end())
		{
			src1 = registerAllocator.getTempVarLocation(var);
		}
	} else
	{
		src1 = "$" + op1;
	}

	if (op2.find('%') == string::npos)
	{
		string var = op2.substr(1);
		if (variables.find(var) != variables.end())
		{
			src2 = registerAllocator.getTempVarLocation(var);
		}
	} else
	{
		src2 = "$" + op2;
	}

	// process div
	if (op == "sdiv")
	{
		handleDivision(op1, op2, result);
	}

	// process other ops
	if (opMap.find(op) != opMap.end())
	{
		string resultReg = registerAllocator.allocReg(result);
		addAsmLine("	movl	" + src1 + ", " + resultReg);
		addAsmLine("	" + opMap[op] + "	" + src2 + ", " + resultReg);
	}

	return "";
}

void TargetGenerator::processCall(const vector<string>& tokens)
{
	if (tokens.empty()) return;

	string funcName = tokens[0];
	funcName.erase(funcName.size()-4);

	if (funcName == "@print_int" || funcName == "@exit")
	{
		string arg;
		if (tokens.size() > 1)
		{
			arg = tokens[1];
			arg.pop_back();

			// if arg is variable
			if (arg.find('%') != string::npos)
			{
				string var = registerAllocator.allocReg(arg.substr(1));
				if (variables.find(var) != variables.end())
				{
					addAsmLine("	movl	" + var + ", %edi");
				}
			}
			// else an immediate number
			else
			{
				addAsmLine("	movl	$" + arg + ", %edi");
			}
		}

		CallingConvention::addCallerSave();
		addAsmLine("	call	" + funcName.substr(1));
		CallingConvention::restoreCallerSave();
	}
}

vector<string> TargetGenerator::convertIRToASM(const vector<string>& irLines)
{
	// add asm prefix
	addAsmLine("#Generated AT&T assembly from LLVM IR");
	addAsmLine("	.text");
	addAsmLine("	.global	main");
	addAsmLine("	.type	main, @function");
	addAsmLine("");
	addAsmLine("main:");
	addAsmLine("	pushq	%rbp");
	addAsmLine("	movq	%rsp, %rbp");

	for (const auto& line : irLines)
	{
		string trimmed = trim(line);
		if (trimmed.empty()) continue;

		// skip decl and metadata
		if (trimmed.find("declare") == 0 ||
			trimmed.find("define") == 0 ||
				trimmed.find("}") == 0)
		{
			continue;
		}

		// split instruction
		vector<string> tokens;
		istringstream iss(trimmed);
		string token;
		while (iss >> token)
		{
			tokens.push_back(token);
		}

		if (tokens.empty()) continue;

		// alloca instruction
		if (tokens[2].find("alloca") != string::npos)
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[3]);
			processAlloca(args);
		}
		else if (tokens[0] == "store")
		{
			vector<string> args;
			args.push_back(tokens[2]);
			args.push_back(tokens[1]);
			args.push_back(tokens[4]);
			processStore(args);
		}
		else if (tokens[2] == "load")
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[5]);
			processLoad(args);
		}
		else if (opMap.find(tokens[2]) != opMap.end())
		{
			string result = *tokens.rbegin();
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[2]);
			args.push_back(tokens[3]);
			args.push_back(tokens[4]);
			args.push_back(tokens[5]);

			processBinaryOp(args);
		}
		else if (tokens[0] == "call")
		{
			vector<string> args;
			args.push_back(tokens[2]);
			args.push_back(tokens[3]);
			processCall(args);
		}
	}

	// calculate stack size and add asm suffix
	ostringstream ir_oss;
	for (size_t i = 0; i < irLines.size(); ++i)
	{
		ir_oss << irLines[i] << endl;
	}

	int stackAlloc = staticProgramAnalyzer.analyze(ir_oss.str());
	asmLines.insert(asmLines.begin() + 8, "	subq	$" + to_string(stackAlloc) + ", %rsp");

	addAsmLine("	movl	$0, %eax");
	addAsmLine("	leave");
	addAsmLine("	ret");
	addAsmLine("");
	addAsmLine("	.section	.note.GNU-stack,\"\",@progbits");

	return asmLines;
}




