//
// Created by Nwseb on 4/24/2025.
//

#ifndef INTERPRETER_EXECUTE_HPP
#define INTERPRETER_EXECUTE_HPP

#include <stack>
#include <string>
#include "ASTNode.hpp"
#include "SymbolTable.hpp"

class Execute {
public:
    Execute(ASTNode *rootAST, STNode *headST, std::string *fileName);
    ASTNode* getMain();
    ASTNode* beginForLoop();
    ASTNode* executeFunctionOrProcedure(ASTNode* current);
    void executeBlock();
    void skipBlock();
    void assignChar();
    void executeAssignment();
    void executeIfStatement();
    void printAndF();
    int evaluatePostfixInt(ASTNode* current);
    bool evaluatePostfixBool(ASTNode* current);

private:
    ASTNode *_rootAST,*_currentAST;
    STNode *_headST,*_currentST;
    std::stack<ASTNode*> executionStack;
    int currentScope = 0;
    int scopeCount = 0;
    std::string _fileName;
    bool ifConditionMet = false;
};


#endif //INTERPRETER_EXECUTE_HPP
