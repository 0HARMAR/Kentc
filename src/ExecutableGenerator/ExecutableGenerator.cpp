//
// Created by hemin on 25-7-23.
//

#include "ExecutableGenerator.h"

ExecutableGenerator::ExecutableGenerator(const string& elfFile, const string& buildRoot)
	: asmFile(elfFile + ".s"), objFile(elfFile + ".o"), elfFile(elfFile), buildRoot(buildRoot) {}

void ExecutableGenerator::generateExecutable(const string& asmCode, const vector<string>& linkLibs)
{
	// 1. create asm file
	writeAsmFile(asmCode);

	// 2. use as compile
	assembleCode();

	// 3. use ld create executable
	linkExecutable(linkLibs);
}

void ExecutableGenerator::writeAsmFile(const string& asmCode)
{
	ofstream out(buildRoot + asmFile);
	if (!out) throw runtime_error("Failed to create file." + asmFile);
	out << asmCode;
	out.flush();
	out.close();
}

void ExecutableGenerator::assembleCode()
{
	string cmd = "as --64 -g -o " + buildRoot + objFile + " " + buildRoot + asmFile;
	if (system(cmd.c_str()) != 0)
	{
		// filesystem::remove(asmFile); // clean up failed file
		throw runtime_error("Failed to assemble code. Command: " + cmd);
	}
}

void ExecutableGenerator::linkExecutable(const vector<string>& linkLibs)
{
	stringstream cmd;
	// use ld to link
	// cmd << "ld -m elf_x86_64 -e main -o " << buildRoot + "output/" + elfFile << " " << buildRoot + objFile;

	// use gcc to link
	cmd << "gcc -m64 -nostartfiles -e main -o " + buildRoot + "output/" + elfFile + " " + buildRoot + objFile;

	for (auto& lib : linkLibs)
	{
		if (!filesystem::exists(buildRoot + "lib/" + lib))
			throw runtime_error("Link library not found: " + lib);
		cmd << " " << buildRoot + "lib/" + lib;
	}

	if (system(cmd.str().c_str()) != 0)
	{
		cleanupTempFiles();
		throw runtime_error("Failed to link executable. Command: " + cmd.str());
	}

	cleanupTempFiles();
}

void ExecutableGenerator::cleanupTempFiles()
{
	filesystem::remove(asmFile);
	filesystem::remove(objFile);
}
