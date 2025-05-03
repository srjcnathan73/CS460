//
// Created by Nwseb on 4/24/2025.
////
#include <iostream>
#include <queue>
#include "Execute.hpp"

Execute::Execute(ASTNode *rootAST, STNode *headST, std::string *fileName)
{
    _fileName = *fileName;
    _rootAST = rootAST;
    _headST = headST;
    std::string mainName = "main";
    _currentAST = getFunctionOrProcedure(mainName);
    if (_currentAST)
    {
        _currentST = _currentAST->stNode();
        std::cout << "  "  << std::endl;
        executionStack.push(_currentAST);
        _currentAST = _currentAST->leftChild();
        while(!executionStack.empty())
        {
            executeBlock();
        }
    }
}

void Execute::executeBlock()
{
    if(_currentAST->label() == "END BLOCK")
    {
        if((_currentAST->leftChild() != nullptr && _currentAST->scope() != _currentAST->leftChild()->scope())||((insideLoop && !insideIf && ifCount <= 0)&&((executionStack.top()->label() == "FOR EXPRESSION 2")||(executionStack.top()->label() == "ASSIGNMENT")||(executionStack.top()->label() == "WHILE"))))
        {
            if(ifCount < 0)
                ifCount = 0;
            _currentAST = executionStack.top();
        }
        else
        {
            if(insideLoop && insideIf)
                insideIf = false;
            if(ifCount > 0)
                ifCount--;
            executionStack.pop();
        }
    }
    if(_currentAST->label() == "ASSIGNMENT")
    {
        executeAssignment();
    }
    if(_currentAST->label() == "CALL" && (_currentAST->stNode()->identifierType() == "procedure" || _currentAST->stNode()->identifierType() == "function"))
    {
        _currentAST = executeFunctionOrProcedure(_currentAST);
        _currentAST = getFunctionOrProcedure(currentFunctionOrProcedure);
    }
    if(_currentAST->label() == "FOR EXPRESSION 1")
    {
        _currentAST = beginForLoop();
    }
    if(_currentAST->label() == "FOR EXPRESSION 2")
    {
        executeLoopStatement();
    }
    if(_currentAST->label() == "WHILE")
    {
        executeLoopStatement();
    }
    if(_currentAST->label() == "RETURN")
    {
        currentReturnVal = _currentAST->rightSibling()->stNode()->variableValue();
    }
    if(_currentAST->label() == "IF")
    {
        executeIfStatement();
    }
    if(_currentAST->label() == "ELSE")
    {
        _currentAST = _currentAST->leftChild();
        if(ifConditionMet)
        {
            skipBlock();
        }
        else
        {
            executionStack.push(_currentAST);
            insideIf = true;
            ifCount++;
        }
    }
    if(_currentAST->label() == "PRINTF")
    {
        printAndFnew();
    }
    if (_currentAST->leftChild()) {
        _currentAST = _currentAST->leftChild();
    }else if (_currentAST->rightSibling())
    {
        _currentAST = _currentAST->rightSibling();
    }
}

//Expects _currentAST->label() == "BEGIN BLOCK"
void Execute::skipBlock()
{
    int curlyBraces = 0;
    curlyBraces++;
    _currentAST = _currentAST->leftChild();
    std::queue<ASTNode*> queue;
    queue.push(_currentAST);
    while (!queue.empty() && curlyBraces != 0) {
        ASTNode* current = queue.front();
        _currentAST = current;
        queue.pop();
        if(current->label() == "BEGIN BLOCK")
        {
            curlyBraces++;
        }
        if(current->label() == "END BLOCK")
        {
            curlyBraces--;
        }
        if (current->leftChild()) {
            queue.push(current->leftChild());
        }
        if (current->rightSibling())
        {
            queue.push(current->rightSibling());
        }
    }
}

ASTNode *Execute::getFunctionOrProcedure(const std::string& funcName)
{
    if (!_rootAST) return nullptr;
    std::queue<ASTNode*> queue;
    queue.push(_rootAST);
    while (!queue.empty()) {
        ASTNode* current = queue.front();
        queue.pop();
        if(current->label() == "DECLARATION" && current->value() == funcName)
        {
            return current;
        }
        if (current->leftChild()) {
            queue.push(current->leftChild());
        }
        if (current->rightSibling())
        {
            queue.push(current->rightSibling());
        }
    }
}

ASTNode *Execute::beginForLoop()
{
    int i = 0;
    std::string iString = std::to_string(i);
    STNode* stNodeI = _currentAST->stNode();
    inForLoop = true;
    //Initialize "FOR EXPRESSION 1"
    stNodeI->setVariableValue(&iString);
    //Move to "FOR EXPRESSION 2"
    while(_currentAST->rightSibling())
    {
        _currentAST = _currentAST->rightSibling();
    }
    _currentAST = _currentAST->leftChild();
    executionStack.push(_currentAST);
    insideLoop = true;
    //Move to "FOR EXPRESSION 3"
    while(_currentAST->rightSibling())
    {
        _currentAST = _currentAST->rightSibling();
    }
    _currentAST = _currentAST->leftChild();
    while(_currentAST->rightSibling())
    {
        _currentAST = _currentAST->rightSibling();
    }
    return _currentAST;
}

ASTNode *Execute::executeFunctionOrProcedure(ASTNode* current)
{
    if(executionStack.top()->label() != _currentAST->label())
    {
        _currentAST = _currentAST->rightSibling();
        executionStack.push(_currentAST);
        currentFunctionOrProcedure = _currentAST->value();
    }
    else if (currentFunctionOrProcedure == _currentAST->value())
    {
        executionStack.pop();
        return _currentAST->rightSibling();
    }

    if(!current->stNode()->parameterList() && current->leftChild())
    {
        return current->leftChild();
    }
    ASTNode* currentTemp = current;
    STNode* currentST = current->stNode()->parameterList();
    std::string tempValue;
    while (currentTemp->rightSibling())
    {
        if(currentTemp->type() == "L_PAREN")
        {
            while (currentTemp->rightSibling() && currentTemp->type() != "R_PAREN")
            {
                if(currentTemp->stNode() && currentTemp->stNode()->variableIsArray())
                {
                    int arrayIndex;
                    tempValue = currentTemp->stNode()->variableValue();
                    while (currentTemp->rightSibling() && currentTemp->type() != "R_BRACKET")
                    {
                        if(currentTemp->stNode() && currentTemp->stNode()->variableDataType() == "int")
                            arrayIndex = stoi(currentTemp->stNode()->variableValue());
                        currentTemp = currentTemp->rightSibling();
                    }
                    tempValue = tempValue[arrayIndex];
                    currentST->setVariableValue(&tempValue);
                }
                else if(currentTemp->stNode())
                {
                    tempValue = currentTemp->stNode()->variableValue();
                    currentST->setVariableValue(&tempValue);
                }
                currentTemp = currentTemp->rightSibling();
            }
        }
        if(currentTemp->rightSibling())
            currentTemp = currentTemp->rightSibling();
    }
    return currentTemp;
}

void Execute::executeAssignment()
{
    ASTNode* currentTemp = _currentAST;
    std::string tempValue;
    currentTemp = currentTemp->rightSibling();
    if(currentTemp->stNode()->variableDataType() == "int")
    {
        currentTemp = currentTemp->rightSibling();
        if(currentTemp->type() != "IDENTIFIER" || (currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->identifierType() != "function" &&
        currentTemp->stNode()->identifierType() != "procedure" && !currentTemp->stNode()->variableIsArray()))
        {
            tempValue = std::to_string(evaluatePostfixInt(currentTemp));
            _currentAST->stNode()->setVariableValue(&tempValue);
        }
        else if(currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->identifierType() != "function" &&
        currentTemp->stNode()->identifierType() != "procedure" && currentTemp->stNode()->variableIsArray())
        {
            ASTNode* arrayTemp = currentTemp;
            currentTemp = currentTemp->rightSibling();
            currentTemp = currentTemp->rightSibling();
            std::string stringTemp = arrayTemp->stNode()->variableValue();
            tempValue = currentTemp->stNode()->variableValue();
            _currentAST->stNode()->setVariableValue(reinterpret_cast<std::string *>(&stringTemp[stoi(tempValue)]));
        }
        else if(currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->identifierType() == "function")
        {
            if(executionStack.top()->label() != _currentAST->label())
            {
                executionStack.push(_currentAST);
                tempValue = currentTemp->label();
                executeFunctionOrProcedure(currentTemp);
                _currentAST = getFunctionOrProcedure(tempValue);
            }
            else if(currentTemp->stNode()->variableValue().size() != 0)
            {
                tempValue = currentTemp->stNode()->variableValue();
                _currentAST->stNode()->setVariableValue(&currentReturnVal);
                _currentAST = currentTemp->rightSibling();
                if(executionStack.top()->label() == "ASSIGNMENT")
                    executionStack.pop();
            }
            else if(!inForLoop)
            {
                _currentAST->stNode()->setVariableValue(&currentReturnVal);
            }
            else
            {
                currentTemp->stNode()->setVariableValue(&currentReturnVal);
            }
        }
    }
    else if(currentTemp->stNode()->variableDataType() == "char")
    {
        _currentAST = currentTemp;
        if(currentTemp->type() != "IDENTIFIER" || (currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->identifierType() != "function" && currentTemp->stNode()->identifierType() != "procedure"))
        {
            currentTemp = currentTemp->rightSibling();
            currentTemp = currentTemp->rightSibling();
            tempValue = currentTemp->value();
            _currentAST->stNode()->setVariableValue(&tempValue);
        }
    }
}

void Execute::executeIfStatement()
{
    _currentAST = _currentAST->rightSibling();
    ifConditionMet = evaluatePostfixBool(_currentAST);
    if(ifConditionMet)
    {
        executionStack.push(_currentAST);
        insideIf = true;
        ifCount++;
    }
    else
    {
        skipBlock();
        insideIf = false;
    }
}

void Execute::executeLoopStatement()
{
    ASTNode* currentTemp = _currentAST;
    ASTNode* forStateTemp = _currentAST;
    currentTemp = currentTemp->rightSibling();
    if(inForLoop && currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->variableDataType() == "int")
    {
        int currentInt = stoi(currentTemp->stNode()->variableValue());
        std::string newInt = std::to_string(currentInt + 1);
        currentTemp->stNode()->setVariableValue(&newInt);
    }
    ifConditionMet = evaluatePostfixBool(currentTemp);
    if(ifConditionMet)
    {
        if(executionStack.top()->label() != forStateTemp->label())
            executionStack.push(forStateTemp);
        insideLoop = true;
    }
    else
    {
        currentTemp = _currentAST;
        while(currentTemp->rightSibling()){
            currentTemp = currentTemp->rightSibling();
        }
        if(currentTemp->type() != "L_BRACE")
            currentTemp = currentTemp->leftChild();
        _currentAST = currentTemp;
        skipBlock();
        insideLoop = false;
        inForLoop = false;
    }
}

void Execute::printAndF()
{
    ASTNode* currentTemp = _currentAST;
    currentTemp = currentTemp->rightSibling();
    std::string tempValue;
    std::string tempString;
    int tempInt;
    while (currentTemp && currentTemp->type() != "R_PAREN")
    {
        if(currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->variableDataType() == "int")
        {
            tempInt = stoi(currentTemp->stNode()->variableValue());
        }
        else if(currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->variableDataType() == "char")
        {
            tempString = currentTemp->stNode()->variableValue();
        }
        else if(currentTemp->type() == "STRING")
        {
            tempValue = currentTemp->value();
        }
        currentTemp = currentTemp->rightSibling();
    }
    size_t n = tempValue.size();
    char arr[n+1];
    for(size_t i=0; i<(n+1);i++)
    {
        if(tempValue[i] == '\\' && tempValue[i+1] == 'n')
        {
            tempValue[i]='\n';
            tempValue[i+1]=' ';
        }
        arr[i]=tempValue[i];
    }
    size_t n2 = tempString.size();
    char arr2[n2+1];
    for(size_t i=0; i<(n2+1);i++)
    {
        if(tempString[i] == '\\' && tempString[i+1] == 'x')
        {
            tempString[i]='\x0';
            tempString[i+1]=' ';
        }
        arr2[i]=tempString[i];
    }
    char *cstr1 = arr;
    char *cstr2 = arr2;
    std::printf(cstr1, cstr2, tempInt);
}

void Execute::printAndFnew()
{
    ASTNode* currentTemp = _currentAST;
    std::vector<ASTNode*> args;
    currentTemp = currentTemp->rightSibling();
    std::string tempValue;
    std::string tempString;
    int tempInt1;
    int tempInt2;
    while (currentTemp && currentTemp->type() != "R_PAREN")
    {
        if(currentTemp->type() == "STRING")
        {
            args.push_back(currentTemp);
        }
        else if(currentTemp->type() == "IDENTIFIER")
        {
            args.push_back(currentTemp);
        }
        currentTemp = currentTemp->rightSibling();
    }
    tempValue = args[0]->value();
    size_t n = tempValue.size();
    char arr[n+1];
    for(size_t i=0; i<(n+1);i++)
    {
        if(tempValue[i] == '\\' && tempValue[i+1] == 'n')
        {
            tempValue[i]='\n';
            tempValue[i+1]=' ';
        }
        arr[i]=tempValue[i];
    }
    char *cstr1 = arr;
    char *cstr2;
    if(args.size() == 3 && args.at(1)->stNode()->variableDataType() == "int" && args.at(2)->stNode()->variableDataType() == "int")
    {
        tempInt1 = stoi(args.at(1)->stNode()->variableValue());
        tempInt2 = stoi(args.at(2)->stNode()->variableValue());
        std::printf(cstr1, tempInt1, tempInt2);
    }
    else if(args.size() == 3 && args.at(1)->stNode()->variableDataType() == "char" && args.at(2)->stNode()->variableDataType() == "int")
    {
        tempString = args.at(1)->stNode()->variableValue();
        size_t n2 = tempString.size();
        char arr2[n2+1];
        for(size_t i=0; i<(n2+1);i++)
        {
            if(tempString[i] == '\\' && tempString[i+1] == 'x')
            {
                tempString[i]='\x0';
                tempString[i+1]=' ';
            }
            arr2[i]=tempString[i];
        }
        cstr2 = arr2;
        tempInt2 = stoi(args.at(2)->stNode()->variableValue());
        std::printf(cstr1, cstr2, tempInt2);
    }
    else if(args.size() == 2 && args.at(1)->stNode()->variableDataType() == "int")
    {
        tempInt1 = stoi(args.at(1)->stNode()->variableValue());
        std::printf(cstr1, tempInt1);
    }
    else if(args.size() == 1)
    {
        std::printf(cstr1);
    }
}

void Execute::assignChar()
{
    ASTNode* currentTemp = _currentAST;
    std::string val;
}

int Execute::evaluatePostfixInt(ASTNode* current)
{
    std::stack<int> intStack;
    ASTNode* currentTemp = current;
    std::string val;
    while (currentTemp->rightSibling())
    {
        val = currentTemp->label();
        if(currentTemp->type() == "IDENTIFIER")
        {
            val = currentTemp->stNode()->variableValue();
        }
        if (std::isdigit(val[0]) || (val.size() > 1 && val[0] == '-'))
        {
            intStack.push(stoi(val));
        }
        else if (val.size() == 1 && val != "'" && val != "+" && val != "-" && val != "*" && val != "/" && val != "%")
        {
            char ch = val[0];
            intStack.push(static_cast<int>(ch));
        }

            // Otherwise, it must be an operator
        else if (val != "'")
        {
            int val1,val2;
            if(intStack.size() > 1)
            {
                val1 = intStack.top();
                intStack.pop();
                val2 = intStack.top();
                intStack.pop();
            }

            if (val == "+")
            {
                intStack.push(val2 + val1);
            } else if (val == "-")
            {
                intStack.push(val2 - val1);
            } else if (val == "*")
            {
                intStack.push(val2 * val1);
            } else if (val == "/")
            {
                intStack.push(val2 / val1);
            }else if (val == "%")
            {
                intStack.push(val2 % val1);
            }
        }
        currentTemp = currentTemp->rightSibling();
    }
    return intStack.top();
}

bool Execute::evaluatePostfixBool(ASTNode* current)
{
    std::stack<int> intStack;
    std::stack<bool> boolStack;
    ASTNode* currentTemp = current;
    ASTNode* previousTemp;
    std::string val;
    while (currentTemp)
    {
        val = currentTemp->label();
        if(currentTemp->type() == "IDENTIFIER")
        {
            val = currentTemp->stNode()->variableValue();
        }
        if (std::isdigit(val[0]) || (val.size() > 1 && val[0] == '-'))
        {
            intStack.push(stoi(val));
        }
        else if (val == "true")
        {
            boolStack.push(true);
        }
        else if (val == "false")
        {
            boolStack.push(false);
        }
        else if (val.size() == 1 && val != "'" && val != "<" && val != ">" && val != "%")
        {
            char ch = val[0];
            intStack.push(static_cast<int>(ch));
        }

            // Otherwise, it must be an operator
        else if (val != "'")
        {
            int val1,val2;
            if(intStack.size() > 1)
            {
                val1 = intStack.top();
                intStack.pop();
                val2 = intStack.top();
                intStack.pop();
            }
            bool bool1;
            int bool2;
            if(boolStack.size() > 1)
            {
                bool1 = boolStack.top();
                boolStack.pop();
                bool2 = boolStack.top();
                boolStack.pop();
            }

            if (val == "%")
            {
                intStack.push(val2 % val1);
            }
            if (val == ">")
            {
                boolStack.push(val2 > val1);
            }
            else if (val == "<")
            {
                boolStack.push(val2 < val1);
            }
            else if (val == "<=")
            {
                boolStack.push(val2 <= val1);
            }
            else if (val == ">=")
            {
                boolStack.push(val2 >= val1);
            }
            else if (val == "==")
            {
                boolStack.push(val2 == val1);
            }
            else if (val == "&&")
            {
                boolStack.push(bool2 && bool1);
            }
            else if (val == "||")
            {
                boolStack.push(bool2 || bool1);
            }
        }
        previousTemp = currentTemp;
        currentTemp = currentTemp->rightSibling();
    }
    if(previousTemp->leftChild())
        _currentAST = previousTemp->leftChild();
    return boolStack.top();
}
