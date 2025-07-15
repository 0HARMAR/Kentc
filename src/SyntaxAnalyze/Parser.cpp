//
// Created by hemin on 25-7-9.
//

#include "Parser.h"

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens), position_(0) {};

std::unique_ptr<ProgramNode> Parser::parse()
{
    auto program = std::make_unique<ProgramNode>();

    while (position_ < tokens_.size())
    {
        if (match(TokenType::SET))
        {
            program->statements.push_back(parseDeclaration());
        }
        else if (match (TokenType::PRINT))
        {
            program->statements.push_back(parsePrint());
        }
        else if (match (TokenType::FIND))
        {
            program->statements.push_back(parseFind());
        }
        else if (match (TokenType::MOV))
        {
            program->statements.push_back(parseMov());
        }
        else if (peek().type == TokenType::IDENTIFIER)
        {
            position_++;
            program->statements.push_back(parseAssignment());
        }
        else
        {
            advance();
        }
    }
    return program;
}

bool Parser::match(TokenType expected)
{
    if (position_ < tokens_.size() &&
        tokens_[position_].type == expected)
    {
        advance();
        return true;
    }
    return false;
}

Token Parser::peek() const
{
    return position_ < tokens_.size() ?
    tokens_[position_] : Token{TokenType::END, ""};
}

void Parser::advance()
{
    if (position_ < tokens_.size()) position_++;
}

std::unique_ptr<DeclNode> Parser::parseDeclaration()
{
    auto decl = std::make_unique<DeclNode>();

    if (!match(TokenType::INT))
    {
        throw std::runtime_error("Expected type specifier");
    }

    Token id = consume(TokenType::IDENTIFIER, "Expected identifier");
    decl->identifier = id.lexeme;
    decl->varType = ValueType::INT;

    consume(TokenType::EQUAL, "Expected '=' after identifier");
    decl->initValue = parseLiteral();

    consume(TokenType::AT, "Expected 'at' keyword");
    uintptr_t addr = std::stoul(consume(TokenType::HEXADDRESS, "Expected address").lexeme, 0, 16);
    decl->address = addr;

    return decl;
}

std::unique_ptr<Literal> Parser::parseLiteral()
{
    Token initValue = consume(TokenType::NUMBER, "Expected integer");
    std::string value_str = initValue.lexeme;
    int value_int = std::stoi(value_str);
    auto literal = std::make_unique<Literal>();
    literal->value = value_int;
    literal->type = ValueType::INT;
    return literal;
}

std::unique_ptr<AssignNode> Parser::parseAssignment()
{
    auto assign = std::make_unique<AssignNode>();
    Token id = tokens_[position_ - 1]; // back to identifier

    assign->identifier = id.lexeme;
    consume(TokenType::EQUAL, "Expected '=' after assignment");
    assign->value = parseExpression();

    return assign;
}

std::unique_ptr<PrintNode> Parser::parsePrint()
{
    auto print = std::make_unique<PrintNode>();
    print->expr = parseExpression();
    return print;
}

std::unique_ptr<FindNode> Parser::parseFind()
{
    auto find = std::make_unique<FindNode>();
    find->target = parseExpression();
    return find;
}

std::unique_ptr<MovNode> Parser::parseMov()
{
    auto mov = std::make_unique<MovNode>();
    mov->src = parseExpression();
    consume(TokenType::TO, "Expected 'to' keyword");

    uintptr_t addr = std::stoul(consume(TokenType::HEXADDRESS, "Expected address").lexeme, 0, 16);
    mov->destAddr = addr;
    return mov;
}

std::unique_ptr<ExprNode> Parser::parseExpression()
{
    return parseAdditive();
}

std::unique_ptr<ExprNode> Parser::parseAdditive()
{
    auto left = parseMultiplicative();

    while (match(TokenType::PLUS) || match(TokenType::MINUS))
    {
        auto binary = std::make_unique<ExprNode>();
        binary->exprType = ExprType::BINARY_OP;

        char op = tokens_[position_ - 1].type == TokenType::PLUS ? '+' : '-';
        BinaryExpr binExpr{op, std::move(left),parseMultiplicative()};

        binary->content = binExpr;
        left = std::move(binary);
    }

    return left;
}

std::unique_ptr<ExprNode> Parser::parseMultiplicative()
{
    auto left = parsePrimary();

    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE))
    {
        auto binary = std::make_unique<ExprNode>();
        binary->exprType = ExprType::BINARY_OP;

        char op = tokens_[position_ - 1].type == TokenType::MULTIPLY ? '*' : '/';
        BinaryExpr binExpr{op,std::move(left),parsePrimary()};

        binary->content = binExpr;
        left = std::move(binary);
    }

    return left;
}
std::unique_ptr<ExprNode> Parser::parsePrimary()
{
    if (match(TokenType::NUMBER))
    {
        auto lit =  std::make_unique<ExprNode>();
        lit->exprType = ExprType::LITERAL;
        int value = std::stoi(tokens_[position_ - 1].lexeme);
        auto literal = std::make_unique<Literal>();
        literal->value = value;
        literal->type = ValueType::INT;
        lit->content = *literal;
        return lit;
    }
    else if (match(TokenType::HEXADDRESS))
    {
        auto lit = std::make_unique<ExprNode>();
        lit->exprType = ExprType::LITERAL;
        uintptr_t addr = std::stoul(tokens_[position_ - 1].lexeme, 0, 16);
        auto literal = std::make_unique<Literal>();
        literal->value = addr;
        literal->type = ValueType::ADDRESS;
        lit->content = *literal;
        return lit;
    }
    else if (match(TokenType::IDENTIFIER)) {
        auto id = std::make_unique<ExprNode>();
        id->exprType = ExprType::IDENTIFIER;
        id->content = Identifier{tokens_[position_ - 1].lexeme};
        return id;
    }
    throw std::runtime_error("Expected primary expression");
}

Token Parser::consume(TokenType expected, const std::string& error) {
    if (position_ < tokens_.size() && tokens_[position_].type == expected) {
        return tokens_[position_++];
    }
    throw std::runtime_error(error);
}

void Parser::printAST(const std::unique_ptr<ASTNode>& astRoot) {
    printAST(astRoot.get(), 0, true);
}

std::string Parser::getIndent(int indent, bool isLast)
{
    static std::string indentUnit = "|   ";

    if (indent == 0) return "";

    std::string result;
    for (int i = 1; i < indent; i++)
    {
        result += indentUnit;
    }
    result += isLast ? "└── " : "├── ";
    return result;
}

void Parser::printAddress(uintptr_t addr)
{
    std::cout << "0x" << std::hex << std::setw(8) <<
        std::setfill('0') << addr << std::dec;
}

void Parser::printAST(const ASTNode* node, int depth, bool isLast)
{
    if (!node) return;

    std::cout << getIndent(depth, isLast);

    if (dynamic_cast<const ProgramNode*>(node))
    {
        std::cout << "ProgramNode";
    }
    else if (auto decl = dynamic_cast<const DeclNode*>(node))
    {
        std::cout << "DeclNode (";
        std::cout << (decl->varType == ValueType::INT ? "int" : "addr");
        std::cout << decl->identifier << " @";
        printAddress(decl->address);
        std::cout << " = ";
    }
    else if (auto assign = dynamic_cast<const AssignNode*>(node))
    {
        std::cout << "AssignNode (" << assign->identifier << " = ";
    }
    else if (auto print = dynamic_cast<const PrintNode*>(node))
    {
        std::cout << "PrintNode (";
    }
    else if (auto find = dynamic_cast<const FindNode*>(node))
    {
        std::cout << "FindNode (";
    }
    else if (auto mov = dynamic_cast<const MovNode*>(node))
    {
        std::cout << "MovNode (src: ";
    }
    std::cout << std::endl;

    // process children
    const ProgramNode* program = dynamic_cast<const ProgramNode*>(node);
    const DeclNode* decl = dynamic_cast<const DeclNode*>(node);
    const AssignNode* assign = dynamic_cast<const AssignNode*>(node);
    const PrintNode* print = dynamic_cast<const PrintNode*>(node);
    const FindNode* find = dynamic_cast<const FindNode*>(node);
    const MovNode* mov = dynamic_cast<const MovNode*>(node);
    const ExprNode* expr = dynamic_cast<const ExprNode*>(node);

    int childCount = 0;
    ASTNode* child = nullptr;

    if (program)
    {
        childCount = program->statements.size();
        if (childCount > 0)
        {
            for (int i = 0; i < childCount; i++)
            {
                printAST(program->statements[i].get(), depth + 1, i == childCount - 1);
            }
        }
    }
    else if (decl && decl->initValue)
    {
        printAST(decl->initValue.get(), depth + 1, true);
    }
    else if (assign && assign->value)
    {
        printAST(assign->value.get(), depth + 1, true);
    }
    else if (print && print->expr)
    {
        printAST(print->expr.get(), depth + 1, true);
    }
    else if (find && find->target)
    {
        printAST(find->target.get(), depth + 1, true);
    }
    else if (mov)
    {
        if (mov->src)
        {
            printAST(mov->src.get(), depth + 1, true);
        }
        std::cout << getIndent(depth + 1, true) << "Dest: ";
        printAddress(mov->destAddr);
        std::cout << std::endl;
    }
    else if (expr)
    {
        if (auto id = std::get_if<Identifier>(&expr->content))
        {
            std::cout << getIndent(depth + 1, true) << "Identifier: " << id->name << std::endl;

        }
        else if (auto lit = std::get_if<Literal>(&expr->content))
        {
            if (lit->type == ValueType::INT)
            {
                std::cout << getIndent(depth + 1, true) << "Literal: "
                << std::get<int>(lit->value) << std::endl;
            } else
            {
                std::cout << getIndent(depth + 1, true) << "Address Literal: ";
                printAddress(std::get<uintptr_t>(lit->value));
                std::cout << std::endl;
            }
        }
        else if (auto bin = std::get_if<BinaryExpr>(&expr->content))
        {
            std::cout << getIndent(depth + 1, false) << "BinaryExpr (OP: "
            << bin->op << ")" << std::endl;
            printAST(bin->lhs.get(), depth + 2, false);
            printAST(bin->rhs.get(), depth + 2, true);
        }
    }
}
