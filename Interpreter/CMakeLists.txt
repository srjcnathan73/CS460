cmake_minimum_required(VERSION 3.27)
project(Interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(Interpreter main.cpp
        Token.cpp
        Token.hpp
        Constants.hpp
        CommentRemover.cpp
        CommentRemover.hpp
        Tokenizer.cpp
        Tokenizer.hpp
        RCParser.cpp
        RCParser.hpp
        CSTNode.cpp
        CSTNode.hpp
        STNode.hpp
        STNode.cpp
        SymbolTable.cpp
        SymbolTable.hpp
)
