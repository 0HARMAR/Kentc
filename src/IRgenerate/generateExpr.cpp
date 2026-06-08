#include "IRGenerator.h"

std::string IRGenerator::generateExpr(const json &expr)
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

        std::string tempReg = irWriter.newTemp();
        irWriter.emitLine(tempReg + " = load i" + typeSize + ", i" + typeSize + "* %" + varName);
        return tempReg;
    }
    if (expr["type"] == "BinaryExpr")
    {
        std::string left = generateExpr(expr["left"]);
        std::string right = generateExpr(expr["right"]);
        std::string op = expr["operator"];
        std::string resultReg = irWriter.newTemp();

        if (op == "+") irWriter.emitLine(resultReg + " = add i32 " + left + ", " + right);
        else if (op == "-") irWriter.emitLine(resultReg + " = sub i32 " + left + ", " + right);
        else if (op == "*") irWriter.emitLine(resultReg + " = mul i32 " + left + ", " + right);
        else if (op == "/") irWriter.emitLine(resultReg + " = sdiv i32 " + left + ", " + right);

        return resultReg;
    }
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
        std::string tempReg = irWriter.newTemp();
        irWriter.emitLine(tempReg + " = load i32, i32* " + left);

        std::string right = generateExpr(expr["right"]);

        string typeSize;
        string value = expr["right"]["value"].is_string() ? expr["right"]["value"].get<string>() :
        std::to_string(expr["right"]["value"].get<int>());
        if (!isdigit(value[0])) typeSize = "8";
        else typeSize = "32";

        std::string resultReg = irWriter.newTemp();
        irWriter.emitLine(resultReg + " = icmp eq i" + typeSize + " " + tempReg + ", " + right);
        return resultReg;
    }
    if (expr["type"] == "CallExpr")
    {
        string functionName = expr["functionName"];
        vector<string> arguments;
        for (auto& [key, value] : expr["arguments"].items())
        {
            string result = generateExpr(value);
            arguments.push_back(result);
        }
        string callResultTempReg = irWriter.newTemp();
        string callLine = callResultTempReg + " = call i32 " + functionName + "(";
        for (int i = 0; i < arguments.size(); i++)
        {
            callLine += typeToIRtype[functionArguments[functionName][i].type] + " " + arguments[i];
            if (i + 1 < arguments.size()) callLine += ", ";
        }
        callLine += ")";
        irWriter.emitLine(callLine);
        return callResultTempReg;
    }
    return "";
}
