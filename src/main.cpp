#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "LexerAnalyze/Lexer.hpp"
#include "SyntaxAnalyze/Parser.h"
#include "IRGenerate/IRGenerator.h"

#include "TargetGenerator/TargetGenerator.h"
#include "ExecutableGenerator/ExecutableGenerator.h"
#include "CompileOptimise/Optimizer.h"
#include "CompileOptimise/AssemblyParser.h"

int main()
{
    // 打开源文件
	std::string srcPath;
	srcPath = "../v1.0.kent";
    std::ifstream src(srcPath);
    if (!src) throw std::runtime_error("Could not open file " + srcPath);
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
	std::cout << std::endl;

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

	// optimize
	AssemblyParser assemblyParser;
	Function main = assemblyParser.parse(ASMLines);
	Optimizer optimizer(main);
	Function mainOptimized = optimizer.optimize();
	mainOptimized.print();
	std::cout << std::endl;

	// Executable generate
	std::cout << "\033[35mExecutable generate:\033[0m\n";
	ExecutableGenerator executableGenerator;
	// convert ASM lines to ASM string
	std::string ASM;
	for (const string& line : ASMLines)
	{
		ASM += line + "\n";
	}
	executableGenerator.generateExecutable(ASM);
    return 0;
}
