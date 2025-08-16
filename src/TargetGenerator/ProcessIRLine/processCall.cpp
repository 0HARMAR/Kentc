//
// Created by hemin on 25-8-11.
//

#include "../TargetGenerator.h"

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

	// inner functions
	if (funcName == "print_int") {
		handlePrintInt(tokens);
	} else if (funcName == "print_hex")
	{
		handlePrintHex(tokens);
	}
	else if (funcName == "exit") {
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

	// self-definition functions
	for (auto function : functions)
	{
		if (funcName == function.name)
		{
			vector<Variable> arguments;
			for (auto arg : argList)
			{
				arguments.push_back({arg.first, arg.second});
			}
			handleCall(Function_{funcName, returnType, arguments},returnReg);
		}
	}

}