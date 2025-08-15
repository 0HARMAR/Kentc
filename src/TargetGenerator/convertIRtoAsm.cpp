#include <complex>

#include "TargetGenerator.h"

bool isMain = true;
extern string runMode;
vector<string> TargetGenerator::convertIRToASM(const vector<string>& irLines)
{
	ostringstream ir_oss;
	for (size_t i = 0; i < irLines.size(); ++i)
	{
		ir_oss << irLines[i] << endl;
	}

	// add asm prefix
	addAsmLine("#Generated AT&T assembly from LLVM IR");
	addAsmLine("	.section .rodata");
	addAsmLine("	.text");
	addAsmLine("	.global	main");
	addAsmLine("	.type	main, @function");
	addAsmLine("	.extern	print_int");
	addAsmLine("	.extern	exit");
	addAsmLine("	.extern malloc_at");
	addAsmLine("	.extern in");
	addAsmLine("");
	addAsmLine("main:");
	addAsmLine("	pushq	%rbp");
	addAsmLine("	movq	%rsp, %rbp");

	// calculate stack size and add asm suffix
	int stackAlloc = staticProgramAnalyzer.analyze(ir_oss.str());
	asmWriter.sub("$" + std::to_string(stackAlloc), "%rsp", "q");

	asmWriter.mov("$9", "%rax", "q");
	asmWriter.mov("$0x600000", "%rdi", "q");
	asmWriter.mov("$0x1000000", "%rsi", "q");
	asmWriter.mov("$3", "%rdx", "q");
	asmWriter.mov("$0x32", "%r10", "q");
	asmWriter.Xor("%r8", "%r8", "q");
	asmWriter.Xor("%r9", "%r9", "q");
	asmWriter.syscall();

	map<string, pair<int, int>> liveRanges = irLiveAnalyzer.calculateLiveRanges(ir_oss.str());
	if (runMode == "DEV")
	{
		for (const auto& range : liveRanges)
		{
			std::cout << "Rigister " << range.first
			<< ": Def at " << range.second.first
			<< ", Last use at " << range.second.second
			<< ", Range [" << range.second.first
			<< ", " << range.second.second << "]\n";
		}
		std::cout << std::endl;
	}

	int irEffectLineNum = 0;
	// find function defs
	for (const auto& line : irLines)
	{
		istringstream iss(line);
		string token;
		vector<string> tokens;
		while (iss >> token)
		{
			tokens.push_back(token);
		}
		if (tokens.empty()) continue;
		if (tokens[0] == "define")
		{
			Function_ function = parseFunctionDef(trim(line));
			functions.push_back(function);

			// init function stack offset
			stackOffset[function.name] = 0;
 		}
	}

	for (const auto& line : irLines)
	{
		string trimmed = trim(line);
		if (trimmed.empty()) continue;

		// skip decl and metadata
		if (trimmed.find("declare") == 0 ||
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

		if (tokens[0] == "define")
		{
			string name = getFunctionName(trimmed);
			if (name == "main") continue;
			else isMain = false;
			Function_ function = parseFunctionDef(trimmed);
			processDefine(function);
			irEffectLineNum--; // not count define
			currentFunction = name;
		}

		if (tokens[0][0] == '@') // string define
		{
			tokens = parseString(trimmed);
			processString(tokens);
		}
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
			args.push_back(tokens[3]);
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
			if (tokens[0].substr(0, tokens[0].length() - 1) == "main")
			{
				asmWriter.label(tokens[0].substr(0, tokens[0].length() - 1) + '_' + currentFunction);
				asmWriter.push("%rbp");
				asmWriter.mov("%rsp", "%rbp", "q");
				asmWriter.sub("$256", "%rsp", "q");
			}
			else
			asmWriter.label(tokens[0].substr(0, tokens[0].length() - 1));
		}
		else if (tokens[2] == "getelementptr" && tokens.size() >= 3)
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[9].substr(1, tokens[9].length() - 2));
			processGetElementPtr(args);
		}
		else if (tokens[2] == "xor" and tokens.size() >= 3)
		{
			vector<string> args;
			args.push_back(tokens[0]);
			args.push_back(tokens[4].substr(0, tokens[4].length() - 1));
			args.push_back(tokens[5]);
			processXor(args);
		}
		else if (tokens[0] == "ret")
		{
			string returnType = tokens[1];
			string returnValue = tokens[2];

			string returnBitWide = bitWideToMovSubfix[stoi(returnType.substr(1))];

			if (returnValue[0] == '%')
			{
				string returnTempReg = returnValue;
				string returnRealReg = registerAllocator.getTempVarLocation(returnTempReg);
				asmWriter.mov(formatReg(returnRealReg, stoi(returnType.substr(1))),
					formatReg("%rax", stoi(returnType.substr(1))), returnBitWide);
			} else
			{
				asmWriter.mov('$' + returnValue, formatReg("%rax", stoi(returnType.substr(1))), returnBitWide);
			}
			asmWriter.leave();
			asmWriter.ret();
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

	addAsmLine("");
	addAsmLine("	.section	.note.GNU-stack,\"\",@progbits");

	// write register alloca history
	string path = R"(/mnt/c/Users/hemin/kentc/ELFBUILD/SSAs.map)";
	ofstream file(path);
	if (file.is_open())
	{
		for (auto history : registerAllocator.allocaHistory)
		{
			file << history << endl;
		}
		file.close();
	} else
	{
		cerr << "connot open file" << path << endl;
	}

	return asmLines;
}

bool TargetGenerator::isMemory(string op)
{
	size_t openParen = op.find('(');
	size_t closeParen = op.find(')');

	return (openParen != string::npos) &&
		(closeParen != string::npos) &&
			(openParen < closeParen);
}

string TargetGenerator::getFunctionName(string line)
{
	size_t at_pos = line.find("@");
	size_t left_paren = line.find("(");
	string functionName = line.substr(at_pos + 1, left_paren - at_pos - 1);

	return functionName;
}





