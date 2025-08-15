cmake_minimum_required(VERSION 3.28.3)
project(kentc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

add_executable(kentc src/main.cpp
        src/LexerAnalyze/Lexer.cpp
        src/SyntaxAnalyze/parser.cpp
        src/SyntaxAnalyze/parser.h
        include/Token.h
        include/TokenType.h
        src/SyntaxAnalyze/AstToJson.cpp
        src/IRgenerate/IRGenerator.cpp
        src/IRgenerate/IRGenerator.h
        src/TargetGenerator/TargetGenerator.cpp
        src/TargetGenerator/TargetGenerator.h
        src/TargetGenerator/RegisterAllocator/RegisterAllocator.cpp
        src/TargetGenerator/RegisterAllocator/RegisterAllocator.h
        src/StaticProgramAnalysis/StaticProgramAnalyzer.cpp
        src/StaticProgramAnalysis/StaticProgramAnalyzer.h
        src/TargetGenerator/ProcessIRLine/HandleDivision.cpp
        src/TargetGenerator/CallingConvention.cpp
        src/TargetGenerator/CallingConvention.h
        src/CompileOptimise/Instruction.cpp
        src/CompileOptimise/Instruction.h
        src/CompileOptimise/BasicBlock.cpp
        src/CompileOptimise/BasicBlock.h
        src/CompileOptimise/Function.cpp
        src/CompileOptimise/Function.h
        src/CompileOptimise/Optimizer.cpp
        src/CompileOptimise/Optimizer.h
        src/CompileOptimise/deadCodeElimination.cpp
        src/CompileOptimise/constantPropagation.cpp
        src/CompileOptimise/callSequenceOptimization.cpp
        src/CompileOptimise/peepHoleOptimization.cpp
        src/CompileOptimise/statckOptimization.cpp
        src/CompileOptimise/AssemblyParser.cpp
        src/CompileOptimise/AssemblyParser.h
        src/ExecutableGenerator/ExecutableGenerator.cpp
        src/ExecutableGenerator/ExecutableGenerator.h
        src/memoryAllocator/memoryAllocator.cpp
        src/TargetGenerator/convertIRtoAsm.cpp
        src/TargetGenerator/ProcessIRLine/handleFunctionCall.cpp
        src/TargetGenerator/AsmWriter/AsmWriter.cpp
        src/TargetGenerator/AsmWriter/AsmWriter.h
        src/StaticProgramAnalysis/IRliveAnalyzor.cpp
        src/StaticProgramAnalysis/IRliveAnalyzor.h
        src/utils/stringUtils.cpp
        include/stringUtils.h
        src/TargetGenerator/parseIRLine/parseString.cpp
        src/TargetGenerator/ProcessIRLine/processGetElementPtr.cpp
        src/TargetGenerator/ProcessIRLine/processXor.cpp
        src/TargetGenerator/parseIRLine/parseCall.cpp
        src/TargetGenerator/parseIRLine/parseFunctionDef.cpp
        src/TargetGenerator/ProcessIRLine/processDefine.cpp
        src/TargetGenerator/ProcessIRLine/processCall.cpp
        src/TargetGenerator/ProcessIRLine/processBinaryOp.cpp
        src/IRgenerate/generateExpr.cpp
        src/TargetGenerator/include/utils.h
        src/TargetGenerator/include/utils.cpp
)

add_executable(malloc_test src/memoryAllocator/mallocTest.cpp
src/memoryAllocator/memoryAllocator.cpp)
