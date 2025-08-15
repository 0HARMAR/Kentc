//
// Created by hemin on 25-8-10.
//

#include "../TargetGenerator.h"

void TargetGenerator::processDefine(Function_ function)
{
	string functionName = function.name;
	string returnType = function.returnType;
	vector<Variable> arguments = function.parameters;

	asmWriter.label(functionName);
	for (int i = 0; i < arguments.size(); i++)
	{
		// alloca real parameter register for argument
		registerAllocator.allocaArguments(arguments[i].name, i);
	}

}