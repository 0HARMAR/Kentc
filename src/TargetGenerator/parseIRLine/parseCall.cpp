//
// Created by hemin on 25-8-9.
//
#include "../TargetGenerator.h"

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
