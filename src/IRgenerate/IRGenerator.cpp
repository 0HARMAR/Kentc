//
// Created by hemin on 25-7-15.
//

#include "IRGenerator.h"

void IRGenerator::generateIR(const json &program, std::string &outputIR)
{
	// add LLVM header decl
	outputIR += "declare void @print_int(i32)\n";
	outputIR += "declare void @exit(i32)\n";

	// for memory alloc
	outputIR += "declare i8* @malloc(i64)\n";
	outputIR += "declare i8* @malloc_at(i64, i64)\n";
	outputIR += "declare void @free(i8*)\n\n";

	outputIR += "define i32 @main() {\n";

	// var addr alloc manage
	std::map<std::string,std::string> varAddrMap;

	// first step scan all var and alloc addr
	for (const auto &stmt : program["statements"])
	{
		// var decl
		if (stmt["type"] == "Declaration")
		{
			std::string varName = stmt["identifier"];
			varAddrMap[varName] = stmt["address"];
		}
	}

	// TODO second step generate alloc inst
	// for (int addr : allAddrs)
	// {
	// 	outputIR += "  %mem" + std::to_string(addr) + " = alloca i32"+ "\n";
	// }

	// temp reg counter
	int tempRegCount = 1;

	// third step process each stmt
	for (const auto &stmt : program["statements"])
	{
		std::string type = stmt["type"];

		if (type == "Declaration" && stmt.contains("initValue"))
		{
			// malloc and init
			std::string varName = stmt["identifier"];
			std::string varAddr = stmt["address"];
			outputIR += "%" + varName + " = call i8* @malloc(i64 " + std::to_string(4) + ", i64 "
			+ varAddr + ")\n";

			std::string irExpr;
			std::string value = generateExpr(stmt["initValue"],varAddrMap,tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "  store i32 " + value + ", i32* %" + varName + "\n";
		}

		else if (type == "Assignment")
		{
			std::string irExpr;
			std::string value = generateExpr(stmt["value"],varAddrMap,tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "  store i32 " + value + ", i32* " + varAddrMap["%" + stmt["target"]] + "\n";
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
				outputIR += "  call void @print_int(i " +  + ")\n";
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