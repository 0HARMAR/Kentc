//
// Created by hemin on 25-7-20.
//

#include "CallingConvention.h"

vector<string> CallingConvention::callerSaved = {"rax", "rcx", "rdx", "rsi", "rdi",
			"r8", "r9", "r10", "r11"};
vector<string> CallingConvention::calleeSaved = {"rbx", "rbp", "r12", "r13", "r14", "r15"};
vector<string> CallingConvention::parameters = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
string CallingConvention::returnReg = "rax";
int CallingConvention::alignment = 16;

vector<string> CallingConvention::addCallerSave()
{
	vector<string> pushCallerSave;
	for (const auto& reg : callerSaved)
	{
		pushCallerSave.push_back("	push	%" + reg);
	}
	// check alignment
	pushCallerSave.push_back("	subq	$8, %rsp");

	return pushCallerSave;
}

vector<string> CallingConvention::addCalleeSave()
{
	vector<string> pushCalleeSave;
	for (const auto& reg : calleeSaved)
	{
		pushCalleeSave.push_back("	push	%" + reg);
	}
	pushCalleeSave.push_back("	subq	$8, %rsp");
	return pushCalleeSave;
}

vector<string> CallingConvention::restoreCallerSave()
{
	vector<string> popCallerSave;
	popCallerSave.push_back("	addq	$8, %rsp");
	for (auto it = callerSaved.rbegin(); it != callerSaved.rend(); ++it)
	{
		popCallerSave.push_back("	pop	%" + *it);
	}
	return popCallerSave;
}

vector<string> CallingConvention::restoreCalleeSave()
{
	vector<string> popCalleeSave;
	popCalleeSave.push_back("	addq	$8, %rsp");
	for (auto it = calleeSaved.rbegin(); it != calleeSaved.rend(); ++it)
	{
		popCalleeSave.push_back("	pop	%" + *it);
	}
	return popCalleeSave;
}
