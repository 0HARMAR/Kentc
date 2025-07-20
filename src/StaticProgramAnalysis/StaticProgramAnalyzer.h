//
// Created by hemin on 25-7-19.
//

#ifndef STATICPROGRAMANALYSIS_H
#define STATICPROGRAMANALYSIS_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
using namespace std;
struct MemoryEntry
{
	int start_address;
	int size;
	int end_address;
	string var_name;

	MemoryEntry(int addr, int sz, const string& name)
		: start_address(addr), size(sz), end_address(addr + sz), var_name(name) {}
};

class StaticProgramAnalyzer {
public:
	StaticProgramAnalyzer();
	int analyze(const string& ir_code);
	int entendStackSize();

private:
	vector<MemoryEntry> memory_map;
	int total_size;
	int total_size_aligned;
	int current_offset;

	int getTypeSize(const string& type);
	void allocateMemory(int id, int size, const string& name);
	int calculateFinalMemory();
};



#endif //STATICPROGRAMANALYSIS_H
