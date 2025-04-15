//
// Created by Nwseb on 2/24/2025.
//

#ifndef INTERPRETER_RDPARSER_HPP
#define INTERPRETER_RDPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "Token.hpp"
#include "CSTNode.hpp"
#include "ASTNode.hpp"
#include "SymbolTable.hpp"

class RDParser {
public:
    RDParser(std::vector<char> *fileBuffer, std::string fileName);
    void createCST();
    void createAST();
    void addASTLeaf(ASTNode* newEntry, bool rightSibling);
    void breadthFirstCSTPrint();
    void breadthFirstASTPrint();
    void breadthFirstCSTFilePrint(std::string inputFileName);
    void breadthFirstASTFilePrint(std::string inputFileName);
    bool checkForReservedWords(const std::string& word);
    bool checkIsDataType(const std::string& word);
    bool checkForNegativeInteger(const std::string& num);
    bool checkIsOperator(const std::string& type);
    CSTNode* addVariableDeclaration(CSTNode* current);
    CSTNode* addFunctionDeclaration(CSTNode* current);
    CSTNode* addProcedureDeclaration(CSTNode* current);
    CSTNode* addForLoopExpressions(CSTNode* current);
    CSTNode* addWhileOrIfExpression(CSTNode* current);
    CSTNode* createBoolExprPostfix(CSTNode* current);
    CSTNode* createIntExprPostfix(CSTNode* current);
    STNode* determineSTNode(CSTNode* current);
    CSTNode* getRootOfCST(){return rootCST;};


private:
    Token *headTokenizer, *previous;
    CSTNode *rootCST;
    ASTNode *rootAST,*currentAST;
    STNode *rootST,*currentST;
    int lastLine;
    int currentScope = 0;
    int scopeCount = 0;
    int forExpCount = 0;
    std::vector<std::string> reservedWords = {"char","int","procedure","function","printf","void","for","while","if","else"};
    std::vector<std::string>* _reservedWords = &reservedWords;
    std::vector<std::string> dataTypes = {"char","int","bool"};
    std::vector<std::string>* _dataTypes = &dataTypes;
    std::string _fileName;
};


#endif //INTERPRETER_RDPARSER_HPP
