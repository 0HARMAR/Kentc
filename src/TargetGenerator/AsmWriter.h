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
	void movMemToMem(const string& src, const string& dest, const string& spilledTransitReg, string bitWide);
	void add(const string& src, const string& dest, string bitWide);
	void sub(const string& src, const string& dest, string bitWide);
	void mul(const string& src, const string& dest, string bitWide);
	void Xor(const string& src, const string& dest, string bitWide);
	void call(const string& target);
	void ret();
	void syscall();
	void push(const string& reg);
	void pop(const string& reg);
	size_t size() const;
	void clear();
	string getContent() const;
};

#endif //ASMWRITER_H
