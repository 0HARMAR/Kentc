//
// Created by hemin on 25-7-16.
//

#include "TargetGenerator.h"

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
	string type = trim(tokens[1]);
	string destVar = trim(tokens[2]);

	// if prefix no %, it`s a immediate operand
	if (value.find('%') == string::npos)
	{
		if (variables.find(destVar) != variables.end())
		{
			addAsmLine("	movl	$" + value + ", " +
				to_string(variables[destVar].stackOffset) + "(%rbp)");
		}
	}

	// var assignment
	else
	{
		if (variables.find(value.substr(1)) != variables.end() &&
			variables.find(destVar) != variables.end())
		{
			addAsmLine("	movl	" + to_string(variables[value.substr(1)].stackOffset) + "(%rbp), %eax");
			addAsmLine("	movl	%eax, " + to_string(variables[destVar].stackOffset) + "(%rbp)");
		}
	}
}

string TargetGenerator::processLoad(const vector<string>& tokens)
{
	if (tokens.size() != 2) return "";

	// analyze : %t1 = load i32, i32* %mem512
	string destVar = trim(tokens[0]);
	string srcVar = trim(tokens[1]);

	if (variables.find(srcVar) != variables.end())
	{
		string tempReg = createTempVar();
		addAsmLine("	movl	" + to_string(variables[srcVar].stackOffset) + "(%rbp), %eax");
		addAsmLine("	movl	%eax, " + tempReg + "(%rsp)");
	}
	return "";
}

string TargetGenerator::processBinaryOp(const vector<string>& tokens)
{
	if (tokens.size() != 4) return "";

	string op = tokens[0];
	string type = tokens[1];
	string op1 = tokens[2];
	string op2 = tokens[3];

	// create a temp var to store the result
	string result = createTempVar();

	// process operand
	string src1,src2;
	if (op1.find('%') == string::npos)
	{
		string var = op1.substr(1);
		if (variables.find(var) != variables.end())
		{
			addAsmLine("	movl	" + to_string(variables[var].stackOffset) + "(%rbp), %eax");
			src1 = "%eax";
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
			addAsmLine("	movl	" + to_string(variables[var].stackOffset) + "(%rbp), %ecx");
			src2 = "%ecx";
		}
	} else
	{
		src2 = "$" + op2;
	}

	// process div
	if (op == "sdiv")
	{
		addAsmLine("	movl	%eax, %edx");
		addAsmLine("	sarl	$31, %edx");
		addAsmLine("	idivl	%ecx");
		addAsmLine("	movl	%eax, " + result + "(%rsp)");
		return result;
	}

	// process other ops
	if (opMap.find(op) != opMap.end())
	{
		addAsmLine("	" + opMap[op] + "	" + src2 + ", %eax");
		addAsmLine("	movl	%eax, " + result + "(%rsp)");
		return result;
	}

	return "";
}

void TargetGenerator::processCall(const vector<string>& tokens)
{
	if (tokens.empty()) return;

	string funcName = tokens[0];

	if (funcName == "@print_int" || funcName == "@exit")
	{
		string arg;
		if (tokens.size() > 1)
		{
			arg = tokens[1];

			// if arg is const
			if (arg.find('%') == string::npos)
			{
				string var = arg.substr(1);
				if (variables.find(var) != variables.end())
				{
					addAsmLine("	movl	" + to_string(variables[var].stackOffset) + "(%rbp), %edi");
				}
			}
			// else an immediate number
			else
			{
				addAsmLine("	movl	$" + arg + ", %edi");
			}
		}

		addAsmLine("	call	" + funcName.substr(1));
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

		// skip decl and matedata
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

		// alloca instrunction
		if (tokens[0].find("alloca") != string::npos)
		{
			vector<string> args(tokens.begin() + 1, tokens.end());
			processAlloca(args);
		}
		else if (tokens[0] == "store")
		{
			vector<string> args(tokens.begin() + 1, tokens.end());
			processStore(args);
		}
		else if (tokens[0] == "load")
		{
			vector<string> args(tokens.begin() + 1, tokens.end());
			processLoad(args);
		}
		else if (opMap.find(tokens[0]) != opMap.end())
		{
			string result = *tokens.rbegin();
			vector<string> args(tokens.begin() + 1, tokens.end());
			string tempVar = processBinaryOp(args);
			if (!tempVar.empty() && result.find("%") != string::npos)
			{
				string destVar = result.substr(1);
				if (variables.find(destVar) != variables.end())
				{
					addAsmLine("	movl	" + tempVar +  "(%rsp), %eax");
					addAsmLine("	movl	%eax, " + to_string(variables[destVar].stackOffset) + "(%rbp)");
				}
			}
		}
		else if (tokens[0] == "call")
		{
			vector<string> args(tokens.begin() + 1, tokens.end());
			processCall(args);
		}
	}

	// calculate stack size and add asm suffix
	int stackAlloc = (maxStackOffset + 15) & ~15;
	asmLines.insert(asmLines.begin() + 4, "	subq	$" + to_string(stackAlloc) + ", %rsp");

	addAsmLine("	movl	$0, %eax");
	addAsmLine("	leave");
	addAsmLine("	ret");
	addAsmLine("");
	addAsmLine("	.section	.note.GNU-stack,\"\",@progbits");

	return asmLines;
}




