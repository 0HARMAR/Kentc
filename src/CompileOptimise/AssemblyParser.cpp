//
// Created by hemin on 25-7-22.
//

#include "AssemblyParser.h"


Function AssemblyParser::parse(const vector<string>& lines)
{
	Function func("main");
	auto block = func.add_block("main");

	for (const string& line : lines)
	{
		if (line.empty() || line[0] == '#') continue;

		// skip pseudo-op
		if (trim(line)[0] == '.') continue;

		// process label
		size_t colon_pos = line.find(':');
		if (colon_pos != string::npos && colon_pos > 0 &&
			!isspace(line[colon_pos - 1]))
		{
			string label = line.substr(0, colon_pos);
			if (label == "main");
			else block = func.add_block(label);
			continue;
		}

		// parse instruction
		istringstream iss(line);
		string token;
		vector<string> tokens;

		while (iss >> token)
		{
			// remove comma
			if (!token.empty() && token.back() == ',')
			{
				token.pop_back();
			}
			tokens.push_back(token);
		}

		if (tokens.empty()) continue;

		// process label line
		if (tokens[0].back() == ':')
		{
			string label = tokens[0].substr(0, tokens[0].size() - 1);
			block = func.add_block(label);
			continue;
		}

		// opcode is first token
		string opcode = tokens[0];
		vector<Operand> operands;

		// process comments
		string comment;
		auto comment_pos = find_if(tokens.begin(), tokens.end(),
			[](const string& t) {return t == "#"; });
		if (comment_pos != tokens.end())
		{
			for (auto it = comment_pos + 1; it != tokens.end(); ++it)
			{
				comment += *it + " ";
			}
			tokens.erase(comment_pos, tokens.end());
		}

		// process operands
		for (size_t i = 1; i < tokens.size(); ++i)
		{
			Operand op;
			string tok = tokens[i];

			// immediate
			if (tok[0] == '$')
			{
				long value = stol(tok.substr(1));
				op = Operand::create_immediate(value);
			}
			// register
			else if (tok[0] == '%')
			{
				op = Operand::create_register(tok.substr(1));
			}
			// memory reference
			else if (tok.find('(') != string::npos && tok.find(')') != string::npos)
			{
				// find offset and base register
				size_t pos1 = tok.find('(');
				size_t pos2 = tok.find(')');
				int disp;
				if (pos1 == 0) disp = 0;
				else disp = stoi(tok.substr(0, pos1));
				string base = tok.substr(pos1 + 2, pos2 - pos1 - 2);
				op = Operand::create_memory(disp, base);

			}
			// label
			else
			{
				op = Operand::create_label(tok);
			}
			operands.push_back(op);
		}

		block->add_instruction(Instruction(opcode, operands, "", comment));
	}

	return func;
}
