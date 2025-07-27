//
// Created by hemin on 25-7-26.
//

#include "AsmWriter.h"

AsmWriter::AsmWriter(vector<string>& lines) : lines(lines) {}

bool AsmWriter::isValidReg(const string& reg) const
{
	static const char* regs[] = {"%rax", "%rbx", "%rcx", "%rdx", "%rdi", "%rsi", "%rbp", "%rsp",
									 "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
	for (const auto* r : regs)
	{
		if (reg == r) return true;
	}
	return false;
}

void AsmWriter::mov(const string& src, const string& dest, string bitWide)
{
	lines.push_back("	mov" + bitWide + "	" + src + ", " + dest);
}

void AsmWriter::movMemToMem(const string& src, const string& dest, const string& spilledTransitReg, string bitWide)
{
	push(spilledTransitReg);
	mov(src, spilledTransitReg, "q");
	mov(spilledTransitReg, dest, "q");
	pop(spilledTransitReg);
}

void AsmWriter::add(const string& src, const string& dest, string bitWide)
{
	lines.push_back("	add" + bitWide + "	" + src + ", " + dest);
}

void AsmWriter::sub(const string& src, const string& dest, string bitWide)
{
	lines.push_back("	sub" + bitWide + "	" + src + ", " + dest);
}

void AsmWriter::mul(const string& src, const string& dest, string bitWide)
{
	lines.push_back("	mul" + bitWide + "	" + src + ", " + dest);
}

void AsmWriter::Xor(const string& src, const string& dest, string bitWide)
{
	lines.push_back("	xor" + bitWide + "	" + src + ", " + dest);
}


void AsmWriter::call(const string& target)
{
	lines.push_back("	call	" + target);
}

void AsmWriter::ret()
{
	lines.push_back("	ret");
}

void AsmWriter::syscall()
{
	lines.push_back("	syscall");
}

void AsmWriter::push(const string& reg)
{
	if (!isValidReg(reg)) throw std::invalid_argument("Invalid reg: " + reg);
	lines.push_back("	push	" + reg);
}

void AsmWriter::pop(const string& reg)
{
	if (!isValidReg(reg)) throw std::invalid_argument("Invalid reg: " + reg);
	lines.push_back("	pop	" + reg);
}

size_t AsmWriter::size() const
{
	return lines.size();
}

void AsmWriter::clear()
{
	lines.clear();
}

string AsmWriter::getContent() const
{
	ostringstream oss;
	for (const auto& l : lines)
	{
		oss << l << endl;
	}
	return oss.str();
}






