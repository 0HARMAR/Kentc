//
// Created by hemin on 25-7-9.
//

#ifndef PARSER_H
#define PARSER_H
#include <string>
#include <variant>
#include <memory>
#include <iostream>
#include <vector>
#include "../../include/Token.h"
#include <iomanip>
#include <typeinfo>
#include "../../include/json.hpp"
using json = nlohmann::json;

// base enum type
enum class StmtType
{
    DECLARATION, ASSIGNMENT, PRINT, FIND, MOV, PROGRAM
};

enum class ExprType
{
    IDENTIFIER, LITERAL, BINARY_OP, EQUALITY
};

enum class ValueType
{
    INT, ADDRESS, STRING, BYTE
};

struct Identifier
{
    std::string name;
};

// AST Node
struct ASTNode
{
    virtual ~ASTNode() = default;
};

struct Literal : ASTNode
{
    std::variant<int, uintptr_t, std::string> value;
    ValueType type;
};

struct BinaryExpr
{
    char op;
    std::shared_ptr<ASTNode> lhs;
    std::shared_ptr<ASTNode> rhs;
};

struct EqualityExpr
{
    std::shared_ptr<ASTNode> lhs;
    std::shared_ptr<ASTNode> rhs;
};

struct DeclNode : ASTNode
{
    ValueType varType;
    std::string identifier;
    std::unique_ptr<ASTNode> initValue;
    uintptr_t address;
};

struct AssignNode : ASTNode
{
    std::string identifier;
    std::unique_ptr<ASTNode> value;
};

struct PrintNode : ASTNode
{
    std::unique_ptr<ASTNode> expr;
};

struct FindNode : ASTNode
{
    std::unique_ptr<ASTNode> target;
};

struct MovNode : ASTNode
{
    std::unique_ptr<ASTNode> src;
    uintptr_t destAddr;
};

struct ExprNode : ASTNode
{
    ExprType exprType;
    std::variant<Identifier,Literal,BinaryExpr, EqualityExpr> content;
};

struct ProgramNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;
};

struct SelectorNode : ASTNode
{
    std::unique_ptr<ExprNode> conditionalExpr;
    std::unique_ptr<ProgramNode> conditionalProgram;
};

struct InNode : ASTNode
{
    int inBytesNum;
    std::string inAddress;
};

struct PrintableNode : ASTNode
{
    std::vector<std::variant<std::string, Identifier>> printableTokens;
};

struct LooperNode : ASTNode
{
    int looperTimes;
    std::unique_ptr<ProgramNode> looperBody;
};


class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parse();
    void printAST(const std::unique_ptr<ASTNode>& astRoot);
    json astToJson(const ASTNode* node);

private:
    bool match(TokenType expected);
    Token peek() const;
    void advance();
    std::unique_ptr<DeclNode> parseDeclaration();
    std::unique_ptr<Literal> parseLiteral();
    std::unique_ptr<AssignNode> parseAssignment();
    std::unique_ptr<PrintNode> parsePrint();
    std::unique_ptr<FindNode> parseFind();
    std::unique_ptr<MovNode> parseMov();
    std::unique_ptr<SelectorNode> parseSelector();
    std::unique_ptr<InNode> parseIn();
    std::unique_ptr<LooperNode> parseLooper();
    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseConditionalExpression();
    std::unique_ptr<ExprNode> parseAdditive();
    std::unique_ptr<ExprNode> parseMultiplicative();
    std::unique_ptr<ExprNode> parsePrimary();
    Token consume(TokenType expected,const std::string& error);
    Token consume();
    void printAddress(uintptr_t addr);
    const std::vector<Token>& tokens_;
    size_t position_;
};



#endif //PARSER_H
