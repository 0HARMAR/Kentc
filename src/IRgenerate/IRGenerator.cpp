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

// functions
string functions;

// current function
string currentFunction = "main";

extern string runMode;

IRGenerator::IRGenerator()
{
	functionVariables["main"] = {};
}

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

	if (!functions.empty()) outputIR += functions;

	if (runMode == "DEV")
	{
		string IRPath = R"(/mnt/c/Users/hemin/kentc/ELFBUILD/ir.llvm)";
		ofstream file(IRPath);
		if (file.is_open())
		{
			file << outputIR << std::endl;
			file.close();
		} else
		{
			cerr << "connot open file" << IRPath << endl;
		}
	}
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
					string varName = value["name"];
					string varType = getVarTypeByName(varName);
					if (varType == "int") outputIR += "	call void @print_int(i32 " + tempReg + ")\n";
					else outputIR += "	call void @print_string(i8* %" + varName + ", i32 1)\n";
				}
				irExpr = "";
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

			string looperLabelNum_ = std::to_string(looperLabelNum);
			std::string looperLabel = "%lopper" + looperLabelNum_;
			outputIR += looperLabel.substr(1, looperLabel.length() - 1) + ":\n";
			looperLabelNum++;
			parseStatements(stmt["looperBody"], outputIR);
			std::string looperTimesLoadResult = "%t" + std::to_string(tempRegCount++);
			std::string looperTempReg = "%t" + std::to_string(tempRegCount++);

			// looperTimes = looperTimes + 1
			outputIR += "	" + looperTimesLoadResult + " = load i32, i32* "  + tempReg + "\n";
			outputIR += "	" + looperTempReg + " = add i32 " + looperTimesLoadResult + ", 1\n";
			outputIR += "	store i32 " + looperTempReg + ", i32* " + tempReg + "\n";
			std::string cmpResultTempReg = "%t" + std::to_string(tempRegCount++);

			outputIR += "	" + cmpResultTempReg + " = icmp eq i32 " + looperTempReg + ", " + looperTimes + "\n";

			std::string looperEnd = "%end" + looperLabelNum_;
			outputIR += "	br i1 " + cmpResultTempReg + ", label " + looperEnd
			+ ", label " + looperLabel + "\n";
			outputIR += looperEnd.substr(1, looperEnd.length() - 1) + ":\n";
		}
		else if (type == "Function")
		{
			string functionName = stmt["functionName"];
			string returnType = stmt["returnType"];
			vector<pair<string,string>> arguments = stmt["arguments"]; // (type, name)

			// write function define
			ostringstream oss;
			oss << "define " << typeToIRtype[returnType] << " @" << functionName << "(";
			for (size_t i = 0; i < arguments.size(); i++)
			{
				oss << typeToIRtype[arguments[i].first] << " " << '%' + arguments[i].second;
				if (i + 1 < arguments.size()) oss << ", ";
			}

			oss << ") {";
			functions += oss.str() + "\n";

			// write function body
			functions += "main:\n"; // default block named main

			string oldFunction = currentFunction;
			currentFunction = functionName;
			// alloca and store for local variable
			for (size_t i = 0; i < arguments.size(); i++)
			{
				functions += "	%" + arguments[i].second + ".addr = alloca i32\n";
				functionArguments[currentFunction].push_back({arguments[i].second, arguments[i].first});
				functionVariables[currentFunction].push_back({arguments[i].second + ".addr", arguments[i].first});
			}
			for (size_t i = 0; i < arguments.size(); i++)
			{
				string name = arguments[i].second;
				functions += "	store i32 %" + name + ", i32* %" + name + ".addr\n";
			}
			parseStatements(stmt["functionBody"], functions);
			currentFunction = oldFunction;

			functions += "}\n";
		}
		else if (type == "Return")
		{
			string resultTempReg = generateExpr(stmt["returnValue"], tempRegCount, outputIR);
			outputIR += "	ret i32 " + resultTempReg + "\n";
		}
	}
}

string IRGenerator::getVarTypeByName(string name)
{
	for (auto var : functionVariables[currentFunction])
	{
		if (var.name == name) return var.type;
	}
}
