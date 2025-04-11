//
// Created by Nwseb on 3/31/2025.
//

#include <queue>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "SymbolTable.hpp"
#include "Constants.hpp"

SymbolTable::SymbolTable(CSTNode *rootCST) : _rootCST(rootCST),headSymbolTable{nullptr}, previousSymbolTable{nullptr}, tailSymbolTable{nullptr}, parametersHead{nullptr} {}

void SymbolTable::createSymbolTable()
{
    if (!_rootCST) return;
    std::queue<CSTNode*> queue;
    queue.push(_rootCST);
    bool inFunctionOrProcedure = false;
    int curlyBraceCount = 0;
    while (!queue.empty()) {
        CSTNode* current = queue.front();
        queue.pop();
        if(inFunctionOrProcedure)
        {
            if(current->type() == "L_BRACE")
                curlyBraceCount++;
            if(current->type() == "R_BRACE")
                curlyBraceCount--;
        }
        if(curlyBraceCount == 0)
        {
            inFunctionOrProcedure = false;
            currentScope = 0;
        }
        if (current->rightSibling()) {
            queue.push(current->rightSibling());
            if (current->type() == "IDENTIFIER" && checkIsReservedWord(current->value()) && checkIsDataType(current->value()))
            {
                current = addVariable(current);
                queue.pop();
            }
            else if (current->type() == "IDENTIFIER" && checkIsReservedWord(current->value()))
            {
                if (current->value() == "function")
                {
                    inFunctionOrProcedure = true;
                    current = addFunction(current);
                    queue.pop();
                }
                else if (current->value() == "procedure")
                {
                    inFunctionOrProcedure = true;
                    current = addProcedure(current);
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

bool SymbolTable::checkIsReservedWord(const std::string& word)
{
    for(size_t i=0; i < _reservedWords->size(); i++){
        if(word == _reservedWords->at(i))
            return true;
    }
    return false;
}

bool SymbolTable::checkIsDataType(const std::string& word)
{
    for(size_t i=0; i < _dataTypes->size(); i++){
        if(word == _dataTypes->at(i))
            return true;
    }
    return false;
}

bool SymbolTable::checkIsGlobalVar(const std::string& word)
{
    for(size_t i=0; i < _globalVars->size(); i++){
        if(word == _globalVars->at(i))
            return true;
    }
    return false;
}

bool SymbolTable::checkVarInSameScope(const std::string& word)
{
    for(auto cur = headSymbolTable; cur; cur = cur->next() )
    {
        if(word == cur->identifierName() && cur->scope() == currentScope)
            return true;
        if(cur->parameterList())
        {
            for(auto cur1 = cur->parameterList(); cur1; cur1 = cur1->next() )
            {
                if(word == cur1->identifierName() && cur1->scope() == currentScope)
                    return true;
            }
        }
    }
    return false;
}

CSTNode* SymbolTable::addVariable(CSTNode* current)
{
    CSTNode* currentTemp = current;
    CSTNode* previousTemp = current;
    std::string idName;
    std::string idType = "datatype";
    std::string varValue;
    std::string varDataType;
    int varArraySize = 0;
    bool varIsArray = false;

    while(currentTemp->rightSibling()){
        if (currentTemp->type() == "IDENTIFIER" && checkIsDataType(currentTemp->value()))
        {
            varDataType = currentTemp->value();
            currentTemp = currentTemp->rightSibling();
            if(currentScope == 0 && !checkIsGlobalVar(currentTemp->value()))
            {
                globalVars.push_back(currentTemp->value());
            }
            else if((checkIsGlobalVar(currentTemp->value())))
            {
                std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": variable \""<< currentTemp->value() << "\" is already defined globally." << std::endl;
                exit(E_SYNTAX_ERROR);
            }
            else if((checkVarInSameScope(currentTemp->value())))
            {
                std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": variable \""<< currentTemp->value() << "\" is already defined locally." << std::endl;
                exit(E_SYNTAX_ERROR);
            }
            idName = currentTemp->value();
            if (currentTemp->rightSibling()->value() != "[")
            {
                auto *tableEntry = new STNode(idName, idType, nullptr, varDataType, varIsArray, varArraySize, varValue, currentScope);
                addEntry(tableEntry);
            }
        }
        else if (previousTemp->value() == ",")
        {
            idName = currentTemp->value();
            auto *tableEntry = new STNode(idName, idType, nullptr, varDataType, varIsArray, varArraySize, varValue, currentScope);
            addEntry(tableEntry);
        }
        else if (currentTemp->value() == "[")
        {
            currentTemp = currentTemp->rightSibling();
            if (currentTemp->type() == "INTEGER")
            {
                varArraySize = std::stoi(currentTemp->value());
                varIsArray = true;
                auto *tableEntry = new STNode(idName, idType, nullptr, varDataType, varIsArray, varArraySize, varValue, currentScope);
                addEntry(tableEntry);
            }
            else
            {
                std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": array declaration size must be an integer." << std::endl;
                exit(E_SYNTAX_ERROR);
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

CSTNode* SymbolTable::addParameterList(CSTNode* current, STNode *_parameterList)
{
    CSTNode* currentTemp = current;
    STNode *tailList = nullptr;
    std::string idName;
    std::string idType = "datatype";
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
        }
        if (currentTemp->rightSibling()->value() == ","||currentTemp->rightSibling()->value() == ")")
        {
            auto *tableEntry = new STNode(idName, idType, nullptr, varDataType, varIsArray, varArraySize, varValue, currentScope);
            if(_parameterList == nullptr){
                _parameterList = tailList = tableEntry;
            }
            else
            {
                tailList->next(tableEntry);
                tailList = tableEntry;
            }
        }
        else if (currentTemp->value() == "[")
        {
            currentTemp = currentTemp->rightSibling();
            if (currentTemp->type() == "INTEGER")
            {
                varArraySize = std::stoi(currentTemp->value());
                varIsArray = true;
                auto *tableEntry = new STNode(idName, idType, nullptr, varDataType, varIsArray, varArraySize, varValue, currentScope);
                if(_parameterList == nullptr){
                    _parameterList = tailList = tableEntry;
                }
                else
                {
                    tailList->next(tableEntry);
                    tailList = tableEntry;
                }
                currentTemp = currentTemp->rightSibling();
            }
            else
            {
                std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": array declaration size must be an integer." << std::endl;
                exit(E_SYNTAX_ERROR);
            }
        }
        currentTemp = currentTemp->rightSibling();
    }
    parametersHead = _parameterList;
    return currentTemp;
}

CSTNode* SymbolTable::addFunction(CSTNode* current)
{
    parametersHead = nullptr;
    scopeCount++;
    currentScope = scopeCount;
    CSTNode* currentTemp = current;
    std::string idName;
    std::string idType = "function";
    currentTemp = currentTemp->rightSibling();
    std::string varDataType = currentTemp->value();
    std::string varValue;
    STNode* paramList = nullptr;
    int varArraySize = 0;
    bool varIsArray = false;
    bool hasparamList = true;
    currentTemp = currentTemp->rightSibling();
    idName = currentTemp->value();
    currentTemp = currentTemp->rightSibling();

    while(currentTemp->rightSibling())
    {
        if (currentTemp->value() == "(")
        {
            currentTemp = currentTemp->rightSibling();
        }
        else
        {
            std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": array declaration size must be a positive integer." << std::endl;
            exit(E_SYNTAX_ERROR);
        }
        if (currentTemp->value() == ")"||currentTemp->value() == "void")
        {
            hasparamList = false;
            currentTemp = currentTemp->rightSibling();
        }
        else if(hasparamList)
        {
            currentTemp = addParameterList(currentTemp, paramList);
        }
    }
    paramList = parametersHead;
    auto *tableEntry = new STNode(idName, idType, paramList, varDataType, varIsArray, varArraySize, varValue, currentScope);
    addEntry(tableEntry);
    return currentTemp;
}

CSTNode* SymbolTable::addProcedure(CSTNode* current)
{
    parametersHead = nullptr;
    scopeCount++;
    currentScope = scopeCount;
    CSTNode* currentTemp = current;
    std::string idName;
    std::string idType = "procedure";
    std::string varDataType = "N/A";
    std::string varValue;
    STNode* paramList = nullptr;
    int varArraySize = 0;
    bool varIsArray = false;
    bool hasparamList = true;
    currentTemp = currentTemp->rightSibling();
    idName = currentTemp->value();
    currentTemp = currentTemp->rightSibling();

    while(currentTemp->rightSibling())
    {
        if (currentTemp->value() == "(")
        {
            currentTemp = currentTemp->rightSibling();
        }
        else
        {
            std::cerr << errorMessages[E_SYNTAX_ERROR] << currentTemp->lineNumber() << ": array declaration size must be a positive integer." << std::endl;
            exit(E_SYNTAX_ERROR);
        }
        if (currentTemp->value() == ")"||currentTemp->value() == "void")
        {
            hasparamList = false;
            currentTemp = currentTemp->rightSibling();
        }
        else if(hasparamList)
        {
            currentTemp = addParameterList(currentTemp, paramList);
        }
    }
    paramList = parametersHead;
    auto *tableEntry = new STNode(idName, idType, paramList, varDataType, varIsArray, varArraySize, varValue, currentScope);
    addEntry(tableEntry);
    return currentTemp;
}

void SymbolTable::addEntry(STNode* newEntry) {
    if(headSymbolTable == nullptr){
        headSymbolTable = tailSymbolTable = newEntry;
    }
    else
    {
        tailSymbolTable->next(newEntry);
        tailSymbolTable = newEntry;
    }
}

void SymbolTable::print() {
    std::string arrayYesNo;
    for(auto cur = headSymbolTable; cur; cur = cur->next() )
    {
        if(cur->variableIsArray())
        {
            arrayYesNo = "yes";
        }else{
            arrayYesNo = "no";
        }
        std::cout << std::setw(22) << "IDENTIFIER_NAME: " << cur->identifierName() << std::endl;
        std::cout << std::setw(22) << "IDENTIFIER_TYPE: " << cur->identifierType() << std::endl;
        std::cout << std::setw(22) << "DATATYPE: " << cur->variableDataType() << std::endl;
        std::cout << std::setw(22) << "DATATYPE_IS_ARRAY: " << arrayYesNo << std::endl;
        std::cout << std::setw(22) << "DATATYPE_ARRAY_SIZE: " << cur->variableArraySize() << std::endl;
        std::cout << std::setw(22) << "SCOPE: " << cur->scope() << std::endl;
        std::cout << std::endl;
    }
    for(auto cur = headSymbolTable; cur; cur = cur->next() )
    {
        if(cur->parameterList())
        {
            std::cout << std::setw(22) << "PARAMETER LIST FOR: " << cur->identifierName() << std::endl;
            for(auto cur1 = cur->parameterList(); cur1; cur1 = cur1->next() )
            {
                if(cur1->variableIsArray())
                {
                    arrayYesNo = "yes";
                }else{
                    arrayYesNo = "no";
                }
                std::cout << std::setw(22) << "IDENTIFIER_NAME: " << cur1->identifierName() << std::endl;
                std::cout << std::setw(22) << "DATATYPE: " << cur1->variableDataType() << std::endl;
                std::cout << std::setw(22) << "DATATYPE_IS_ARRAY: " << arrayYesNo << std::endl;
                std::cout << std::setw(22) << "DATATYPE_ARRAY_SIZE: " << cur1->variableArraySize() << std::endl;
                std::cout << std::setw(22) << "SCOPE: " << cur1->scope() << std::endl;
                std::cout << std::endl;
            }
        }
    }
}

void SymbolTable::outputToFile(std::string inputFileName) {
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

    std::string arrayYesNo;
    for(auto cur = headSymbolTable; cur; cur = cur->next() )
    {
        if(cur->variableIsArray())
        {
            arrayYesNo = "yes";
        }else{
            arrayYesNo = "no";
        }
        resultsDataFile << std::setw(22) << "IDENTIFIER_NAME: " << cur->identifierName() << std::endl;
        resultsDataFile << std::setw(22) << "IDENTIFIER_TYPE: " << cur->identifierType() << std::endl;
        resultsDataFile << std::setw(22) << "DATATYPE: " << cur->variableDataType() << std::endl;
        resultsDataFile << std::setw(22) << "DATATYPE_IS_ARRAY: " << arrayYesNo << std::endl;
        resultsDataFile << std::setw(22) << "DATATYPE_ARRAY_SIZE: " << cur->variableArraySize() << std::endl;
        resultsDataFile << std::setw(22) << "SCOPE: " << cur->scope() << std::endl;
        resultsDataFile << std::endl;
    }
    for(auto cur = headSymbolTable; cur; cur = cur->next() )
    {
        if(cur->parameterList())
        {
            resultsDataFile << std::endl;
            resultsDataFile << std::setw(22) << "PARAMETER LIST FOR: " << cur->identifierName() << std::endl;
            for(auto cur1 = cur->parameterList(); cur1; cur1 = cur1->next() )
            {
                if(cur1->variableIsArray())
                {
                    arrayYesNo = "yes";
                }else{
                    arrayYesNo = "no";
                }
                resultsDataFile << std::setw(22) << "IDENTIFIER_NAME: " << cur1->identifierName() << std::endl;
                resultsDataFile << std::setw(22) << "DATATYPE: " << cur1->variableDataType() << std::endl;
                resultsDataFile << std::setw(22) << "DATATYPE_IS_ARRAY: " << arrayYesNo << std::endl;
                resultsDataFile << std::setw(22) << "DATATYPE_ARRAY_SIZE: " << cur1->variableArraySize() << std::endl;
                resultsDataFile << std::setw(22) << "SCOPE: " << cur1->scope() << std::endl;
                resultsDataFile << std::endl;
            }
        }
    }
    resultsDataFile.close();

}