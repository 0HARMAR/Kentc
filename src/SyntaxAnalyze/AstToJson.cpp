#include "Parser.h"

json Parser::astToJson(const ASTNode* node)
{
	if (!node) return json(nullptr);

	json j;

	if (auto program = dynamic_cast<const ProgramNode*>(node))
	{
		j["type"] = "Program";
		json statements;
		for (auto& stmt : program->statements)
		{
			statements.push_back(astToJson(stmt.get()));
		}
		j["statements"] = statements;
	}
	else if (auto decl = dynamic_cast<const DeclNode*>(node))
	{
		j["type"] = "Declaration";
		std::string varType;
		if (decl->varType == ValueType::INT) varType = "int";
		else if (decl->varType == ValueType::BYTE) varType = "byte";

		j["varType"] = varType;
		j["identifier"] = decl->identifier;
		j["address"] = decl->address;
		if (decl->initValue)
		{
			j["initValue"] = astToJson(decl->initValue.get());
		}
	}
	else if (auto assign = dynamic_cast<const AssignNode*>(node))
	{
		j["type"] = "Assignment";
		j["target"] = assign->identifier;
		if (assign->value)
		{
			j["value"] = astToJson(assign->value.get());
		}
	}
	else if (auto print = dynamic_cast<const PrintNode*>(node))
	{
		j["type"] = "Print";
		if (print->expr)
		{
			j["expression"] = astToJson(print->expr.get());
		}
	}
	else if (auto find = dynamic_cast<const FindNode*>(node))
	{
		j["type"] = "Find";
		if (find->target)
		{
			j["target"] = astToJson(find->target.get());
		}
	}
	else if (auto mov = dynamic_cast<const MovNode*>(node))
	{
		j["type"] = "Mov";
		if (mov->src)
		{
			j["source"] = astToJson(mov->src.get());
		}
		j["dest"] = mov->destAddr;
	}
	else if (auto selector = dynamic_cast<const SelectorNode*>(node))
	{
		j["type"] = "Selector";
		j["condition"] = astToJson(selector->conditionalExpr.get());
		j["conditionBody"] = astToJson(selector->conditionalProgram.get());
	}
	else if (auto in = dynamic_cast<const InNode*>(node))
	{
		j["type"] = "In";
		j["inBytesNum"] = std::to_string(in->inBytesNum);
		j["inAddress"] = in->inAddress;
	}
	else if (auto looper = dynamic_cast<const LooperNode*>(node))
	{
		j["type"] = "Looper";
		j["looperTimes"] = looper->looperTimes;
		j["looperBody"] = astToJson(looper->looperBody.get());
	}
	else if (auto function = dynamic_cast<const FunctionNode*>(node))
	{
		j["type"] = "Function";
		j["functionName"] = function->functionName;
		j["returnType"] = function->returnType;
		j["arguments"] = function->arguments;
		j["functionBody"] = astToJson(function->functionBody.get());
	}
	else if (auto returnNode = dynamic_cast<const ReturnNode*>(node))
	{
		j["type"] = "Return";
		j["returnValue"] = astToJson(returnNode->returnValue.get());
	}
	else if (auto expr = dynamic_cast<const ExprNode*>(node))
	{
		if (auto id = std::get_if<Identifier>(&expr->content))
		{
			j["type"] = "Identifier";
			j["name"] = id->name;
		}
		else if (auto lit = std::get_if<Literal>(&expr->content))
		{
			if (lit->type == ValueType::INT)
			{
				j["type"] = "Integer";
				j["value"] = std::get<int>(lit->value);
			} else if (lit->type == ValueType::ADDRESS)
			{
				j["type"] = "Address";
				j["value"] = std::get<uintptr_t>(lit->value);
			} else if (lit->type == ValueType::STRING)
			{
				j["type"] = "String";
				j["value"] = std::get<std::string>(lit->value);
			}
		}
		else if (auto bin = std::get_if<BinaryExpr>(&expr->content))
		{
			j["type"] = "BinaryExpr";
			j["operator"] = std::string(1,bin->op);
			j["left"] = astToJson(bin->lhs.get());
			j["right"] = astToJson(bin->rhs.get());
		}
		else if (auto eq = std::get_if<EqualityExpr>(&expr->content))
		{
			j["type"] = "EqualityExpr";
			j["operator"] = "==";
			j["left"] = astToJson(eq->lhs.get());
			j["right"] = astToJson(eq->rhs.get());
		}
		else if (auto callExpr = std::get_if<CallExpr>(&expr->content))
		{
			j["type"] = "CallExpr";
			j["functionName"] = callExpr->functionName;
			j["arguments"] = callExpr->arguments;
		}
	}
	else if (auto literal = dynamic_cast<const Literal*>(node))
	{
		if (literal->type == ValueType::INT)
		{
			j["type"] = "Integer";
			j["value"] = std::get<int>(literal->value);
		} else if (literal->type == ValueType::STRING)
		{
			j["type"] = "String";
			j["value"] = std::get<std::string>(literal->value);
		} else if (literal->type == ValueType::ADDRESS)
		{
			j["type"] = "Address";
			j["value"] = std::get<uintptr_t>(literal->value);
		}
	}
	else if (auto callExpr = dynamic_cast<const CallExpr*>(node))
	{
		j["type"] = "CallExpr";
		j["functionName"] = callExpr->functionName;
		j["arguments"] = callExpr->arguments;
	}
	else if (auto printableNode = dynamic_cast<const PrintableNode*>(node))
	{
		int index = 0;
		for (auto printable : printableNode->printableTokens)
		{
			json item;
			if (std::holds_alternative<std::string>(printable))
			{
				item["value"] = std::get<std::string>(printable);
				item["type"] = "String";
			} else if (std::holds_alternative<Identifier>(printable))
			{
				item["name"] = std::get<Identifier>(printable).name;
				item["type"] = "Identifier";
			}
			j[std::to_string(index)] = item;
			index++;
		}
	}
	else
	{
		j["type"] = "Unknown";
	}

	return j;
}