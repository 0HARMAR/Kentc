#pragma once
#include <string>
#include <vector>
#include "../../include/TokenType.h"
#include "../../include/Token.h"


class Lexer
{
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();
    std::string tokenTypeToString(TokenType type);

private:
    std::string input;
    size_t pos;
    std::vector<Token> tokens;
    void lexIdentifier();
    void lexNumber();
};