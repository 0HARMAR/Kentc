#include "TargetGenerator.h"

/**
 *
 * @param dividend e.g., "%t1", $3
 * @param divisor e.g., "%t2", $1
 * @param result result register
 */
void TargetGenerator::handleDivision(const string& dividend, const string& divisor, string& result)
{
	// force use %eax for dividend
	string eaxUser = registerAllocator.getVarInRegister(normalizeReg("%eax"));
	// %eax is used
	if (!eaxUser.empty())
	{
		if (eaxUser != dividend)
		{
			// make %eax free
			string new_reg = registerAllocator.spillRegister(eaxUser);
			addAsmLine("	movl	%eax, " + denormalizeReg(new_reg, 32));

			// make dividend binding to %eax, free old reg
			if (dividend[0] == '$') // dividend is an immediate
			{
				addAsmLine("	movl	" + dividend + ", " + "%eax");
			} else // dividend is a temp var
			{
				string old_reg = registerAllocator.allocReg(dividend, normalizeReg("%eax"));
				addAsmLine("	movl	" + denormalizeReg(old_reg, 32) + ", %eax");
			}

		}
	} else // %eax is free
	{
		// make dividend binding to %eax, free old reg
		if (dividend[0] == '$') // dividend is an immediate
		{
			addAsmLine("	movl	" + dividend + ", " + "%eax");
		} else // dividend is a temp var
		{
			string old_reg = registerAllocator.allocReg(dividend, normalizeReg("%eax"));
			addAsmLine("	movl	" + denormalizeReg(old_reg, 32) + ", %eax");
		}
	}

	// %edx for result extend, force make %edx free
	string edxUser = registerAllocator.getVarInRegister(normalizeReg("%edx"));
	if (!edxUser.empty())
	{
		registerAllocator.spillRegister(edxUser);
	}

	// make sure divisor in register
	string divisorReg;
	if (divisor[0] == '%') {} // temp var
	else  // immediate value
	{
		vector<string> exclude;
		exclude.push_back("%rdx");
		divisorReg = registerAllocator.allocReg(exclude);
		addAsmLine("	movl	" + divisor + ", " + denormalizeReg(divisorReg, 32));
	}

	// generate divide instruction
	addAsmLine("	cltd"); // sign extend %eax to %edx
	addAsmLine("	idivl	" + denormalizeReg(divisorReg, 32));

	string resultReg = registerAllocator.allocReg(result);
	addAsmLine("	movl	%eax, " + denormalizeReg(resultReg, 32)); // move result to result reg

}

// convert to 64 bit register
string TargetGenerator::normalizeReg(string reg)
{
	if (reg[0] == '%')
	{
		string noBitPrefixReg = reg.substr(2);
		// r8d/w/ - r15d/w/
		if (isdigit(noBitPrefixReg[0]))
		{
			reg.pop_back();
			return reg;
		} else
		{
			return "%r" + noBitPrefixReg;
		}
	} else return reg;

}

// convert 64 bit register to bitWide bit register
string TargetGenerator::denormalizeReg(string reg, int bitWide)
{
	if (reg[0] == '%')
	{
		string noBitPrefixReg = reg.substr(2);
		// %r8 - %r15
		if (isdigit(noBitPrefixReg[0]))
		{
			switch (bitWide)
			{
			case 32 : return reg + 'd';
			case 16 : return reg + 'w';
			case 8 : return reg + 'b';
			}
		}
		else
		{
			switch (bitWide)
			{
			case 32 : return "%e" + noBitPrefixReg;
			case 16 : return "%" + noBitPrefixReg;
			case 8 :
				if (noBitPrefixReg == "ax") return "%al";
				else if (noBitPrefixReg == "bx") return "%bl";
				else if (noBitPrefixReg == "cx") return "%cl";
				else if (noBitPrefixReg == "dx") return "%dl";
				else return "%" + noBitPrefixReg + "l";
			}
		}
	} else return reg;

}
