#include <iostream>
#include <cctype>
#include <unordered_map>
#include "Lexer.hpp"

Lexer::Lexer(const std::string& input) : input(input), pos(0) {}

std::vector<Token> Lexer::tokenize()
{
    while (pos < input.size())
    {
        char current = input[pos];
        if (std::isspace(static_cast<unsigned char>(current)))
        {
            if (current == '\n')
            {
                tokens.push_back(Token(TokenType::NEWLINE,"\n"));
            }
            pos++;
        }
        else if (std::isalpha(static_cast<unsigned char>(current)))
        {
            lexIdentifier();
        }
        else if (std::isdigit(static_cast<unsigned char>(current)))
        {
            lexNumber();
        }
        else if (current == '=')
        {
            tokens.push_back(Token(TokenType::EQUAL,"="));
            pos++;
        }
        else if (current == '+')
        {
            tokens.push_back(Token(TokenType::PLUS,"+"));
            pos++;
        }
        else if (current == '-')
        {
            tokens.push_back(Token(TokenType::MINUS,"-"));
            pos++;
        }
        else if (current == '*')
        {
            tokens.push_back(Token(TokenType::MULTIPLY,"*"));
            pos++;
        }
        else if (current == '/')
        {
            tokens.push_back(Token(TokenType::DIVIDE,"/"));
            pos++;
        }
        else
        {
            pos++;
        }
    }
    return tokens;
}

// recognize identifier and key word
void Lexer::lexIdentifier()
{
    size_t start = pos;
    while (pos < input.size() &&
        (std::isalnum(static_cast<unsigned char>(input[pos]))) ||
        input[pos] == '_' ){
        pos++;
        }
    std::string word = input.substr(start,pos - start);

    // check key word
    static const std::unordered_map<std::string,TokenType> keywords = {
        {"set",TokenType::SET}, {"int",TokenType::INT},
        {"print",TokenType::PRINT}, {"find",TokenType::FIND},
        {"mov",TokenType::MOV}, {"to",TokenType::TO},
        {"at",TokenType::AT},
    };

    auto it = keywords.find(word);
    if (it != keywords.end())
    {
        tokens.push_back(Token(it -> second,word));
    } else
    {
        tokens.push_back(Token(TokenType::IDENTIFIER,word));
    }
}
// check number or hexadecimal address
void Lexer::lexNumber()
{
    size_t start = pos;
    // check hexadecimal address
    if (pos + 2 < input.size() &&
        input[pos] == '0' &&
        (input[pos + 1] == 'x' || input[pos + 1] == 'X'))
    {
        pos += 2;
        while (pos < input.size() &&
            std::isxdigit(static_cast<unsigned char>(input[pos])))
        {
            pos++;
        }
        tokens.push_back(Token(TokenType::HEXADDRESS,
            input.substr(start, pos - start)));
        return;
    }
        // handle decimal number
        while (pos < input.size() &&
            std::isdigit(static_cast<unsigned char>(input[pos])))
        {
            pos++;
        }
        tokens.push_back(Token(TokenType::NUMBER,
            input.substr(start,pos - start)));
}
std::string Lexer::tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::SET:        return "SET";
    case TokenType::INT:        return "INT";
    case TokenType::AT:         return "AT";
    case TokenType::IDENTIFIER: return "IDENTIFIER";
    case TokenType::EQUAL:      return "EQUAL";
    case TokenType::NUMBER:     return "NUMBER";
    case TokenType::HEXADDRESS: return "HEXADDRESS";
    case TokenType::PRINT:      return "PRINT";
    case TokenType::FIND:       return "FIND";
    case TokenType::MOV:        return "MOV";
    case TokenType::TO:         return "TO";
    case TokenType::PLUS:       return "PLUS";
    case TokenType::MINUS:      return "MINUS";
    case TokenType::MULTIPLY:   return "MULTIPLY";
    case TokenType::DIVIDE:     return "DIVIDE";
    case TokenType::NEWLINE:    return "NEWLINE";
    default:                    return "UNKNOWN";
    }
}
