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
		j["varType"] = (decl->varType == ValueType::INT) ? "int" : "addr";
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
	}
	else if (auto literal = dynamic_cast<const Literal*>(node))
	{
		if (literal->type == ValueType::INT)
		{
			j["type"] = "Integer";
			j["value"] = std::get<int>(literal->value);
		} else
		{
			j["type"] = "Address";
			j["value"] = std::get<uintptr_t>(literal->value);
		}
	}
	else
	{
		j["type"] = "Unknown";
	}

	return j;
}