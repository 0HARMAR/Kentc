//
// Created by hemin on 25-7-10.
//

#ifndef TOKEN_H
#define TOKEN_H
#include "TokenType.h"
#include <string>
struct Token
{
    TokenType type;
    std::string lexeme;
    Token(TokenType t,const std::string& l) : type(t), lexeme(l) {}
};
#endif //TOKEN_H
