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

string runMode = "PROD";
int main(int argc, char* argv[])
{
	string srcPath;
	string outputPath;
	if (argc <= 2 || argv[1] == "-v")
	{
		std::cout << "kentc version v4.1.0\n"
		<< "from a Swedish band kent.\n";
		return 0;
	}
	if (argc >= 2)
	{
		srcPath = argv[1];
		for (int i = 1; i < argc; i++)
		{
			string arg = argv[i];
			if (arg == "-runMode")
			{
				runMode = argv[i + 1];
			}
			else if (arg == "-o")
			{
				outputPath = argv[i + 1];
			}
		}

	}

    // 打开源文件
	if (runMode == "DEV") srcPath = "../local-var.kent";
    std::ifstream src(srcPath);
    if (!src) throw std::runtime_error("Could not open file " + srcPath);
    std::ostringstream buffer;
    buffer << src.rdbuf();
    std::string src_str = buffer.str();

	// lexer analyze
    Lexer lexer(src_str);

    std::vector<Token> tokens = lexer.tokenize();

    // output tokens
    if (runMode == "DEV") {
    	std::cout << "\033[31mLexer analyze:\033[0m\n";
    	for (const Token& token : tokens)
    		{
        		std::cout << "Token: " << token.lexeme
                  << ", Type: " << lexer.tokenTypeToString(token.type) << '\n';
    		}
		std::cout << std::endl;
    }

	// parser analyze

	Parser parser(tokens);
    const auto ast = parser.parse();
	json ast_json = parser.astToJson(ast.get());
    if (runMode == "DEV")
    {
    	std::cout << "\033[32mParser analyze:\033[0m\n";
    	std::cout << ast_json.dump(2) << std::endl;
    }

	// IR generate
	IRGenerator irGenerator;
	std::string IR;
	irGenerator.generateIR(ast_json,IR);
	if (runMode == "DEV")
	{
		std::cout << "\033[33mIR generate:\033[0m\n";
		std::cout << IR << std::endl;
	}

	// ASM generate
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
	if (runMode == "DEV")
	{
		std::cout << "\033[34mASM generate:\033[0m\n";
		for (const string& line : ASMLines)
		{
			std::cout << line << std::endl;
		}
	}

	// optimize
	// AssemblyParser assemblyParser;
	// Function main = assemblyParser.parse(ASMLines);
	// Optimizer optimizer(main);
	// Function mainOptimized = optimizer.optimize();
	// mainOptimized.print();
	// std::cout << std::endl;

	// Executable generate
	// convert ASM lines to ASM string
	std::string ASM;
	for (const string& line : ASMLines)
	{
		ASM += line + "\n";
	}
	if (runMode == "DEV")
	{
		std::cout << "\033[35mExecutable generate:\033[0m\n";
		ExecutableGenerator executableGenerator;
		executableGenerator.generateExecutable(ASM);
	}
	else
	{
		ExecutableGenerator executableGenerator(srcPath.substr(0, srcPath.length() - 5), "./");
		executableGenerator.generateExecutable(ASM);
	}
    return 0;
}
