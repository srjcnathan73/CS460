//
// Created by Nwseb on 2/24/2025.
//
#include <iostream>
#include <queue>
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
    SymbolTable symbolTable(rootCST);
    symbolTable.createSymbolTable();
    breadthFirstCSTPrint();
    breadthFirstCSTFilePrint(_fileName);
    //SymbolTable.print();
    //SymbolTable.outputToFile(_fileName);
}

void RDParser::createAST()
{
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
        } else
        {
            currentAST->leftChild(newEntry);
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
            if (currentTemp->rightSibling()->value() != "[")
            {
                auto *astEntry = new ASTNode(currentTemp, varValue, currentScope);
                addASTLeaf(astEntry,true);
            }
        }
        else if (previousTemp->value() == ",")
        {
            idName = currentTemp->value();
            auto *astEntry = new ASTNode(currentTemp, varValue, currentScope);
            addASTLeaf(astEntry,true);
        }
        else if (currentTemp->value() == "[")
        {
            currentTemp = currentTemp->rightSibling();
            if (currentTemp->type() == "INTEGER")
            {
                varArraySize = std::stoi(currentTemp->value());
                varIsArray = true;
                auto *astEntry = new ASTNode(currentTemp, varValue, currentScope);
                addASTLeaf(astEntry,true);
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

CSTNode* RDParser::addFunctionDeclaration(CSTNode* current)
{
    scopeCount++;
    currentScope = scopeCount;
    CSTNode* currentTemp = current;
    std::string idName;
    std::string idType = "function";
    currentTemp = currentTemp->rightSibling();
    std::string varDataType = currentTemp->value();
    std::string varValue;
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
            currentTemp = currentTemp->rightSibling();
        }
    }
    auto *astEntry = new ASTNode(currentTemp, varValue, currentScope);
    addASTLeaf(astEntry,true);
    return currentTemp;
}

CSTNode* RDParser::addProcedureDeclaration(CSTNode* current)
{
    scopeCount++;
    currentScope = scopeCount;
    CSTNode* currentTemp = current;
    std::string idName;
    std::string idType = "procedure";
    std::string varDataType = "N/A";
    std::string varValue;
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
            currentTemp = currentTemp->rightSibling();
        }
    }
    auto *astEntry = new ASTNode(currentTemp, varValue, currentScope);
    addASTLeaf(astEntry,true);
    return currentTemp;
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
            NewColumnWidth = valueSize;
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

