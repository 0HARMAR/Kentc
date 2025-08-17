//
// Created by hemin on 25-8-11.
//
#include "IRGenerator.h"

extern string currentFunction;

std::string IRGenerator::generateExpr(const json &expr,
	int &tempRegCount, std::string &ir)
{
	if (expr["type"] == "Integer")
	{
		return std::to_string(expr["value"].get<int>());
	}
	if (expr["type"] == "String")
	{
		std::string initValue = expr["value"];
		char value = initValue[0];
		return std::to_string(static_cast<size_t> (value));
	}
	if (expr["type"] == "Identifier")
	{
		std::string varName = expr["name"];
		std::string varType;
		for (auto argument : functionArguments[currentFunction])
		{
			if (varName == argument.name)
			{
				varType = argument.type;
				varName = argument.name + ".addr";
				goto HERE;
			}
		}
		for (auto var : functionVariables[currentFunction])
		{
			if (var.name.find(".addr") != string::npos)
			{
				if (var.name == varName + ".addr") varType = var.type;
			} else
			if (var.name == varName) varType = var.type;
		}
		if (currentFunction != "main" and (varName.find(".addr" ) == string::npos))
			varName = varName + ".addr";
HERE:
		std::string typeSize;
		if (varType == "int") typeSize = "32";
		else if (varType == "byte") typeSize = "8";

		std::string tempReg = "%t" + std::to_string(tempRegCount++);
		ir += "	" + tempReg + " = load i" + typeSize + ", i" + typeSize + "* " + "%" + varName + "\n";
		return tempReg;
	}
	if (expr["type"] == "BinaryExpr")
	{
		std::string left = generateExpr(expr["left"],tempRegCount,ir);
		std::string right = generateExpr(expr["right"],tempRegCount,ir);
		std::string op = expr["operator"];
		std::string resultReg = "%t" + std::to_string(tempRegCount++);

		if (op == "+") ir += "	" + resultReg + " = add i32 " + left + ", " + right + "\n";
		else if (op == "-") ir += "	" + resultReg + " = sub i32 " + left + ", " + right + "\n";
		else if (op == "*") ir += "	" + resultReg + " = mul i32 " + left + ", " + right + "\n";
		else if (op == "/") ir += "  " + resultReg + " = sdiv i32 " + left + ", " + right + "\n";

		return resultReg;
	}
	// e.g. %cond = icmp eq i32 %a, 42/%b
	if (expr["type"] == "EqualityExpr")
	{
		std::string left = expr["left"]["name"];
		for (auto argument : functionArguments[currentFunction])
		{
			if (left == argument.name)
			{
				left = left + ".addr";
			}
		}
		left = "%" + left;
		std::string tempReg = "%t" + std::to_string(tempRegCount++);
		ir += "	" + tempReg + " = load i32, i32* " + left + "\n";

		std::string right = generateExpr(expr["right"], tempRegCount,ir);
		std::string resultReg = "%t" + std::to_string(tempRegCount++);

		string typeSize;
		string value = expr["right"]["value"].is_string() ? expr["right"]["value"].get<string>() :
		std::to_string(expr["right"]["value"].get<int>());
		if (!isdigit(value[0])) typeSize = "8";
		else typeSize = "32";
		ir += "	" + resultReg + " = icmp eq i" + typeSize + " " + tempReg + ", " + right + "\n";
		return resultReg;
	}
	if (expr["type"] == "CallExpr")
	{
		string functionName = expr["functionName"];
		vector<string> arguments;
		for (auto& [key, value] : expr["arguments"].items())
		{
			string irExpr;
			string result = generateExpr(value, tempRegCount, irExpr);
			ir += irExpr;
			arguments.push_back(result);
		}
		string callResultTempReg = "%t" + std::to_string(tempRegCount++);
		ostringstream oss;
		oss << "	" + callResultTempReg + " = call i32 " + functionName + "(";
		for (int i = 0; i < arguments.size(); i++)
		{
			oss << typeToIRtype[functionArguments[functionName][i].type] << " " << arguments[i];
			if (i + 1 < arguments.size()) oss << ", ";
		}
		oss << ")\n";
		ir += oss.str();
		return callResultTempReg;
	}
	return "";
}