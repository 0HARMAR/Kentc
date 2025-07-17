#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "LexerAnalyze/Lexer.hpp"
#include "SyntaxAnalyze/Parser.h"
#include "IRGenerate/IRGenerator.h"
#include <windows.h>

#include "TargetGenerator/TargetGenerator.h"

int main()
{
	system("chcp 65001 > nul"); // 设置控制台编码为UTF-8

    // 打开v1.0.kent
    std::ifstream src("../v1.0.kent");
    if (!src) throw std::runtime_error("Could not open file v1.0.kent");
    std::ostringstream buffer;
    buffer << src.rdbuf();
    std::string src_str = buffer.str();

	// lexer analyze
	std::cout << "\033[31mLexer analyze:\033[0m\n";
    Lexer lexer(src_str);

    std::vector<Token> tokens = lexer.tokenize();

    // output tokens
    for (const Token& token : tokens)
    {
        std::cout << "Token: " << token.lexeme
                  << ", Type: " << lexer.tokenTypeToString(token.type) << '\n';
    }

	// parser analyze
	std::cout << "\033[32mParser analyze:\033[0m\n";

	Parser parser(tokens);
    const auto ast = parser.parse();
	json ast_json = parser.astToJson(ast.get());
	std::cout << ast_json.dump(2) << std::endl;

	// IR generate
	std::cout << "\033[33mIR generate:\033[0m\n";
	IRGenerator irGenerator;
	std::string IR;
	irGenerator.generateIR(ast_json,IR);
	std::cout << IR << std::endl;

	// ASM generate
	std::cout << "\033[34mASM generate:\033[0m\n";
	TargetGenerator targetGenerator;
	// convert IR string to IR lines
	std::vector<string> IRLines;
	std::string IRLine;
	std::istringstream iss(IR);
	while (std::getline(iss,IRLine))
	{
		IRLines.push_back(IRLine);
	}
	std::vector<string> ASMLines = targetGenerator.convertIRToASM(IRLines);
	for (const string& line : ASMLines)
	{
		std::cout << line << std::endl;
	}
    return 0;
}
