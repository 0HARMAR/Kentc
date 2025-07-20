//
// Created by hemin on 25-7-19.
//

#include "StaticProgramAnalyzer.h"

#include <regex>

StaticProgramAnalyzer::StaticProgramAnalyzer() : total_size(0), current_offset(0) {}

int StaticProgramAnalyzer::analyze(const string& ir_code)
{
	istringstream stream(ir_code);
	string line;

	while (getline(stream, line))
	{
		smatch match;
		if (regex_search(line, match, regex(R"%\w+ = alloca (\w+))")))
		{
			string type = match[1];
			int size = getTypeSize(type);

			// simulate memory allocation
			allocateMemory(memory_map.size(), size, "mem" + to_string(512 + memory_map.size()));
		}
		else if (regex_search(line, match, regex(R"(store \w+ (\d+), )")))
		{
			string value = match[1];
		}
	}
	return calculateFinalMemory();
}

int StaticProgramAnalyzer::getTypeSize(const string& type)
{
	if (type == "i32") return 4;
	if (type == "i64") return 8;
	if (type == "i16") return 2;
	if (type == "i8") return 1;
	return 4; // default size
}

void StaticProgramAnalyzer::allocateMemory(int id, int size, const string& name)
{
	memory_map.emplace_back(512 + id * size, size, name);
	total_size += size;
}

int StaticProgramAnalyzer::calculateFinalMemory()
{
	const int alignment = 16;
	total_size_aligned = (total_size + alignment - 1) / alignment * alignment;

	for (size_t i = 0; i < memory_map.size(); ++i)
	{
		for (size_t j = i + 1; j < memory_map.size(); ++j)
		{
			if (memory_map[i].end_address > memory_map[j].start_address &&
				memory_map[j].end_address > memory_map[i].start_address )
			{
				cout << "warning : memory overlap ("
						<< memory_map[i].var_name << " and "
						<< memory_map[j].var_name << ")" << std::endl;
			}
		}
	}

	return total_size_aligned;
}

int StaticProgramAnalyzer::entendStackSize()
{
	// TODO
	return 16; // default return min aligned stack size
}