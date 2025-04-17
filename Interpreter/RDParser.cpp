//
// Created by Nwseb on 2/24/2025.
//
#include <iostream>
#include <queue>
#include <stack>
#include <iomanip>
#include <cstring>
#include "RDParser.hpp"
#include "Tokenizer.hpp"
#include "Constants.hpp"

RDParser::RDParser(std::vector<char> *fileBuffer, std::string fileName)
{
    Tokenizer tokenizer(fileBuffer);
    tokenizer.createTokens();
    headTokenizer = tokenizer.getHeadOfTokenList();
    _fileName = fileName;
    //tokenizer.print();
    createCST();
    SymbolTable symbolTable(rootCST);
    symbolTable.createSymbolTable();
    rootST = currentST = symbolTable.getHeadOfSymbolTable();
    createAST();
    //breadthFirstASTPrint();
    breadthFirstASTFilePrint(_fileName);
    //breadthFirstCSTFilePrint(_fileName);
    //symbolTable.print();
    //SymbolTable.outputToFile(_fileName);
}

void RDParser::createCST()
{
    rootCST = new CSTNode(headTokenizer);
    int currentLineNumber=rootCST->lineNumber();
    int parenCounter=0;
    auto tempNode = rootCST;
    auto newNode = rootCST;
    previous = headTokenizer;
    for(auto cur = headTokenizer->next(); cur; cur = cur->next() )
    {
        if(previous->tokenType() == "IDENTIFIER" && checkForReservedWords(previous->tokenValue())
        && cur->tokenType() == "IDENTIFIER" && checkForReservedWords(cur->tokenValue()) && cur->next()->tokenValue() == "(")
        {
            std::cerr << errorMessages[E_SYNTAX_ERROR] << cur->lineNumber() << ": reserved word \"" << cur->tokenValue() << "\" can not be used for the name of a function." << std::endl;
            exit(E_SYNTAX_ERROR);
        }
        if(previous->tokenType() == "IDENTIFIER" && checkForReservedWords(previous->tokenValue())
        && cur->tokenType() == "IDENTIFIER" && checkForReservedWords(cur->tokenValue()) && (cur->next()->tokenValue() == ";" || cur->next()->tokenValue() == "=" || cur->next()->tokenValue() == ")"))
        {
            std::cerr << errorMessages[E_SYNTAX_ERROR] << cur->lineNumber() << ": reserved word \"" << cur->tokenValue() << "\" can not be used for the name of a variable." << std::endl;
            exit(E_SYNTAX_ERROR);
        }
        if(previous->tokenType() == "L_BRACKET" && cur->tokenType() == "INTEGER" && checkForNegativeInteger(cur->tokenValue()) && cur->next()->tokenType() == "R_BRACKET" )
        {
            std::cerr << errorMessages[E_SYNTAX_ERROR] << cur->lineNumber() << ": array declaration size must be a positive integer." << std::endl;
            exit(E_SYNTAX_ERROR);
        }
        if(previous->tokenType() == "DOUBLE_QUOTE" && cur->tokenType() == "STRING" && cur->next()->tokenType() != "DOUBLE_QUOTE" )
        {
            std::cerr << errorMessages[E_SYNTAX_ERROR] << cur->lineNumber() << ": unterminated string quote." << std::endl;
            exit(E_SYNTAX_ERROR);
        }
        if(cur->tokenType() == "L_PAREN")
        {
            parenCounter++;
        }
        if(cur->tokenType() == "R_PAREN")
        {
            parenCounter--;
        }

        if(cur->lineNumber() != currentLineNumber)
        {
            currentLineNumber = cur->lineNumber();
        }
        if((previous->tokenType() == "SEMICOLON" && parenCounter == 0) || previous->tokenType() == "R_BRACE" || previous->tokenType() == "L_BRACE" || cur->tokenType() == "L_BRACE" || (previous->tokenType() == "R_PAREN" && cur->tokenType() != "SEMICOLON" && cur->tokenType() != "R_PAREN" && !checkIsOperator(cur->tokenType()) && parenCounter == 0))
        {
            newNode = new CSTNode(cur);
            tempNode->leftChild(newNode);
            tempNode = newNode;
        }
        else
        {
            newNode = new CSTNode(cur);
            tempNode->rightSibling(newNode);
            tempNode = newNode;
        }
        previous = cur;
    }
    lastLine=currentLineNumber;
}

void RDParser::createAST()
{
    rootAST = nullptr;
    if (!rootCST) return;
    std::queue<CSTNode*> queue;
    queue.push(rootCST);
    bool inFunctionOrProcedure = false;
    int curlyBraceCount = 0;
    while (!queue.empty()) {
        CSTNode* current = queue.front();
        queue.pop();
        if(inFunctionOrProcedure)
        {
            if(current->type() == "L_BRACE")
            {
                auto *astEntry = new ASTNode(current, nullptr, "BEGIN BLOCK", currentScope);
                addASTLeaf(astEntry,false);
                curlyBraceCount++;
            }
            if(current->type() == "R_BRACE")
            {
                auto *astEntry = new ASTNode(current, nullptr, "END BLOCK", currentScope);
                addASTLeaf(astEntry,false);
                curlyBraceCount--;
            }
            if (current->value() == "else")
            {
                auto *astEntry = new ASTNode(current, nullptr, "ELSE", currentScope);
                addASTLeaf(astEntry,false);
            }
        }
        if(curlyBraceCount == 0)
        {
            inFunctionOrProcedure = false;
            currentScope = 0;
        }
        if (current->rightSibling()) {
            queue.push(current->rightSibling());
            if (current->type() == "IDENTIFIER" && checkForReservedWords(current->value()) && checkIsDataType(current->value()))
            {
                current = addVariableDeclaration(current);
                queue.pop();
            }
            else if (current->type() == "IDENTIFIER" && checkForReservedWords(current->value()))
            {
                if (current->value() == "function")
                {
                    inFunctionOrProcedure = true;
                    current = addFunctionDeclaration(current);
                    queue.pop();
                }
                else if (current->value() == "procedure")
                {
                    inFunctionOrProcedure = true;
                    current = addProcedureDeclaration(current);
                    queue.pop();
                }
                else if (current->value() == "for")
                {
                    current = addForLoopExpressions(current);
                    queue.pop();
                }
                else if (current->value() == "while"||current->value() == "if")
                {
                    current = addWhileOrIfExpression(current);
                    queue.pop();
                }
                else if (current->value() == "printf"||current->value() == "return")
                {
                    current = addReturnOrPrintF(current);
                    queue.pop();
                }
            }
            else if (current->type() == "IDENTIFIER" && current->rightSibling()->type() == "L_PAREN")
            {
                current = addFunctionCall(current);
                queue.pop();
            }
            else if (current->type() == "IDENTIFIER")
            {
                if (current->value() != "printf" && current->rightSibling()->type() != "L_PAREN")
                {
                    current = addAssignment(current);
                    queue.pop();
                }
            }
        }
        if (current->leftChild())
        {
            queue.push(current->leftChild());
        }
    }
}

void RDParser::addASTLeaf(ASTNode* newEntry, bool rightSibling)
{
    if(rootAST == nullptr){
        rootAST = currentAST = newEntry;
    }
    else
    {
        if(rightSibling)
        {
            currentAST->rightSibling(newEntry);
            currentAST = newEntry;
        } else
        {
            currentAST->leftChild(newEntry);
            currentAST = newEntry;
        }
    }

}

bool RDParser::checkForReservedWords(const std::string& word)
{
    for(size_t i=0; i < _reservedWords->size(); i++){
        if(word == _reservedWords->at(i))
            return true;
    }
    return false;
}

bool RDParser::checkIsDataType(const std::string& word)
{
    for(size_t i=0; i < _dataTypes->size(); i++){
        if(word == _dataTypes->at(i))
            return true;
    }
    return false;
}

bool RDParser::checkForNegativeInteger(const std::string& num)
{
    int int_num = std::stoi(num);
    return int_num < 0;
}

bool RDParser::checkIsOperator(const std::string& type)
{
    return type == "MODULO"||type == "ASTERISK"||type == "PLUS"||type == "MINUS"||type == "DIVIDE"||type == "CARET"||type == "LT"||type == "GT"||type == "BOOLEAN_NOT";
}

CSTNode* RDParser::addVariableDeclaration(CSTNode* current)
{
    CSTNode* currentTemp = current;
    CSTNode* previousTemp = current;
    std::string idName;
    std::string label = "DECLARATION";
    std::string varValue;
    std::string varDataType;
    int varArraySize = 0;
    bool varIsArray = false;

    while(currentTemp->rightSibling()){
        if (currentTemp->type() == "IDENTIFIER" && checkIsDataType(currentTemp->value()))
        {
            varDataType = currentTemp->value();
            currentTemp = currentTemp->rightSibling();
            idName = currentTemp->value();
            if (currentTemp->rightSibling()->value() != "[")
            {
                auto *astEntry = new ASTNode(currentTemp, currentST, label, currentScope);
                currentST = currentST->next();
                addASTLeaf(astEntry,false);
            }
        }
        else if (previousTemp->value() == ",")
        {
            idName = currentTemp->value();
            auto *astEntry = new ASTNode(currentTemp, currentST, label, currentScope);
            currentST = currentST->next();
            addASTLeaf(astEntry,false);
        }
        else if (currentTemp->value() == "[")
        {
            currentTemp = currentTemp->rightSibling();
            if (currentTemp->type() == "INTEGER")
            {
                varArraySize = std::stoi(currentTemp->value());
                varIsArray = true;
                auto *astEntry = new ASTNode(currentTemp, currentST, label, currentScope);
                currentST = currentST->next();
                addASTLeaf(astEntry,false);
            }
        }
        else if (currentTemp->value() == ";")
        {
            return currentTemp;
        }
        if (currentTemp->rightSibling())
        {
            previousTemp = currentTemp;
            currentTemp = currentTemp->rightSibling();
        }
    }
    return currentTemp;
}

CSTNode* RDParser::addFunctionDeclaration(CSTNode* current)
{
    scopeCount++;
    currentScope = scopeCount;
    CSTNode* currentTemp = current;
    std::string idName;
    std::string label = "DECLARATION";
    std::string idType = "function";
    currentTemp = currentTemp->rightSibling();
    std::string varDataType = currentTemp->value();
    std::string varValue;
    currentTemp = currentTemp->rightSibling();
    idName = currentTemp->value();
    auto *astEntry = new ASTNode(currentTemp, currentST, label, currentScope);
    currentST = currentST->next();
    addASTLeaf(astEntry,false);
    while(currentTemp->rightSibling())
    {
        currentTemp = currentTemp->rightSibling();
    }
    return currentTemp;
}

CSTNode* RDParser::addProcedureDeclaration(CSTNode* current)
{
    scopeCount++;
    currentScope = scopeCount;
    CSTNode* currentTemp = current;
    std::string idName;
    std::string idType = "procedure";
    std::string label = "DECLARATION";
    std::string varDataType = "N/A";
    std::string varValue;
    currentTemp = currentTemp->rightSibling();
    idName = currentTemp->value();
    currentTemp = currentTemp->rightSibling();

    auto *astEntry = new ASTNode(currentTemp, currentST, label, currentScope);
    currentST = currentST->next();
    addASTLeaf(astEntry,false);
    while(currentTemp->rightSibling())
    {
        currentTemp = currentTemp->rightSibling();
    }
    return currentTemp;
}

CSTNode* RDParser::addForLoopExpressions(CSTNode* current)
{
    forExpCount++;
    CSTNode* currentTemp = current;
    std::string label = "FOR EXPRESSION " + std::to_string(forExpCount);
    currentTemp = currentTemp->rightSibling();//(
    currentTemp = currentTemp->rightSibling();
    STNode* foundST = determineSTNode(currentTemp);
    auto *astEntry = new ASTNode(currentTemp, foundST, label, currentScope);
    addASTLeaf(astEntry,false);
    //currentTemp = currentTemp->rightSibling();
    currentTemp = createIntExprPostfix(currentTemp);
    forExpCount++;
    label = "FOR EXPRESSION " + std::to_string(forExpCount);
    currentTemp = currentTemp->rightSibling();
    astEntry = new ASTNode(currentTemp, foundST, label, currentScope);
    addASTLeaf(astEntry,false);
    currentTemp = currentTemp->rightSibling();
    currentTemp = createBoolExprPostfix(currentTemp);
    forExpCount++;
    label = "FOR EXPRESSION " + std::to_string(forExpCount);
    currentTemp = currentTemp->rightSibling();
    astEntry = new ASTNode(currentTemp, foundST, label, currentScope);
    addASTLeaf(astEntry,false);
    currentTemp = createIntExprPostfix(currentTemp);
    return currentTemp;
}

CSTNode* RDParser::addAssignment(CSTNode* current)
{
    CSTNode* currentTemp = current;
    std::string label = "ASSIGNMENT";
    STNode* foundST = determineSTNode(currentTemp);
    if(foundST == nullptr)
    {
        std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": undeclared variable, procedure or function." << std::endl;
        exit(E_SYNTAX_ERROR);
    }
    auto *astEntry = new ASTNode(currentTemp, foundST, label, currentScope);
    addASTLeaf(astEntry,false);
    if(foundST->variableDataType() == "int")
    {
        currentTemp = createIntExprPostfix(currentTemp);
    }
    else if(foundST->variableDataType() == "bool")
    {
        currentTemp = currentTemp->rightSibling();
        auto *astAssignmentOp = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
        currentTemp = currentTemp->rightSibling();
        currentTemp = createBoolExprPostfix(currentTemp);
        addASTLeaf(astAssignmentOp,true);
    }
    else if(foundST->variableDataType() == "char")
    {
        astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
        addASTLeaf(astEntry,true);
        currentTemp = currentTemp->rightSibling();
        if(currentTemp->type()=="L_BRACKET")
        {
            for(size_t i=0;i<3;i++)
            {
                astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
                addASTLeaf(astEntry,true);
                currentTemp = currentTemp->rightSibling();
            }
        }
        auto *astAssignmentOp = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
        currentTemp = currentTemp->rightSibling();
        while(currentTemp->type() != "SEMICOLON"){
            astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
            addASTLeaf(astEntry,true);
            currentTemp = currentTemp->rightSibling();
        }
        addASTLeaf(astAssignmentOp,true);
    }
    return currentTemp;
}


CSTNode* RDParser::addReturnOrPrintF(CSTNode *current)
{
    CSTNode* currentTemp = current;
    std::string label = "PRINTF";
    if (currentTemp->value() == "return")
    {
        label = "RETURN";
    }
    auto *astEntry = new ASTNode(currentTemp, nullptr, label, currentScope);
    addASTLeaf(astEntry,false);
    currentTemp = currentTemp->rightSibling();
    STNode* foundST;
    while(currentTemp->type() != "SEMICOLON")
    {
        if (currentTemp->type() != "L_PAREN" && currentTemp->type() != "R_PAREN" && currentTemp->type() != "COMMA" && currentTemp->type() != "DOUBLE_QUOTE")
        {
            foundST = determineSTNode(currentTemp);
            astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
            addASTLeaf(astEntry,true);
        }
        currentTemp = currentTemp->rightSibling();
    }
    return currentTemp;
}


CSTNode* RDParser::addFunctionCall(CSTNode *current)
{
    CSTNode* currentTemp = current;
    std::string label = "CALL";
    STNode* foundST = determineSTNode(currentTemp);
    auto *astEntry = new ASTNode(currentTemp, foundST, label, currentScope);
    addASTLeaf(astEntry,false);
    currentTemp = currentTemp->rightSibling();
    while(currentTemp->type() != "SEMICOLON")
    {
        if (currentTemp->type() != "L_PAREN" && currentTemp->type() != "R_PAREN" && currentTemp->type() != "COMMA" && currentTemp->type() != "DOUBLE_QUOTE")
        {
            foundST = determineSTNode(currentTemp);
            astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
            addASTLeaf(astEntry,true);
        }
        currentTemp = currentTemp->rightSibling();
    }
    return currentTemp;
}

CSTNode* RDParser::addWhileOrIfExpression(CSTNode* current)
{
    CSTNode* currentTemp = current;
    std::string label = "WHILE";
    if (currentTemp->value() == "if")
    {
        label = "IF";
    }
    auto *astEntry = new ASTNode(currentTemp, nullptr, label, currentScope);
    addASTLeaf(astEntry,false);
    currentTemp = currentTemp->rightSibling();
    currentTemp = createBoolExprPostfix(currentTemp);
    return currentTemp;
}

CSTNode* RDParser::createBoolExprPostfix(CSTNode* current)
{
    CSTNode* currentTemp = current;
    STNode* foundST;
    std::stack<CSTNode*> cstStack;
    bool finished;
    int parenCount;
    while(currentTemp->type() != "SEMICOLON" && currentTemp->rightSibling())
    {
        if ((currentTemp->type() == "INTEGER") || (currentTemp->type() == "IDENTIFIER") || (currentTemp->type() == "SINGLE_QUOTE") || (currentTemp->type() == "DOUBLE_QUOTE") ||
            (currentTemp->type() == "STRING") || (currentTemp->type() == "L_BRACKET") || (currentTemp->type() == "R_BRACKET"))
        {
            if(currentTemp->type() == "IDENTIFIER")
                foundST = determineSTNode(currentTemp);
            else
                foundST = nullptr;
            auto *astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
            addASTLeaf(astEntry,true);
        }
        else
        {
            if (currentTemp->type() == "L_PAREN")
            {
                parenCount++;
                cstStack.push(currentTemp);
            }
            else
            {
                if (currentTemp->type() == "R_PAREN")
                {
                    finished = false;
                    while (!finished)
                    {
                        if (cstStack.top()->type() == "L_PAREN")
                        {
                            cstStack.pop();
                            finished = true;
                        }
                        else
                        {
                            auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                            addASTLeaf(astEntry,true);
                            cstStack.pop();

                            finished = true;
                        }
                    }
                }
                else
                {
                    if ((currentTemp->type() == "BOOLEAN_EQUAL") || (currentTemp->type() == "NOT_EQUAL") || (currentTemp->type() == "LT") || (currentTemp->type() == "GT") ||
                        (currentTemp->type() == "LT_EQUAL") || (currentTemp->type() == "GT_EQUAL") || (currentTemp->type() == "BOOLEAN_AND") || (currentTemp->type() == "BOOLEAN_OR")   ||
                        (currentTemp->type() == "BOOLEAN_NOT") || (currentTemp->type() == "PLUS") || (currentTemp->type() == "MINUS") || (currentTemp->type() == "ASTERISK")     ||
                        (currentTemp->type() == "DIVIDE") || (currentTemp->type() == "MODULO"))
                    {
                        if (cstStack.empty())
                        {
                            cstStack.push(currentTemp);
                        }
                        else
                        {
                            if (cstStack.top()->type() == "L_PAREN")
                            {
                                cstStack.push(currentTemp);
                            }
                            else
                            {
                                if (currentTemp->type() == "BOOLEAN_NOT")
                                {
                                    finished = false;
                                    while (!finished)
                                    {
                                        if (!cstStack.empty())
                                        {
                                            if (cstStack.top()->type() == "BOOLEAN_NOT")
                                            {
                                                auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                addASTLeaf(astEntry,true);
                                                cstStack.pop();
                                            }
                                            else
                                            {
                                                cstStack.push(currentTemp);
                                                finished = true;
                                            }
                                        }
                                        else
                                        {
                                            cstStack.push(currentTemp);
                                            finished = true;
                                        }
                                    }
                                }
                                else
                                {
                                    if ((currentTemp->type() == "ASTERISK") || (currentTemp->type() == "DIVIDE") || (currentTemp->type() == "MODULO"))
                                    {
                                        finished = false;
                                        while (!finished)
                                        {
                                            if (!cstStack.empty())
                                            {
                                                if ((cstStack.top()->type() == "BOOLEAN_NOT") || (cstStack.top()->type() == "ASTERISK") || (cstStack.top()->type() == "DIVIDE") || (cstStack.top()->type() == "MODULO"))
                                                {
                                                    auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                    addASTLeaf(astEntry,true);
                                                    cstStack.pop();
                                                }
                                                else
                                                {
                                                    cstStack.push(currentTemp);
                                                    finished = true;
                                                }
                                            }
                                            else
                                            {
                                                cstStack.push(currentTemp);
                                                finished = true;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if ((currentTemp->type() == "PLUS") || (currentTemp->type() == "MINUS"))
                                        {
                                            finished = false;
                                            while (!finished)
                                            {
                                                if (!cstStack.empty())
                                                {
                                                    if ((cstStack.top()->type() == "BOOLEAN_NOT") || (cstStack.top()->type() == "ASTERISK") ||
                                                        (cstStack.top()->type() == "DIVIDE")      || (cstStack.top()->type() == "MODULO")   ||
                                                        (cstStack.top()->type() == "PLUS")        || (cstStack.top()->type() == "MINUS"))
                                                    {
                                                        auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                        addASTLeaf(astEntry,true);
                                                        cstStack.pop();
                                                    }
                                                    else
                                                    {
                                                        cstStack.push(currentTemp);
                                                        finished = true;
                                                    }
                                                }
                                                else
                                                {
                                                    cstStack.push(currentTemp);
                                                    finished = true;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if ((currentTemp->type() == "BOOLEAN_EQUAL")      || (currentTemp->type() == "NOT_EQUAL") || (currentTemp->type() == "LT") || (currentTemp->type() == "GT") ||
                                                (currentTemp->type() == "LT_EQUAL") || (currentTemp->type() == "GT_EQUAL"))
                                            {
                                                finished = false;
                                                while (!finished)
                                                {
                                                    if (!cstStack.empty())
                                                    {
                                                        if ((cstStack.top()->type() == "BOOLEAN_NOT")        || (cstStack.top()->type() == "ASTERISK")              ||
                                                            (cstStack.top()->type() == "DIVIDE")             || (cstStack.top()->type() == "MODULO")                ||
                                                            (cstStack.top()->type() == "PLUS")               || (cstStack.top()->type() == "MINUS")                 ||
                                                            (cstStack.top()->type() == "BOOLEAN_EQUAL")      || (cstStack.top()->type() == "NOT_EQUAL")             ||
                                                            (cstStack.top()->type() == "LT")          || (cstStack.top()->type() == "GT")          ||
                                                            (cstStack.top()->type() == "LT_EQUAL") || (cstStack.top()->type() == "GT_EQUAL") ||
                                                            (cstStack.top()->type() == "BOOLEAN_NOT"))
                                                        {
                                                            auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                            addASTLeaf(astEntry,true);
                                                            cstStack.pop();
                                                        }
                                                        else
                                                        {
                                                            cstStack.push(currentTemp);
                                                            finished = true;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        cstStack.push(currentTemp);
                                                        finished = true;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (currentTemp->type() == "BOOLEAN_AND")
                                                {
                                                    finished = false;
                                                    while (!finished)
                                                    {
                                                        if (!cstStack.empty())
                                                        {
                                                            if ((cstStack.top()->type() == "BOOLEAN_NOT")        || (cstStack.top()->type() == "ASTERISK")              ||
                                                                (cstStack.top()->type() == "DIVIDE")             || (cstStack.top()->type() == "MODULO")                ||
                                                                (cstStack.top()->type() == "PLUS")               || (cstStack.top()->type() == "MINUS")                 ||
                                                                (cstStack.top()->type() == "BOOLEAN_EQUAL")      || (cstStack.top()->type() == "NOT_EQUAL")             ||
                                                                (cstStack.top()->type() == "LT")          || (cstStack.top()->type() == "GT")          ||
                                                                (cstStack.top()->type() == "LT_EQUAL") || (cstStack.top()->type() == "GT_EQUAL") ||
                                                                (cstStack.top()->type() == "BOOLEAN_AND")        || (cstStack.top()->type() == "BOOLEAN_NOT"))
                                                            {
                                                                auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                                addASTLeaf(astEntry,true);
                                                                cstStack.pop();
                                                            }
                                                            else
                                                            {
                                                                cstStack.push(currentTemp);
                                                                finished = true;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            cstStack.push(currentTemp);
                                                            finished = true;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if (currentTemp->type() == "BOOLEAN_OR")
                                                    {
                                                        finished = false;
                                                        while (!finished)
                                                        {
                                                            if (!cstStack.empty())
                                                            {
                                                                if ((cstStack.top()->type() == "BOOLEAN_NOT")        || (cstStack.top()->type() == "ASTERISK")              ||
                                                                    (cstStack.top()->type() == "DIVIDE")             || (cstStack.top()->type() == "MODULO")                ||
                                                                    (cstStack.top()->type() == "PLUS")               || (cstStack.top()->type() == "MINUS")                 ||
                                                                    (cstStack.top()->type() == "BOOLEAN_EQUAL")      || (cstStack.top()->type() == "NOT_EQUAL")             ||
                                                                    (cstStack.top()->type() == "LT")          || (cstStack.top()->type() == "GT")          ||
                                                                    (cstStack.top()->type() == "LT_EQUAL") || (cstStack.top()->type() == "GT_EQUAL") ||
                                                                    (cstStack.top()->type() == "BOOLEAN_AND")        || (cstStack.top()->type() == "BOOLEAN_OR")            ||
                                                                    (cstStack.top()->type() == "BOOLEAN_NOT"))
                                                                {
                                                                    auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                                    addASTLeaf(astEntry,true);
                                                                    cstStack.pop();
                                                                }
                                                                else
                                                                {
                                                                    cstStack.push(currentTemp);
                                                                    finished = true;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                cstStack.push(currentTemp);
                                                                finished = true;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        currentTemp = currentTemp->rightSibling();
    }
    while (!cstStack.empty())
    {
        if (cstStack.top()->type() != "L_PAREN" && cstStack.top()->type() != "R_PAREN")
        {
            if(cstStack.top()->type() == "IDENTIFIER")
                foundST = determineSTNode(currentTemp);
            else
                foundST = nullptr;
            auto *astEntry = new ASTNode(cstStack.top(), foundST, cstStack.top()->value(), currentScope);
            addASTLeaf(astEntry,true);
        }
        cstStack.pop();
    }
    return currentTemp;
}

CSTNode* RDParser::createIntExprPostfix(CSTNode* current)
{
    CSTNode* currentTemp = current;
    STNode* foundST;
    std::stack<CSTNode*> cstStack;
    bool finished;
    while(currentTemp->type() != "SEMICOLON" && currentTemp->rightSibling())
    {
        if ((currentTemp->type() == "INTEGER") || (currentTemp->type() == "IDENTIFIER") || (currentTemp->type() == "SINGLE_QUOTE") || (currentTemp->type() == "DOUBLE_QUOTE") || (currentTemp->type() == "STRING"))
        {
            if(currentTemp->type() == "IDENTIFIER")
                foundST = determineSTNode(currentTemp);
            else
                foundST = nullptr;
            auto *astEntry = new ASTNode(currentTemp, foundST, currentTemp->value(), currentScope);
            addASTLeaf(astEntry,true);
        }
        else
        {
            if (currentTemp->type() == "L_PAREN")
            {
                cstStack.push(currentTemp);
            }
            else
            {
                if (currentTemp->type() == "R_PAREN")
                {
                    while (cstStack.top()->type() != "L_PAREN")
                    {
                        auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                        addASTLeaf(astEntry,true);
                        cstStack.pop();
                    }
                    cstStack.pop();
                }
                else
                {
                    if ((currentTemp->type() == "PLUS") || (currentTemp->type() == "MINUS") || (currentTemp->type() == "ASTERISK") || (currentTemp->type() == "DIVIDE") || (currentTemp->type() == "MODULO") || (currentTemp->type() == "ASSIGNMENT_OPERATOR"))
                    {
                        if (cstStack.empty())
                        {
                            cstStack.push(currentTemp);
                        }
                        else
                        {
                            if ((currentTemp->type() == "PLUS") || (currentTemp->type() == "MINUS"))
                            {
                                finished = false;
                                while (!finished)
                                {
                                    if (!cstStack.empty())
                                    {
                                        if ((cstStack.top()->type() == "PLUS") || (cstStack.top()->type() == "MINUS") || (cstStack.top()->type() == "ASTERISK") || (cstStack.top()->type() == "DIVIDE") || (cstStack.top()->type() == "MODULO"))
                                        {
                                            auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                            addASTLeaf(astEntry,true);
                                            cstStack.pop();
                                        }
                                        else
                                        {
                                            cstStack.push(currentTemp);
                                            finished = true;
                                        }
                                    }
                                    else
                                    {
                                        cstStack.push(currentTemp);
                                        finished = true;
                                    }
                                }
                            }
                            else
                            {
                                if ((currentTemp->type() == "ASTERISK") || (currentTemp->type() == "DIVIDE") || (currentTemp->type() == "MODULO"))
                                {
                                    finished = false;
                                    while (!finished)
                                    {
                                        if (!cstStack.empty())
                                        {
                                            if ((cstStack.top()->type() == "ASTERISK") || (cstStack.top()->type() == "DIVIDE") || (cstStack.top()->type() == "MODULO"))
                                            {
                                                auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                addASTLeaf(astEntry,true);
                                                cstStack.pop();
                                            }
                                            else
                                            {
                                                cstStack.push(currentTemp);
                                                finished = true;
                                            }
                                        }
                                        else
                                        {
                                            cstStack.push(currentTemp);
                                            finished = true;
                                        }
                                    }
                                }
                                else
                                {
                                    if (currentTemp->type() == "ASSIGNMENT_OPERATOR")
                                    {
                                        finished = false;
                                        while (!finished)
                                        {
                                            if (!cstStack.empty())
                                            {
                                                if ((currentTemp->type() == "PLUS") || (currentTemp->type() == "MINUS") || (currentTemp->type() == "ASTERISK") || (currentTemp->type() == "DIVIDE") || (currentTemp->type() == "MODULO"))
                                                {
                                                    auto *astEntry = new ASTNode(cstStack.top(), nullptr, cstStack.top()->value(), currentScope);
                                                    addASTLeaf(astEntry,true);
                                                    cstStack.pop();
                                                }
                                                else
                                                {
                                                    cstStack.push(currentTemp);
                                                    finished = true;
                                                }
                                            }
                                            else
                                            {
                                                cstStack.push(currentTemp);
                                                finished = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        currentTemp = currentTemp->rightSibling();
    }
    while (!cstStack.empty())
    {
        if(cstStack.top()->type() == "IDENTIFIER")
            foundST = determineSTNode(currentTemp);
        else
            foundST = nullptr;
        auto *astEntry = new ASTNode(cstStack.top(), foundST, cstStack.top()->value(), currentScope);
        addASTLeaf(astEntry,true);
        cstStack.pop();
    }
    return currentTemp;
}

STNode* RDParser::determineSTNode(CSTNode* current)
{
    for(auto cur = rootST; cur; cur = cur->next() )
    {
        if((cur->identifierName() == current->value() && cur->scope() == currentScope)||(cur->identifierName() == current->value() && cur->scope() == 0)||(cur->identifierName() == current->value() && (cur->identifierType() == "function"||cur->identifierType() == "procedure")))
            return cur;
    }
    return nullptr;
}

void RDParser::breadthFirstASTPrint() {
    if (!rootAST) return;
    std::queue<ASTNode*> queue;
    queue.push(rootAST);
    int nullCount = 0;
    int spaceCount = 0;
    int changeWidthSpaceCount = 0;
    int changeWidthNullCount = 0;
    int levelCount = 1;
    int columnWidth = 25;
    int NewColumnWidth = 25;
    while (!queue.empty()) {
        ASTNode* current = queue.front();
        queue.pop();
        if(current->label().size() > columnWidth)
        {
            NewColumnWidth = static_cast<int>(current->label().size());
            changeWidthSpaceCount = spaceCount;
            changeWidthNullCount = nullCount;
            std::cout << std::setw(NewColumnWidth) << current->label();
        }
        else{
            std::cout  << std::setw(NewColumnWidth) << current->label();
        }
        if (current->leftChild()) {
            std::cout << std::setw(NewColumnWidth) << "null" << std::endl;
            levelCount++;
            if(spaceCount > 0)
            {
                for(int i=0; i<spaceCount; i++)
                {
                    if(i<changeWidthSpaceCount)
                    {
                        std::cout << std::setw(columnWidth) << " ";
                    }else
                    {
                        std::cout << std::setw(NewColumnWidth) << " ";
                    }
                }
            }

            for(int i=0; i<nullCount; i++)
            {
                if(i<changeWidthNullCount)
                {
                    std::cout << std::setw(columnWidth) << "null";
                }else
                {
                    std::cout << std::setw(NewColumnWidth) << "null";
                }
            }
            queue.push(current->leftChild());
            spaceCount += nullCount;
            nullCount=0;
        }
        if (current->rightSibling())
        {
            queue.push(current->rightSibling());
            nullCount++;
        }
    }
    std::cout << std::setw(columnWidth) << "null" << std::endl;
    for(int i=0; i<spaceCount; i++)
        std::cout << std::setw(columnWidth) << " ";
    std::cout << std::setw(columnWidth) << "null" << std::endl;
}

void RDParser::breadthFirstCSTPrint() {
    if (!rootCST) return;
    std::queue<CSTNode*> queue;
    queue.push(rootCST);
    int nullCount = 0;
    int spaceCount = 0;
    int changeWidthSpaceCount = 0;
    int changeWidthNullCount = 0;
    int levelCount = 1;
    int columnWidth = 25;
    int NewColumnWidth = 25;
    while (!queue.empty()) {
        CSTNode* current = queue.front();
        queue.pop();
        if(current->value().size() > columnWidth)
        {
            NewColumnWidth = static_cast<int>(current->value().size());
            changeWidthSpaceCount = spaceCount;
            changeWidthNullCount = nullCount;
            std::cout << std::setw(NewColumnWidth) << current->value();
        }
        else{
            std::cout  << std::setw(NewColumnWidth) << current->value();
        }
        if (current->leftChild()) {
            std::cout << std::setw(NewColumnWidth) << "null" << std::endl;
            levelCount++;
            if(spaceCount > 0)
            {
                for(int i=0; i<spaceCount; i++)
                {
                    if(i<changeWidthSpaceCount)
                    {
                        std::cout << std::setw(columnWidth) << " ";
                    }else
                    {
                        std::cout << std::setw(NewColumnWidth) << " ";
                    }
                }
            }

            for(int i=0; i<nullCount; i++)
            {
                if(i<changeWidthNullCount)
                {
                    std::cout << std::setw(columnWidth) << "null";
                }else
                {
                    std::cout << std::setw(NewColumnWidth) << "null";
                }
            }
            queue.push(current->leftChild());
            spaceCount += nullCount;
            nullCount=0;
        }
        if (current->rightSibling())
        {
            queue.push(current->rightSibling());
            nullCount++;
        }
    }
    std::cout << std::setw(columnWidth) << "null" << std::endl;
    for(int i=0; i<spaceCount; i++)
        std::cout << std::setw(columnWidth) << " ";
    std::cout << std::setw(columnWidth) << "null" << std::endl;
}

void RDParser::breadthFirstASTFilePrint(std::string inputFileName) {
    std::string str2 = inputFileName.substr (6,inputFileName.size());
    char *addStart = (char*)"Output/output-";
    const char *copy = str2.c_str();
    char newCopy[strlen(copy)];
    std::strcpy(newCopy, copy);
    newCopy[strlen(newCopy)-2] = '\0';
    char *addEnding = (char*)".txt";
    const unsigned int newWordSize = 120;
    char newFileName[newWordSize];
    std::strcpy(newFileName, addStart);
    std::strcat(newFileName, newCopy);
    std::strcat(newFileName, addEnding);
    std::ofstream resultsDataFile;
    resultsDataFile.open (newFileName);

    if (!rootAST) return;
    std::queue<ASTNode*> queue;
    queue.push(rootAST);
    int nullCount = 0;
    int spaceCount = 0;
    int valueSize = 0;
    int changeWidthSpaceCount = 0;
    int changeWidthNullCount = 0;
    int levelCount = 1;
    int columnWidth = 26;
    int NewColumnWidth = 26;
    while (!queue.empty()) {
        ASTNode* current = queue.front();
        queue.pop();
        valueSize = static_cast<int>(current->label().size());
        if(valueSize > NewColumnWidth)
        {
            changeWidthSpaceCount = spaceCount;
            changeWidthNullCount = nullCount;
            NewColumnWidth = valueSize+1;
            resultsDataFile  << std::setw(NewColumnWidth) << current->label();
        }
        else{
            resultsDataFile  << std::setw(NewColumnWidth) << current->label();
        }
        if (current->leftChild()) {
            if(columnWidth < NewColumnWidth)
            {
                resultsDataFile << std::setw(NewColumnWidth) << "null" << std::endl;
            }
            else
            {
                resultsDataFile << std::setw(columnWidth) << "null" << std::endl;
            }
            levelCount++;
            if(spaceCount > 0)
            {
                for(int i=0; i<changeWidthSpaceCount; i++)
                    resultsDataFile << std::setw(columnWidth) << " ";
                for(int j=changeWidthSpaceCount; j<spaceCount; j++)
                    resultsDataFile << std::setw(NewColumnWidth) << " ";

            }

            for(int i=0; i<changeWidthNullCount; i++)
                resultsDataFile << std::setw(columnWidth) << "null";
            for(int i=changeWidthNullCount; i<nullCount; i++)
                resultsDataFile << std::setw(NewColumnWidth) << "null";
            queue.push(current->leftChild());
            spaceCount += nullCount;
            nullCount=0;
        }
        if (current->rightSibling())
        {
            queue.push(current->rightSibling());
            nullCount++;
        }
    }
    resultsDataFile << std::setw(NewColumnWidth) << "null" << std::endl;
    for(int i=0; i<changeWidthSpaceCount-3; i++)
        resultsDataFile << std::setw(columnWidth) << " ";
    for(int j=changeWidthSpaceCount; j<spaceCount; j++)
        resultsDataFile << std::setw(NewColumnWidth) << " ";
    resultsDataFile << std::setw(NewColumnWidth) << "null" << std::endl;
    resultsDataFile.close();
}

void RDParser::breadthFirstCSTFilePrint(std::string inputFileName) {
    std::string str2 = inputFileName.substr (6,inputFileName.size());
    char *addStart = (char*)"Output/output-";
    const char *copy = str2.c_str();
    char newCopy[strlen(copy)];
    std::strcpy(newCopy, copy);
    newCopy[strlen(newCopy)-2] = '\0';
    char *addEnding = (char*)".txt";
    const unsigned int newWordSize = 120;
    char newFileName[newWordSize];
    std::strcpy(newFileName, addStart);
    std::strcat(newFileName, newCopy);
    std::strcat(newFileName, addEnding);
    std::ofstream resultsDataFile;
    resultsDataFile.open (newFileName);

    if (!rootCST) return;
    std::queue<CSTNode*> queue;
    queue.push(rootCST);
    int nullCount = 0;
    int spaceCount = 0;
    int valueSize = 0;
    int changeWidthSpaceCount = 0;
    int changeWidthNullCount = 0;
    int levelCount = 1;
    int columnWidth = 25;
    int NewColumnWidth = 25;
    while (!queue.empty()) {
        CSTNode* current = queue.front();
        queue.pop();
        valueSize = static_cast<int>(current->value().size());
        if(valueSize > NewColumnWidth)
        {
            changeWidthSpaceCount = spaceCount;
            changeWidthNullCount = nullCount;
            NewColumnWidth = valueSize+1;
            resultsDataFile  << std::setw(NewColumnWidth) << current->value();
        }
        else{
            resultsDataFile  << std::setw(NewColumnWidth) << current->value();
        }
        if (current->leftChild()) {
            if(columnWidth < NewColumnWidth)
            {
                resultsDataFile << std::setw(NewColumnWidth) << "null" << std::endl;
            }
            else
            {
                resultsDataFile << std::setw(columnWidth) << "null" << std::endl;
            }
            levelCount++;
            if(spaceCount > 0)
            {
                for(int i=0; i<changeWidthSpaceCount; i++)
                    resultsDataFile << std::setw(columnWidth) << " ";
                for(int j=changeWidthSpaceCount; j<spaceCount; j++)
                    resultsDataFile << std::setw(NewColumnWidth) << " ";

            }

            for(int i=0; i<changeWidthNullCount; i++)
                resultsDataFile << std::setw(columnWidth) << "null";
            for(int i=changeWidthNullCount; i<nullCount; i++)
                resultsDataFile << std::setw(NewColumnWidth) << "null";
            queue.push(current->leftChild());
            spaceCount += nullCount;
            nullCount=0;
        }
        if (current->rightSibling())
        {
            queue.push(current->rightSibling());
            nullCount++;
        }
    }
    resultsDataFile << std::setw(NewColumnWidth) << "null" << std::endl;
    for(int i=0; i<changeWidthSpaceCount-3; i++)
        resultsDataFile << std::setw(columnWidth) << " ";
    for(int j=changeWidthSpaceCount; j<spaceCount; j++)
        resultsDataFile << std::setw(NewColumnWidth) << " ";
    resultsDataFile << std::setw(NewColumnWidth) << "null" << std::endl;
    resultsDataFile.close();
}

