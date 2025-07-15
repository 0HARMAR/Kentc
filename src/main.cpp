#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "LexerAnalyze/Lexer.hpp"
#include "SyntaxAnalyze/Parser.h"
#include <windows.h>
int main()
{
	system("chcp 65001 > nul"); // 设置控制台编码为UTF-8

    // 打开v1.0.kent
    std::ifstream src("../v1.0.kent");
    if (!src) throw std::runtime_error("Could not open file v1.0.kent");
    std::ostringstream buffer;
    buffer << src.rdbuf();
    std::string src_str = buffer.str();

    Lexer lexer(src_str);

    std::vector<Token> tokens = lexer.tokenize();

    // output tokens
    for (const Token& token : tokens)
    {
        std::cout << "Token: " << token.lexeme
                  << ", Type: " << lexer.tokenTypeToString(token.type) << '\n';
    }


	Parser parser(tokens);
    const auto ast = parser.parse();
	json ast_json = parser.astToJson(ast.get());
	std::cout << ast_json.dump(2) << std::endl;
    return 0;
}
