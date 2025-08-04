//
// Created by hemin on 25-7-30.
//

#include "IRliveAnalyzor.h"

map<string, pair<int, int>> IRliveAnalyzor::calculateLiveRanges(const string& irCode)
{
	// result map tempReg -> (def index, last use index)
	map<string, pair<int, int>> liveRanges;

	// inner analyse data structure
	map<string, int> defs; // def index record
	map<string, vector<int>> uses; // use index record

	// regex : match tempReg(%t after num)
	regex tempRegPattern("%t\\d+");
	sregex_iterator endIterator;

	// spilt IR code to lines
	istringstream stream(irCode);
	string line;
	int instructionIndex = -1; // inst index start with 0

	// analyse lines
	while (getline(stream, line))
	{
		line = trim(line);

		// jmp not inst
		if (line.empty() ||
			line.find("declare") == 0 ||
			line.find("define") == 0 ||
			line == "}" ||
			line == "{")
		{
			continue;
		}

		// effect inst, increment index
		instructionIndex++;

		// check def index : whether you have assign operation
		size_t eqPos = line.find("=");
		if (eqPos != string::npos)
		{
			// extract = left part
			string leftPart = trim(line.substr(0, eqPos));

			// find tempReg def
			sregex_iterator leftIter(leftPart.begin(), leftPart.end(), tempRegPattern);
			if (leftIter != endIterator)
			{
				string reg = (*leftIter).str();
				defs[reg] = instructionIndex;

				// init use index list
				uses.insert({reg, vector<int>()});
			}
		}

		// find use index in line
		sregex_iterator iter(line.begin(), line.end(), tempRegPattern);
		for (; iter != endIterator; ++iter)
		{
			string reg = (*iter).str();

			// ignore self def index
			if (defs.count(reg) && defs[reg] == instructionIndex)
			{
				continue;
			}

			// record use index
			if (uses.find(reg) != uses.end())
			{
				uses[reg].push_back(instructionIndex);
			}
		}
	}

	// calculate live ranges
	for (const auto& def : defs)
	{
		string reg = def.first;
		int start = def.second;
		int end = start; // default end index is def index

		// if exist use index, then find bigger index
		if (uses.find(reg) != uses.end() && !uses[reg].empty())
		{
			end = *max_element(uses[reg].begin(), uses[reg].end());
		}

		liveRanges[reg] = make_pair(start, end);
	}

	return liveRanges;
}
