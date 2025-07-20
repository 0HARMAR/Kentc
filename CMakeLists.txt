cmake_minimum_required(VERSION 3.31)
project(kentc)

set(CMAKE_CXX_STANDARD 20)

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
        src/TargetGenerator/RegisterAllocator.cpp
        src/TargetGenerator/RegisterAllocator.h
        src/StaticProgramAnalysis/StaticProgramAnalyzer.cpp
        src/StaticProgramAnalysis/StaticProgramAnalyzer.h
        src/TargetGenerator/HandleDivision.cpp)
