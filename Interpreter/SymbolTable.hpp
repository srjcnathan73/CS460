//
// Created by Nwseb on 3/31/2025.
//

#ifndef INTERPRETER_SYMBOLTABLE_HPP
#define INTERPRETER_SYMBOLTABLE_HPP

#include <string>
#include <vector>
#include <fstream>
#include "CSTNode.hpp"
#include "STNode.hpp"
#include "RDParser.hpp"

class SymbolTable {
public:
    SymbolTable(CSTNode *rootCST);
    void createSymbolTable();
    CSTNode* addVariable(CSTNode* current);
    CSTNode* addFunction(CSTNode* current);
    CSTNode* addProcedure(CSTNode* current);
    CSTNode* addParameterList(CSTNode* current, STNode *_parameterList);
    void addEntry(STNode* newEntry);
    void print();
    void outputToFile(std::string inputFileName);
    bool checkIsReservedWord(const std::string& word);
    bool checkIsDataType(const std::string& word);
    bool checkIsGlobalVar(const std::string& word);
    bool checkVarInSameScope(const std::string& word);
    STNode* getHeadOfSymbolTable(){return headSymbolTable;};


private:
    STNode *headSymbolTable, *previousSymbolTable, *tailSymbolTable, *parametersHead;
    CSTNode *_rootCST;
    CSTNode *currentCST;
    int currentScope = 0;
    int scopeCount = 0;
    std::vector<std::string> reservedWords = {"char","int","bool","procedure","function","printf","void"};
    std::vector<std::string>* _reservedWords = &reservedWords;
    std::vector<std::string> globalVars = {};
    std::vector<std::string>* _globalVars = &globalVars;
    std::vector<std::string> dataTypes = {"char","int","bool"};
    std::vector<std::string>* _dataTypes = &dataTypes;
};


#endif //INTERPRETER_SYMBOLTABLE_HPP
