//
// Created by hemin on 25-7-15.
//

#include "IRGenerator.h"

// temp reg counter
int tempRegCount = 1;

// condition label num
int conditionLabelNum = 0;

// looper label num
int looperLabelNum = 0;

void IRGenerator::generateIR(const json &program, std::string &outputIR)
{
	// add LLVM header decl
	outputIR += "declare void @print_int(i32)\n";
	outputIR += "declare void @exit(i32)\n";

	// for memory alloc
	outputIR += "declare i8* @malloc(i64)\n";
	outputIR += "declare i8* @malloc_at(i64, i64)\n";
	outputIR += "declare void @free(i8*)\n";

	// for system stdin
	outputIR += "declare void @in(i32, i32)\n";

	// for print string
	outputIR += "declare void @print_string(i8*, i32)\n\n";

	outputIR += "define i32 @main() {\n";

	// TODO second step generate alloc inst

	// third step process each stmt
	parseStatements(program, outputIR);

	// program end
	outputIR += "	call void @exit(i32 0)\n";
	outputIR += "	ret i32 0\n}\n";

}

void IRGenerator::parseStatements(const json& program, std::string& outputIR)
{
	for (const auto &stmt : program["statements"])
	{
		std::string type = stmt["type"];

		if (type == "Declaration" && stmt.contains("initValue"))
		{
			// malloc and init
			std::string varName = stmt["identifier"];
			std::string varType = stmt["varType"];
			variables.push_back(variable{varName, varType});
			std::string typeSize;
			if (varType == "int") typeSize = "32";
			else if (varType == "byte") typeSize = "8";

			std::string varAddr = std::to_string(stmt["address"].get<int>());
			outputIR += "	%" + varName + " = call i" + typeSize + "* @malloc_at(i64 " + std::to_string(4) + ", i64 "
			+ varAddr + ")\n";

			std::string irExpr;
			std::string value = generateExpr(stmt["initValue"],tempRegCount,irExpr);
			outputIR += irExpr;
			outputIR += "	store i" + typeSize + " " + value + ", i" + typeSize + "* %" + varName + "\n";
		}

		else if (type == "Assignment")
		{
			std::string irExpr;
			std::string value = generateExpr(stmt["value"],tempRegCount,irExpr);
			outputIR += irExpr;
			std::string targetVar = "%" + stmt["target"].get<std::string>();
			outputIR += "	store i32 " + value + ", i32* " + targetVar + "\n";
		}

		else if (type == "Print")
		{
			std::string irExpr;
			std::string stringValue;
			for (const auto& [key, value] : stmt["expression"].items())
			{
				if (value["type"] == "String")
				{
					stringValue = value["value"];
					int length = stringValue.length() + 1; // include \0
					outputIR = "@str" + std::to_string(index) + " = constant [" + std::to_string(length)
					+ " x i8] c\"" + stringValue + "\\00\"\n"
					+ outputIR;
					std::string stringTempReg = "%t" + std::to_string(tempRegCount++);
					outputIR += "\t" + stringTempReg + " = getelementptr [" + std::to_string(length)
					+ " x i8], [" + std::to_string(length) + " x i8]* @str" + std::to_string(index)
					+", i32 0, i32, 0\n";
					outputIR += "	call void @print_string(i8* " + stringTempReg
					+ ", i32 " + std::to_string(length) + ")\n";
					index++;
				}
				else if (value["type"] == "Identifier")
				{
					std::string tempReg = generateExpr(value, tempRegCount, irExpr);
					outputIR += irExpr;
					outputIR += "	call void @print_int(i32 " + tempReg + ")\n";
				}
			}
		}

		else if (type == "Find")
		{
			if (stmt["target"]["type"] == "Identifier")
			{
				std::string varName = stmt["target"]["name"];
				outputIR += "	call void @print_int(i32 %" + varName + ")\n";
			}
		}

		else if (type == "Mov")
		{
			std::string irExpr;
			std::string value = generateExpr(stmt["source"],tempRegCount,irExpr);
			outputIR += irExpr;
			std::string addr = std::to_string(stmt["dest"].get<int>());
			// convert immediate address to point
			std::string addrReg = "%t" + std::to_string(tempRegCount++);
			outputIR += "	" + addrReg + " = inttoptr i64 " + addr + " to i32*\n";
			outputIR += "	store i32 " + value + ", i32* " + addrReg + "\n";
		}
		else if (type == "Selector")
		{
			std::string irConditionalExpr;
			std::string resultReg = generateExpr(stmt["condition"], tempRegCount, irConditionalExpr);
			outputIR += irConditionalExpr;
			// do branch on condition
			// e.g. br i1 %cond, label %if_true, label %if_false
			std::string trueLabel = "%if_true" + std::to_string(conditionLabelNum++);
			std::string continueLabel = "%continue" + std::to_string(conditionLabelNum++);
			outputIR += "	br i1 " + resultReg + ", label " + trueLabel + ", label " + continueLabel + "\n";
			outputIR += trueLabel.substr(1, trueLabel.length() - 1) + ":\n";
			parseStatements(stmt["conditionBody"], outputIR);
			outputIR += continueLabel.substr(1, continueLabel.length() - 1) + ":\n";
		}
		else if (type == "In")
		{
			std::string inBytesNum = stmt["inBytesNum"];
			std::string inAddress = stmt["inAddress"];
			inAddress = std::to_string(std::stoul(inAddress, nullptr, 16));
			outputIR += "	call void @in(i32 " + inBytesNum + ", i32 " + inAddress + ")\n";
		}
		else if (type == "Looper")
		{
			std::string looperTimes = std::to_string(stmt["looperTimes"].get<int>());
			std::string tempReg = "%t" + std::to_string(tempRegCount++);

			// reset looperTimes temp reg
			outputIR += "	" + tempReg + " = alloca i32\n";
			outputIR += "	store i32 0, i32* " + tempReg + "\n";

			std::string looperLabel = "%lopper" + std::to_string(looperLabelNum);
			outputIR += looperLabel.substr(1, looperLabel.length() - 1) + ":\n";
			parseStatements(stmt["looperBody"], outputIR);
			std::string looperTimesLoadResult = "%t" + std::to_string(tempRegCount++);
			std::string looperTempReg = "%t" + std::to_string(tempRegCount++);

			// looperTimes = looperTimes + 1
			outputIR += "	" + looperTimesLoadResult + " = load i32, i32* "  + tempReg + "\n";
			outputIR += "	" + looperTempReg + " = add i32 " + looperTimesLoadResult + ", 1\n";
			outputIR += "	store i32 " + looperTempReg + ", i32* " + tempReg + "\n";
			std::string cmpResultTempReg = "%t" + std::to_string(tempRegCount++);

			outputIR += "	" + cmpResultTempReg + " = icmp eq i32 " + looperTempReg + ", " + looperTimes + "\n";

			std::string looperEnd = "%end" + std::to_string(looperLabelNum);
			outputIR += "	br i1 " + cmpResultTempReg + ", label " + looperEnd
			+ ", label " + looperLabel + "\n";
			outputIR += looperEnd.substr(1, looperEnd.length() - 1) + ":\n";
		}
	}
}

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
		char initValue_char = initValue[0];
		int initValue_char_int = static_cast<int> (initValue_char);
		std::stringstream ss;
		ss << std::hex << initValue_char_int;
		std::string initValue_hex = ss.str();
		return initValue_hex;
	}
	if (expr["type"] == "Identifier")
	{
		std::string varName = expr["name"];
		std::string varType;
		for (auto var : variables)
		{
			if (var.name == varName) varType = var.type;
		}
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
		else if (op == "-") ir += "  " + resultReg + " = sub i32 " + left + ", " + right + "\n";
		else if (op == "*") ir += "  " + resultReg + " = mul i32 " + left + ", " + right + "\n";
		else if (op == "/") ir += "  " + resultReg + " = sdiv i32 " + left + ", " + right + "\n";

		return resultReg;
	}
	// e.g. %cond = icmp eq i32 %a, 42/%b
	if (expr["type"] == "EqualityExpr")
	{
		std::string left = expr["left"]["name"];
		left = "%" + left;
		std::string tempReg = "%t" + std::to_string(tempRegCount++);
		ir += "	" + tempReg + " = load i32, i32* " + left + "\n";

		std::string right = std::to_string(static_cast<int>(expr["right"]["value"]));
		std::string resultReg = "%t" + std::to_string(tempRegCount++);

		ir += "	" + resultReg + " = icmp eq i32 " + tempReg + ", " + right + "\n";
		return resultReg;
	}
	return "";
}