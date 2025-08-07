//
// Created by hemin on 25-8-5.
//

#include "TargetGenerator.h"

string TargetGenerator::formatReg(string reg, int bitWide)
{
	// not reg, return
	if (reg.empty() || reg[0] != '%') return reg;

	string body = reg.substr(1); // remove '%'
	string baseName;

	// format: remove subfix, get base name
	if (body.length() > 1 && isdigit(body[1]))
	{
		// extend reg(r8 - r15): remove subfix d/w/b
		char lastChar = body.back();
		if (lastChar == 'd' || lastChar == 'w' || lastChar == 'b')
		{
			string withoutSuffix = body.substr(0, body.length() - 1);
			if (withoutSuffix.length() > 1 && isdigit(withoutSuffix[1]))
				body = withoutSuffix;
		}
		baseName = body;
	} else
	{
		// traditional reg, convert ot rax/rbx/...  format
		if (body == "al" || body == "ah") baseName = "rax";
		else if (body == "bl" || body == "bh") baseName = "rbx";
		else if (body == "cl" || body == "ch") baseName = "rcx";
		else if (body == "dl" || body == "dh") baseName = "rdx";
		else if (body == "sil") baseName = "rsi";
		else if (body == "dil") baseName = "rdi";
		else if (body == "bpl") baseName = "rbp";
		else if (body == "spl") baseName = "rsp";
		// have prefix (ax/eax/rax)
		else if (body == "ax" || body == "eax" || body == "rax") baseName = "rax";
		else if (body == "bx" || body == "ebx" || body == "rbx") baseName = "rbx";
		else if (body == "cx" || body == "ecx" || body == "rcx") baseName = "rcx";
		else if (body == "dx" || body == "edx" || body == "rdx") baseName = "rdx";
		else if (body == "si" || body == "esi" || body == "rsi") baseName = "rsi";
		else if (body == "di" || body == "edi" || body == "rdi") baseName = "rdi";
		else if (body == "bp" || body == "ebp" || body == "rbp") baseName = "rbp";
		else if (body == "sp" || body == "esp" || body == "rsp") baseName = "rsp";
		else return reg;
	}

	// target bitWide 64: return format name
	if (bitWide == 64) return "%" + baseName;

	// deformat base in bitWide
	if (baseName.length() > 1 && isdigit(baseName[1]))
	{
		// extend reg: add d/w/b suffix
		switch (bitWide)
		{
			case 32: return "%" + baseName + "d";
			case 16: return "%" + baseName + "w";
			case 8: return "%" + baseName + "b";
			default: return "%" + baseName;
		}
	} else
	{
		// traditional reg: process 32/16/8 bit name
		string bareName = (baseName[0] == 'r') ? baseName.substr(1) : baseName;
		switch (bitWide)
		{
			case 32: return "%e" + bareName;
			case 16: return "%" + bareName;
			case 8:
				if		(bareName == "ax") return "%al";
				else if (bareName == "bx") return "%bl";
				else if (bareName == "cx") return "%cl";
				else if (bareName == "dx") return "%dl";
				else if (bareName == "si") return "%sil";
				else if (bareName == "di") return "%dil";
				else if (bareName == "bp") return "%bpl";
				else if (bareName == "sp") return "%spl";
				else return "%" + bareName + "l";
			default: return "%" + baseName;
		}
	}
}
