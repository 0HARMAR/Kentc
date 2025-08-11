//
// Created by hemin on 25-8-9.
//

#include "../TargetGenerator.h"

Function_ TargetGenerator::parseFunctionDef(string str)
{
	size_t at_pos = str.find("@");
	size_t left_paren = str.find("(");
	string functionName = str.substr(at_pos + 1, left_paren - at_pos - 1);
	// remove space
	functionName.erase(remove_if(functionName.begin(), functionName.end(), ::isspace), functionName.end());

	istringstream iss(str);
	vector<string> tokens;
	string token;
	while (iss >> token)
	{
		tokens.push_back(token);
	}

	string returnType = tokens[1];

	size_t right_paren = str.find(')');
	string arg_list = str.substr(left_paren + 1, right_paren - left_paren - 1);

	istringstream args(arg_list);
	vector<string> arg_defs;
	string arg_def;

	while (getline(args, arg_def, ','))
	{
		arg_defs.push_back(arg_def);
	}

	for (auto &arg : arg_defs)
	{
		if (arg[0] == ' ') arg = arg.substr(1);
	}

	vector<Variable> arguments;
	for (auto arg : arg_defs)
	{
		size_t space_pos = arg.find(' ');
		string arg_type = arg.substr(0, space_pos);
		string arg_name = arg.substr(space_pos + 1);
		arguments.push_back({arg_name, arg_type});
	}

	return Function_(functionName, returnType, arguments);
}

