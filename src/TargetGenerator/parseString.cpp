//
// Created by hemin on 25-8-7.
//
#include "TargetGenerator.h"

vector<string> TargetGenerator::parseString(string str)
{
	vector<string> result;

	// use regex extract name, length, content
	smatch match;
	regex pattern(R"REGEX(@(\w+)\s*=\s*constant\s*\[(\d+)\s+x\s+i8\]\s+c"([^"]*)")REGEX");

	if (regex_search(str, match, pattern))
	{
		string name = match[1];
		int length = stoi(match[2]);
		string content = match[3];

		// process ESC
		content = content.substr(0, length - 1);

		stringTable[name] = make_pair(length, content);

		result.push_back(name);
		result.push_back(to_string(length));
		result.push_back(content);
	}

	return result;
}

void TargetGenerator::processString(const vector<string>& tokens)
{
	string name = tokens[0];
	string length = tokens[1];
	string content = tokens[2];

	asmLines.insert(asmLines.begin() + sectionIndex++, name + ":");
	asmLines.insert(asmLines.begin() + sectionIndex++, "	.string \"" + content + "\"");
}
