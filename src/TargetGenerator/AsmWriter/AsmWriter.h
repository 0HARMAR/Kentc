//
// Created by hemin on 25-7-26.
//

#ifndef ASMWRITER_H
#define ASMWRITER_H

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>
#include <utility>
#include "../include/utils.h"

using namespace std;

class AsmWriter
{
private:
	vector<string>& lines;

	bool isValidReg(const string& reg) const;
public:
	AsmWriter() = default;
	AsmWriter(vector<string>& lines);

	void mov(const string& src, const string& dest, string bitWide);
	void movMemToMem(const string& src, const string& dest, const string& spilledTransitReg, int bitWide);
	void add(const string& src, const string& dest, string bitWide);
	void sub(const string& src, const string& dest, string bitWide);
	void mul(const string& src, const string& dest, string bitWide);
	void Xor(const string& src, const string& dest, string bitWide);
	void cmp(const string& src, const string& dest, string bitWide);
	void lea(const string& src, const string& dest, string bitWide);
	void set(const string& dest, string condition);
	void call(const string& target);
	void label(const string& label);
	void ret();
	void leave();
	void syscall();
	void jmp(const string& target, string condition = "");
	void push(const string& reg);
	void pop(const string& reg);
	void pushfq();
	void popfq();
	size_t size() const;
	void clear();
	string getContent() const;
};

#endif //ASMWRITER_H
