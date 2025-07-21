//
// Created by hemin on 25-7-21.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <string>
#include <vector>

using namespace std;

// operand type
enum class OperandType
{
	IMMEDIATE,
	REGISTER,
	MEMORY,
	LABEL,
};

// operand repr
struct Operand
{
	OperandType type;
	string reg;
	string base_reg;  // base register for memory reference
	int displacement; // offset
	long value; // immediate value
	string label; // label for jump or call

	// create immediate operand
	static Operand create_immediate(long value)
	{
		Operand op;
		op.type = OperandType::IMMEDIATE;
		op.value = value;
		return op;
	}

	// create register operand
	static Operand create_register(const string& reg)
	{
		Operand op;
		op.type = OperandType::REGISTER;
		op.reg = reg;
		return op;
	}

	// create memory operand
	static Operand create_memory(int disp, const string& base_reg)
	{
		Operand op;
		op.type = OperandType::MEMORY;
		op.displacement = disp;
		op.base_reg = base_reg;
		return op;
	}

	// create label operand
	static Operand create_label(const string& label)
	{
		Operand op;
		op.type = OperandType::LABEL;
		op.label = label;
		return op;
	}

	// convert to string repr
	string to_string() const
	{
		switch (type)
		{
			case OperandType::IMMEDIATE:
				return "$" + to_string(value);
			case OperandType::REGISTER:
				return "%" + reg;
			case OperandType::MEMORY:
				return to_string(displacement)  + "(% " + base_reg + ")";
			case OperandType::LABEL:
				return label;
			default:
				return "?";
		}
	}

	// compare operand equal
	bool operator==(const Operand& other) const
	{
		if (type != other.type) return false;

		switch (type)
		{
			case OperandType::IMMEDIATE:
				return value == other.value;
			case OperandType::REGISTER:
				return reg == other.reg;
			case OperandType::MEMORY:
				return displacement == other.displacement &&
					base_reg == other.base_reg;
			case OperandType::LABEL:
				return label == other.label;
			default:
				return false;
		}
	}
};

class Instruction {
public:
	string opcode;
	vector<Operand> operands;
	string comment;
	string label;

	Instruction(const string& op, const vector<Operand>& operands = {},
		const string& lb1 = "", const string& cmt = "")
			: opcode(op), operands(operands), comment(cmt), label(lb1) {}

	string to_string() const;
	bool is_store() const;
	bool is_load() const;
	bool is_call() const;
	bool is_exit() const;
	Operand get_dest() const;
	Operand get_src() const;
};



#endif //INSTRUCTION_H
