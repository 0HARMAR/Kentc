//
// Created by hemin on 25-7-23.
//

#ifndef EXECUTABLEGENERATOR_H
#define EXECUTABLEGENERATOR_H

#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <filesystem>
#include <sstream>
#include <vector>
using namespace std;
class ExecutableGenerator {
public:
	ExecutableGenerator(const std::string& elfFile = "asm_build",
		const std::string& buildRoot = "/mnt/c/Users/hemin/kentc/ELFBUILD/");

	void generateExecutable(const string& asmCode,
		const vector<string>& linkLibs = {"print_int.o", "exit.o", "memoryAllocator.o",
		"in.o", "print_string.o", "print_hex.o"});

private:
	const string buildRoot;
	const string asmFile;
	const string objFile;
	const string elfFile;

	void writeAsmFile(const string& code);
	void assembleCode();
	void linkExecutable(const vector<string>& libs);
	void cleanupTempFiles();
};



#endif //EXECUTABLEGENERATOR_H
