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
        else if (match(TokenType::SELECTOR))
        {
            program->statements.push_back(std::unique_ptr<ASTNode>(parseSelector().release()));
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

std::unique_ptr<SelectorNode> Parser::parseSelector()
{
    auto conditionalExpr = parseConditionalExpression();
    auto conditionalBody = parse();
    std::unique_ptr<SelectorNode> selector = std::make_unique<SelectorNode>();
    selector->conditionalExpr = std::move(conditionalExpr);
    selector->conditionalProgram = std::move(conditionalBody);
    return selector;
}

std::unique_ptr<ExprNode> Parser::parseConditionalExpression()
{
    consume(TokenType::LEFT_PAREN, "expected '(' before conditional expression");
    auto conditionalExpr = std::make_unique<ExprNode>();
    conditionalExpr->exprType = ExprType::EQUALITY;
    auto equalityExpr = std::make_unique<EqualityExpr>();
    equalityExpr->lhs = parseExpression();
    consume(TokenType::EQUALITY, "expect '==' in selector operation");
    equalityExpr->rhs = parseExpression();
    conditionalExpr->content = *equalityExpr;
    return conditionalExpr;
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
    else if (match(TokenType::STRING))
    {
        auto lit = std::make_unique<ExprNode>();
        lit->exprType = ExprType::LITERAL;
        std::string value = tokens_[position_ - 1].lexeme;
        auto literal = std::make_unique<Literal>();
        literal->value = value;
        literal->type = ValueType::STRING;
        lit->content = *literal;
        return lit;
    }
    throw std::runtime_error("Expected primary expression");
}

Token Parser::consume(TokenType expected, const std::string& error) {
    if (position_ < tokens_.size() && tokens_[position_].type == expected) {
        return tokens_[position_++];
    }
    throw std::runtime_error(error);
}


