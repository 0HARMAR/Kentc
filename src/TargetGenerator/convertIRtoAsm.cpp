#include <complex>

#include "TargetGenerator.h"

vector<string> TargetGenerator::convertIRToASM(const vector<string>& irLines)
{
	// add asm prefix
	addAsmLine("#Generated AT&T assembly from LLVM IR");
	addAsmLine("	.text");
	addAsmLine("	.global	main");
	addAsmLine("	.type	main, @function");
	addAsmLine("	.extern	print_int");
	addAsmLine("	.extern	exit");
	addAsmLine("	.extern malloc_at");
	addAsmLine("");
	addAsmLine("main:");
	addAsmLine("	pushq	%rbp");
	addAsmLine("	movq	%rsp, %rbp");
	asmWriter.mov("$9", "%rax", "q");
	asmWriter.mov("$0x600000", "%rdi", "q");
	asmWriter.mov("$0x1000000", "%rsi", "q");
	asmWriter.mov("$3", "%rdx", "q");
	asmWriter.mov("$0x32", "%r10", "q");
	asmWriter.Xor("%r8", "%r8", "q");
	asmWriter.Xor("%r9", "%r9", "q");
	asmWriter.syscall();

	ostringstream ir_oss;
	for (size_t i = 0; i < irLines.size(); ++i)
	{
		ir_oss << irLines[i] << endl;
	}

	map<string, pair<int, int>> liveRanges = irLiveAnalyzer.calculateLiveRanges(ir_oss.str());
	for (const auto& range : liveRanges)
	{
		std::cout << "Rigister " << range.first
		<< ": Def at " << range.second.first
		<< ", Last use at " << range.second.second
		<< ", Range [" << range.second.first
		<< ", " << range.second.second << "]\n";
	}
	std::cout << std::endl;

	int irEffectLineNum = 0;
	for (const auto& line : irLines)
	{
		string trimmed = trim(line);
		if (trimmed.empty()) continue;

		// skip decl and metadata
		if (trimmed.find("declare") == 0 ||
			trimmed.find("define") == 0 ||
				trimmed.find("}") == 0)
		{
			continue;
		}

		// split instruction
		vector<string> tokens;

		// if call instruction
		if (trimmed.find("call") != string::npos)
		{
			tokens = parseCall(trimmed);
		} else
		{
			istringstream iss(trimmed);
			string token;
			while (iss >> token)
			{
				tokens.push_back(token);
			}
		}

		if (tokens.empty()) continue;

		// alloca instruction
		if (tokens.size() >= 3 and tokens[2].find("alloca") != string::npos)
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[3]);
			processAlloca(args);
		}
		else if (tokens[0] == "store")
		{
			vector<string> args;
			args.push_back(tokens[2]);
			args.push_back(tokens[1]);
			args.push_back(tokens[4]);
			processStore(args);
		}
		else if (tokens.size() >= 3 and tokens[2] == "load")
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[5]);
			processLoad(args);
		}
		else if (tokens.size() >= 3 and opMap.find(tokens[2]) != opMap.end())
		{
			string result = *tokens.rbegin();
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[2]);
			args.push_back(tokens[3]);
			args.push_back(tokens[4]);
			args.push_back(tokens[5]);

			processBinaryOp(args);
		}
		else if (tokens.size() >= 2 and (tokens[0] == "call" || tokens[1] == "call"))  // call instruction
		{
			vector<string> args;
			auto it = find(tokens.begin(), tokens.end(), "call");
			int callIndex = distance(tokens.begin(), it);
			args.push_back(tokens[callIndex + 2]);
			args.push_back(tokens[callIndex + 1]);
			if (callIndex != 0)
			args.push_back(tokens[callIndex - 1]);
			else args.push_back("");
			int funcNameIndex = callIndex + 2;
			for (int i = funcNameIndex + 1; i < tokens.size(); i += 2)
			{
				if (i + 1 >= tokens.size()) break;
				args.push_back(tokens[i]);
				args.push_back(tokens[i + 1]);
			}
			processCall(args);
		}
		else if (tokens.size() >= 3 and tokens[2] == "inttoptr")
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[4]);
			processInttoptr(args);
		}
		else if (tokens.size() >= 3 and tokens[2] == "icmp")
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[3]);
			args.push_back(tokens[4]);
			args.push_back(tokens[5].substr(0, tokens[5].length() - 1));
			args.push_back(tokens[6]);
			processIcmp(args);
		}
		else if (tokens[0] == "br")
		{
			vector<string> args;
			args.push_back(tokens[2].substr(0, tokens[2].length() - 1));
			args.push_back(tokens[4].substr(0, tokens[4].length() - 1));
			args.push_back(tokens[6].substr(1, tokens[6].length() - 1));
			processBr(args);
		}
		else if (tokens[0].back() == ':') // is a label
		{
			asmWriter.label(tokens[0].substr(0, tokens[0].length() - 1));
		}

		for (const auto& range : liveRanges)
		{
			const string& tempRegName = range.first;
			int end = range.second.second;

			// free real reg
			if (end == irEffectLineNum)
			{
				registerAllocator.freeReg(tempRegName);
			}
		}
		irEffectLineNum++;
	}

	// calculate stack size and add asm suffix

	int stackAlloc = staticProgramAnalyzer.analyze(ir_oss.str());
	asmLines.insert(asmLines.begin() + 11, "	subq	$" + to_string(stackAlloc) + ", %rsp");

	addAsmLine("");
	addAsmLine("	.section	.note.GNU-stack,\"\",@progbits");

	return asmLines;
}

vector<string> TargetGenerator::parseCall(string callStr)
{
	vector<string> tokens;
	string callBody = callStr;

	// find index of '='
	size_t equalPos = callStr.find('=');
	if (equalPos != string::npos)
	{
		// extract '=' left part
		string leftPart = callStr.substr(0, equalPos);
		// remove blank
		leftPart.erase(0, leftPart.find_first_not_of(" \t"));
		leftPart.erase(leftPart.find_last_not_of(" \t") + 1);
		tokens.push_back(leftPart);
		callBody = callStr.substr(equalPos + 1);
	}

	// remove blank
	callBody.erase(0, callBody.find_first_not_of(" \t"));

	// parse function call part
	istringstream iss(callBody);
	string token;

	while (iss >> token)
	{
		// process function name with @
		if (token.find('@') != string::npos)
		{
			// remove '@'
			size_t atPos = token.find('@');
			if (atPos != string::npos)
			{
				token.erase(atPos, 1);
			}
		}

		// process comma
		token.erase(remove(token.begin(), token.end(), ','), token.end());

		// remove parentheses
		if (token.find('(') != string::npos)
		{
			int leftParenthesesIndex = token.find('(');
			string functionName = token.substr(0, leftParenthesesIndex);
			string fistType = token.substr(leftParenthesesIndex + 1);
			tokens.push_back(functionName);
			tokens.push_back(fistType);
			continue;
		}
		token.erase(remove(token.begin(), token.end(), ')'), token.end());

		if (!token.empty())
		{
			tokens.push_back(token);
		}
	}

	return tokens;
}

bool TargetGenerator::isMemory(string op)
{
	size_t openParen = op.find('(');
	size_t closeParen = op.find(')');

	return (openParen != string::npos) &&
		(closeParen != string::npos) &&
			(openParen < closeParen);
}




