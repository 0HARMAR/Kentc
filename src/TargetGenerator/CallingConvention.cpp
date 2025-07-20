//
// Created by hemin on 25-7-20.
//

#include "CallingConvention.h"

vector<string> CallingConvention::callerSaved = {"rax", "rcx", "rdx", "rsi", "rdi",
			"r8", "r9", "r10", "r11"};
vector<string> CallingConvention::calleeSaved = {"rbx", "rbp", "r12", "r13", "r14", "r15"};
vector<string> CallingConvention::parameters = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
string CallingConvention::returnReg = "rax";

vector<string> CallingConvention::addCallerSave()
{
	vector<string> pushCallerSave;
	for (const auto& reg : callerSaved)
	{
		pushCallerSave.push_back("	push " + reg);
	}
	return pushCallerSave;
}

vector<string> CallingConvention::addCalleeSave()
{
	vector<string> pushCalleeSave;
	for (const auto& reg : calleeSaved)
	{
		pushCalleeSave.push_back("	push " + reg);
	}
	return pushCalleeSave;
}

vector<string> CallingConvention::restoreCallerSave()
{
	vector<string> popCallerSave;
	for (const auto& reg : callerSaved)
	{
		popCallerSave.push_back("	pop " + reg);
	}
	return popCallerSave;
}

vector<string> CallingConvention::restoreCalleeSave()
{
	vector<string> popCalleeSave;
	for (const auto& reg : calleeSaved)
	{
		popCalleeSave.push_back("	pop " + reg);
	}
	return popCalleeSave;
}
