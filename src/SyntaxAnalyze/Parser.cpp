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
        else if (match(TokenType::IN))
        {
            program->statements.push_back(parseIn());
        }
        else if (match(TokenType::LOOPER))
        {
            program->statements.push_back(parseLooper());
        }
        else if (match(TokenType::FUNCTION))
        {
            program->statements.push_back(parseFunction());
        }
        else if (match(TokenType::RETURN))
        {
            program->statements.push_back(parseReturn());
        }
        else if (peek().type == TokenType::IDENTIFIER)
        {
            position_++;
            program->statements.push_back(parseAssignment());
        }
        else if (match(TokenType::RIGHT_BRACE)) return program;
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

    Token varType = consume();
    if (varType.type != TokenType::INT && varType.type != TokenType::BYTE)
    {
        throw std::runtime_error("Expected type specifier");
    }

    if (varType.type == TokenType::INT) decl->varType = ValueType::INT;
    else if (varType.type == TokenType::BYTE) decl->varType = ValueType::BYTE;

    Token id = consume(TokenType::IDENTIFIER, "Expected identifier");
    decl->identifier = id.lexeme;

    consume(TokenType::EQUAL, "Expected '=' after identifier");
    decl->initValue = parseExpression();

    consume(TokenType::AT, "Expected 'at' keyword");
    uintptr_t addr = stoi(consume(TokenType::HEXADDRESS, "Expected address").lexeme.substr(2, 3));
    decl->address = addr;

    return decl;
}

std::unique_ptr<Literal> Parser::parseLiteral()
{
    Token initValue = consume();
    std::variant<int, uintptr_t, std::string> value;
    ValueType type;
    std::string value_str = initValue.lexeme;
    if (initValue.type == TokenType::NUMBER)
    {
        type = ValueType::INT;
        int value_int = std::stoi(value_str);
        value = value_int;
    } else if (initValue.type == TokenType::STRING)
    {
        type = ValueType::STRING;
        value = value_str;
    }
    auto literal = std::make_unique<Literal>();
    literal->value = value;
    literal->type = type;
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
    auto printableNode = std::make_unique<PrintableNode>();
    while (true)
    {
        Token printable = consume();
        if (printable.type == TokenType::NEWLINE || printable.type == TokenType::END) break;
        if (printable.type == TokenType::PLUS) continue;
        if (printable.type == TokenType::STRING) printableNode->printableTokens.push_back(printable.lexeme);
        else if (printable.type == TokenType::IDENTIFIER)
            printableNode->printableTokens.push_back(Identifier{ printable.lexeme });
    }
    print->expr = std::move(printableNode);
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

std::unique_ptr<InNode> Parser::parseIn()
{
    consume(TokenType::LEFT_PAREN, "expect '(' after in");
    Token inBytesNum = consume();
    Token inAddress = consume();
    auto inNode = std::make_unique<InNode>();
    inNode->inBytesNum = stoi(inBytesNum.lexeme);
    inNode->inAddress = inAddress.lexeme.substr(2, inAddress.lexeme.length() - 2);
    return inNode;
}

std::unique_ptr<LooperNode> Parser::parseLooper()
{
    consume(TokenType::LEFT_PAREN, "expect '(' after looper");
    std::string looperTimes = consume(TokenType::NUMBER, "expect looper times").lexeme;
    consume(TokenType::RIGHT_PAREN, "expect ')' after looper");

    Token token = consume();
    if (token.type == TokenType::NEWLINE)
    {
        Token nextToken = consume();
        while (nextToken.type == TokenType::NEWLINE)
        {
            consume();
            continue;
        }
    }
    else if (token.type == TokenType::LEFT_BRACE);

    auto looperNode = std::make_unique<LooperNode>();
    looperNode->looperTimes = stoi(looperTimes);
    auto looperBody = parse();
    looperNode->looperBody = std::move(looperBody);
    return looperNode;
}

std::unique_ptr<FunctionNode> Parser::parseFunction()
{
    auto function = std::make_unique<FunctionNode>();
    std::string functionName = consume(TokenType::IDENTIFIER, "expect function name").lexeme;
    consume(TokenType::LEFT_PAREN, "expect '(' after function");
    std::vector<std::pair<std::string, std::string>> arguments;
    while (true)
    {
        Token token = consume();
        if (token.type == TokenType::RIGHT_PAREN) break;
        else if (token.type == TokenType::INT)
        {
            arguments.push_back(std::make_pair(token.lexeme, consume().lexeme));
            continue;
        }
    }
    consume(TokenType::RETURNARROW, "expect -> after arguments");
    std::string returnType = consume().lexeme;
    consume(TokenType::LEFT_BRACE, "expect { after return type");
    function->functionName = functionName;
    function->arguments = arguments;
    function->returnType = returnType;
    auto functionBody = parse();
    function->functionBody = std::move(functionBody);
    return function;
}

std::unique_ptr<ReturnNode> Parser::parseReturn()
{
    auto returnNode = std::make_unique<ReturnNode>();
    auto returnValue = parseExpression();
    returnNode->returnValue = std::move(returnValue);
    return returnNode;
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
        if (peek().type == TokenType::LEFT_PAREN) // identifier is a function name
        {
            consume(TokenType::LEFT_PAREN, "expected '(' before identifier");
            auto callExpr = make_unique<CallExpr>();
            callExpr->functionName = tokens_[position_ - 2].lexeme;

            while (true)
            {
                callExpr->arguments.push_back(*parseExpression());
                if (peek().type == TokenType::RIGHT_PAREN)
                {
                    advance();
                    break;
                }
                if (peek().type == TokenType::COMMA)
                {
                    consume();
                    continue;
                }
            }
            id->exprType = ExprType::CallExpr;
            id->content = *callExpr;
        }
        else // identifier is a variable
        {
            id->exprType = ExprType::IDENTIFIER;
            id->content = Identifier{tokens_[position_ - 1].lexeme};
        }
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

Token Parser::consume()
{
    if (position_ < tokens_.size()) {
        return tokens_[position_++];
    }
}


