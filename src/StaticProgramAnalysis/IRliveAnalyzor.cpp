//
// Created by hemin on 25-7-30.
//

#include "IRliveAnalyzor.h"

map<string, pair<int, int>> IRliveAnalyzor::calculateLiveRanges(const string& irCode)
{
	// result map tempReg -> (def index, last use index)
	map<string, pair<int, int>> liveRanges;

	// inner analyse data structure
	map<string, pair<string, int>> defs; // def index record. tempReg -> (block, def index)
	map<string, vector<pair<string, int>>> uses; // use index record tempReg -> vector((block, def index))

	// regex : match tempReg(%t after num)
	regex tempRegPattern("%t\\d+");
	sregex_iterator endIterator;

	// current basic block
	string currentBlock = "main";

	// block end
	map<string, int> blockEnds;

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

		if (line.back() == ':')
		{
			blockEnds[currentBlock] = instructionIndex - 1;
			currentBlock = line.substr(0, line.size() - 1);
			continue;
		}

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
				defs[reg] = make_pair(currentBlock, instructionIndex);

				// init use index list
				uses.insert({reg, vector<pair<string, int>>()});
			}
		}

		// find use index in line
		sregex_iterator iter(line.begin(), line.end(), tempRegPattern);
		for (; iter != endIterator; ++iter)
		{
			string reg = (*iter).str();

			// ignore self def index
			if (defs.count(reg) && defs[reg].second == instructionIndex)
			{
				continue;
			}

			// record use index
			if (uses.find(reg) != uses.end())
			{
				uses[reg].push_back(make_pair(currentBlock, instructionIndex));
			}
		}
	}

	// calculate live ranges
	for (const auto& def : defs)
	{
		string name = def.first;
		string block = def.second.first;
		int defIndex = def.second.second;

		int useIndex;
		if (uses.count(name))
		{
			vector<pair<string, int>> useList = uses[name];
			auto maxPair = max_element(
				useList.begin(), useList.end(),
				[](const pair<string, int>& a, const pair<string, int>& b)
				{
					return a.second < b.second;
				});
			// use in other block
			if (maxPair->first != block) useIndex = blockEnds[maxPair->first];
			else useIndex = maxPair->second;
		}
		liveRanges[name] = make_pair(defIndex, useIndex);
	}

	return liveRanges;
}