#include "IRGenerator.h"
#include <fstream>
#include <iostream>

extern string runMode;

IRGenerator::IRGenerator()
{
    functionVariables["main"] = {};
}

void IRGenerator::generateIR(const json &program, std::string &outputIR)
{
    irWriter.emitDecl("declare void @print_int(i32)");
    irWriter.emitDecl("declare void @print_hex(i32)");
    irWriter.emitDecl("declare void @exit(i32)");
    irWriter.emitDecl("declare i8* @malloc(i64)");
    irWriter.emitDecl("declare i8* @malloc_at(i64, i64)");
    irWriter.emitDecl("declare void @free(i8*)");
    irWriter.emitDecl("declare void @in(i32, i32)");
    irWriter.emitDecl("declare void @print_string(i8*, i32)");

    irWriter.emitStrConst("0x");

    irWriter.emitRaw("define i32 @main() {\n");

    parseStatements(program);

    irWriter.emitLine("call void @exit(i32 0)");
    irWriter.emitLine("ret i32 0");
    irWriter.emitRaw("}\n");

    outputIR = irWriter.build();
}

void IRGenerator::parseStatements(const json& program)
{
    for (const auto &stmt : program["statements"])
    {
        std::string type = stmt["type"];

        if (type == "Declaration" && stmt.contains("initValue"))
        {
            std::string varName = stmt["identifier"];
            std::string varType = stmt["varType"];
            std::string typeSize;
            if (varType == "int") typeSize = "32";
            else if (varType == "byte") typeSize = "8";

            std::string irExpr;
            std::string value = generateExpr(stmt["initValue"]);

            std::string varAddr = std::to_string(stmt["address"].get<int>());
            if (varAddr == "-1")
            {
                irWriter.emitLine("%" + varName + ".addr = alloca i32");
                std::string loadReg = irWriter.newTemp();
                irWriter.emitLine(loadReg + " = load i32, i32* %" + varName + ".addr");
                functionVariables[currentFunction].push_back(variable{varName + ".addr", varType});
                functionVariables[currentFunction].push_back(variable{varName, varType});
                irWriter.emitLine("store i" + typeSize + " " + value + ", i" + typeSize + "* %" + varName + ".addr");
            }
            else
            {
                irWriter.emitLine("%" + varName + " = call i" + typeSize + "* @malloc_at(i64 " + std::to_string(4) + ", i64 "
                    + varAddr + ")");
                functionVariables[currentFunction].push_back(variable{varName, varType});
                irWriter.emitLine("store i" + typeSize + " " + value + ", i" + typeSize + "* %" + varName);
            }
        }

        else if (type == "Assignment")
        {
            std::string value = generateExpr(stmt["value"]);
            std::string targetVar = "%" + stmt["target"].get<std::string>();
            irWriter.emitLine("store i32 " + value + ", i32* " + targetVar);
        }

        else if (type == "Print")
        {
            std::string irExpr;
            for (const auto& [key, value] : stmt["expression"].items())
            {
                if (value["type"] == "String")
                {
                    std::string stringValue = value["value"];
                    std::string strName = irWriter.emitStrConst(stringValue);
                    std::string stringTempReg = irWriter.newTemp();
                    int length = stringValue.length() + 1;
                    irWriter.emitLine(stringTempReg + " = getelementptr [" + std::to_string(length)
                        + " x i8], [" + std::to_string(length) + " x i8]* " + strName
                        + ", i32 0, i32, 0");
                    irWriter.emitLine("call void @print_string(i8* " + stringTempReg
                        + ", i32 " + std::to_string(length) + ")");
                }
                else if (value["type"] == "Identifier")
                {
                    std::string tempReg = generateExpr(value);
                    string varName = value["name"];
                    string varType = getVarTypeByName(varName);
                    if (varType == "int")
                        irWriter.emitLine("call void @print_int(i32 " + tempReg + ")");
                    else
                        irWriter.emitLine("call void @print_string(i8* %" + varName + ", i32 1)");
                }
            }
        }

        else if (type == "Find")
        {
            if (stmt["target"]["type"] == "Identifier")
            {
                std::string varName = stmt["target"]["name"];
                std::string relativeAddrTempReg = irWriter.newTemp();
                irWriter.emitLine(relativeAddrTempReg + " = sub i32 %" + varName + ", 6291456");

                std::string stringTempReg = irWriter.newTemp();
                int len = 3;
                irWriter.emitLine(stringTempReg + " = getelementptr [" + to_string(len)
                    + " x i8], [" + to_string(len) + " x i8]* @str0"
                    + ", i32 0, i32, 0");
                irWriter.emitLine("call void @print_string(i8* " + stringTempReg
                    + ", i32 " + to_string(len) + ")");

                irWriter.emitLine("call void @print_hex(i32 " + relativeAddrTempReg + ")");
            }
        }

        else if (type == "Mov")
        {
            std::string value = generateExpr(stmt["source"]);
            std::string addr = std::to_string(stmt["dest"].get<int>());
            std::string addrReg = irWriter.newTemp();
            irWriter.emitLine(addrReg + " = inttoptr i64 " + addr + " to i32*");
            irWriter.emitLine("store i32 " + value + ", i32* " + addrReg);
        }
        else if (type == "Selector")
        {
            std::string resultReg = generateExpr(stmt["condition"]);
            std::string trueLabel = irWriter.newLabel("%if_true");
            std::string continueLabel = irWriter.newLabel("%continue");
            irWriter.emitLine("br i1 " + resultReg + ", label " + trueLabel + ", label " + continueLabel);
            irWriter.emitRaw(trueLabel.substr(1) + ":\n");
            parseStatements(stmt["conditionBody"]);
            irWriter.emitRaw(continueLabel.substr(1) + ":\n");
        }
        else if (type == "In")
        {
            std::string inBytesNum = stmt["inBytesNum"];
            std::string inAddress = stmt["inAddress"];
            inAddress = std::to_string(std::stoul(inAddress, nullptr, 16));
            irWriter.emitLine("call void @in(i32 " + inBytesNum + ", i32 " + inAddress + ")");
        }
        else if (type == "Looper")
        {
            std::string looperTimes = std::to_string(stmt["looperTimes"].get<int>());
            std::string tempReg = irWriter.newTemp();

            irWriter.emitLine(tempReg + " = alloca i32");
            irWriter.emitLine("store i32 0, i32* " + tempReg);

            string looperLabelNum_ = to_string(looperLabelNum);
            std::string looperLabel = "%lopper" + looperLabelNum_;
            irWriter.emitRaw(looperLabel.substr(1) + ":\n");
            looperLabelNum++;
            parseStatements(stmt["looperBody"]);
            std::string looperTimesLoadResult = irWriter.newTemp();
            std::string looperTempReg = irWriter.newTemp();

            irWriter.emitLine(looperTimesLoadResult + " = load i32, i32* " + tempReg);
            irWriter.emitLine(looperTempReg + " = add i32 " + looperTimesLoadResult + ", 1");
            irWriter.emitLine("store i32 " + looperTempReg + ", i32* " + tempReg);
            std::string cmpResultTempReg = irWriter.newTemp();

            irWriter.emitLine(cmpResultTempReg + " = icmp eq i32 " + looperTempReg + ", " + looperTimes);

            std::string looperEnd = "%end" + looperLabelNum_;
            irWriter.emitLine("br i1 " + cmpResultTempReg + ", label " + looperEnd
                + ", label " + looperLabel);
            irWriter.emitRaw(looperEnd.substr(1) + ":\n");
        }
        else if (type == "Function")
        {
            string functionName = stmt["functionName"];
            string returnType = stmt["returnType"];
            vector<pair<string,string>> arguments = stmt["arguments"];

            vector<pair<string,string>> irParams;
            for (auto& arg : arguments)
            {
                irParams.push_back({typeToIRtype[arg.first], arg.second});
            }

            irWriter.beginFunctionDef(functionName, typeToIRtype[returnType], irParams);

            string oldFunction = currentFunction;
            currentFunction = functionName;

            for (size_t i = 0; i < arguments.size(); i++)
            {
                irWriter.emitLine("%" + arguments[i].second + ".addr = alloca i32");
                functionArguments[currentFunction].push_back({arguments[i].second, arguments[i].first});
                functionVariables[currentFunction].push_back({arguments[i].second + ".addr", arguments[i].first});
            }
            for (size_t i = 0; i < arguments.size(); i++)
            {
                string name = arguments[i].second;
                irWriter.emitLine("store i32 %" + name + ", i32* %" + name + ".addr");
            }
            parseStatements(stmt["functionBody"]);
            currentFunction = oldFunction;

            irWriter.endFunctionDef();
        }
        else if (type == "Return")
        {
            std::string resultTempReg = generateExpr(stmt["returnValue"]);
            irWriter.emitLine("ret i32 " + resultTempReg);
        }
    }
}

string IRGenerator::getVarTypeByName(string name)
{
    for (auto var : functionVariables[currentFunction])
    {
        if (var.name == name) return var.type;
    }
    return "";
}
