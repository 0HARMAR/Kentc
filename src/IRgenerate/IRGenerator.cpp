//
// Created by hemin on 25-7-15.
//

#include "IRGenerator.h"

void IRGenerator::generateIR(const json &program, std::string &outputIR)
{
	// add LLVM header decl
	outputIR += "declare void @print_int(i32)\n";
	outputIR += "declare void @exit(i32)\n\n";
	outputIR += "define i32 @main() {\n";

	// var addr alloc manage
	int nextAddr = 512;
	std::map<std::string,int> varAddrMap;
	std::set<int> allAddrs;

	// first step scan all var and alloc addr
	for (const auto &stmt : program["statements"])
	{
		// var decl
		if (stmt["type"] == "Declaration")
		{
			std::string varName = stmt["identifier"];
			varAddrMap[varName] = nextAddr++;
			allAddrs.insert(varAddrMap[varName]);
		}
		// mov stmt dest addr
		if (stmt["type"] == "Mov")
		{
			int addr = stmt["dest"];
			allAddrs.insert(addr);
		}
		// find stmt addr
		if (stmt["type"] == "Find" && stmt["target"]["type"] == "Address")
		{
			int addr = stmt["target"]["value"];
			allAddrs.insert(addr);
		}
	}

	// second step generate alloc inst
	for (int addr : allAddrs)
	{
		outputIR += "  %mem" + std::to_string(addr) + " = alloca i32"+ "\n";
	}

	// temp reg counter
	int tempRegCount = 1;

	// third step process each stmt
	for (const auto &stmt : program["statements"])
	{
		std::string type = stmt["type"];

		if (type == "Declaration" && stmt.contains("initValue"))
		{
			// var init
			std::string irExpr;
			std::string value = generateExpr(stmt["initValue"],varAddrMap,tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "  store i32 " + value + ", i32* %mem" + std::to_string(varAddrMap[stmt["identifier"]]) + "\n";
		}

		else if (type == "Assignment")
		{
			std::string irExpr;
			std::string value = generateExpr(stmt["value"],varAddrMap,tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "  store i32 " + value + ", i32* %mem" + std::to_string(varAddrMap[stmt["target"]]) + "\n";
		}

		else if (type == "Print")
		{
			std::string irExpr;
			std::string value = generateExpr(stmt["expression"],varAddrMap,tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "  call void @print_int(i32 " + value + ")\n";
		}

		else if (type == "Find")
		{
			if (stmt["target"]["type"] == "Identifier")
			{
				std::string varName = stmt["target"]["name"];
				outputIR += "  call void @print_int(i32 " + std::to_string(varAddrMap[varName]) + ")\n";
			} else if (stmt["target"]["type"] == "Address")
			{
				outputIR += "  call void @print_int(i32 " + std::to_string(stmt["target"]["value"].get<int>()) + ")\n";
			}
		}

		else if (type == "Mov")
		{
			std::string irExpr;
			std::string value = generateExpr(stmt["source"],varAddrMap,tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "  store i32 " + value + ", i32* %mem" + std::to_string(stmt["dest"].get<int>()) + "\n";
		}
	}

	// program end
	outputIR += "  call void @exit(i32 0)\n";
	outputIR += "  ret i32 0\n}\n";

}

std::string IRGenerator::generateExpr(const json &expr, std::map<std::string,int> &varAddrMap,
	int &tempRegCount, std::string &ir)
{
	if (expr["type"] == "Integer")
	{
		return std::to_string(expr["value"].get<int>());
	}
	if (expr["type"] == "Identifier")
	{
		std::string varName = expr["name"];
		int addr = varAddrMap[varName];
		std::string tempReg = "%t" + std::to_string(tempRegCount++);
		ir += "  " + tempReg + " = load i32, i32* %mem" + std::to_string(addr) + "\n";
		return tempReg;
	}
	if (expr["type"] == "BinaryExpr")
	{
		std::string left = generateExpr(expr["left"],varAddrMap,tempRegCount,ir);
		std::string right = generateExpr(expr["right"],varAddrMap,tempRegCount,ir);
		std::string op = expr["operator"];
		std::string resultReg = "%t" + std::to_string(tempRegCount++);

		if (op == "+") ir += "  " + resultReg + " = add i32 " + left + ", " + right + "\n";
		else if (op == "-") ir += "  " + resultReg + " = sub i32 " + left + ", " + right + "\n";
		else if (op == "*") ir += "  " + resultReg + " = mul i32 " + left + ", " + right + "\n";
		else if (op == "/") ir += "  " + resultReg + " = sdiv i32 " + left + ", " + right + "\n";

		return resultReg;
	}
	return "";
}