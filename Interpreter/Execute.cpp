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
    _currentAST = getMain();
    if (_currentAST)
    {
        _currentST = _currentAST->stNode();
        std::cout << _currentAST->label() << "  " << _currentAST->value() << std::endl;
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
        executionStack.pop();
    }
    if(_currentAST->label() == "ASSIGNMENT")
    {
        executeAssignment();
    }
    if(_currentAST->label() == "CALL" && (_currentAST->stNode()->identifierType() == "procedure" || _currentAST->stNode()->identifierType() == "function"))
    {
        _currentAST = executeFunctionOrProcedure(_currentAST);
    }
    if(_currentAST->label() == "FOR EXPRESSION 1")
    {
        _currentAST = beginForLoop();
    }
    if(_currentAST->label() == "IF")
    {
        _currentAST = _currentAST->rightSibling();
        ifConditionMet = evaluatePostfixBool(_currentAST);
        if(ifConditionMet)
        {

            executionStack.push(_currentAST);
        }
        else
        {
            skipBlock();
        }
    }
    if(_currentAST->label() == "ELSE")
    {
        _currentAST = _currentAST->leftChild();
        if(ifConditionMet)
        {
            skipBlock();
        }
        executionStack.push(_currentAST);
    }
    if(_currentAST->label() == "PRINTF")
    {
        printAndF();
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

ASTNode *Execute::getMain()
{
    if (!_rootAST) return nullptr;
    std::queue<ASTNode*> queue;
    queue.push(_rootAST);
    while (!queue.empty()) {
        ASTNode* current = queue.front();
        queue.pop();
        if(current->label() == "DECLARATION" && current->value() == "main")
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
    int curlyBraces = 0;
    std::string iString = std::to_string(i);
    STNode* stNodeI = _currentAST->stNode();
    //Initialize "FOR EXPRESSION 1"
    stNodeI->setVariableValue(&iString);
    //Move to "FOR EXPRESSION 2"
    while(_currentAST->rightSibling())
    {
        _currentAST = _currentAST->rightSibling();
    }
    _currentAST = _currentAST->leftChild();
    executionStack.push(_currentAST);
    return _currentAST;
}

ASTNode *Execute::executeFunctionOrProcedure(ASTNode* current)
{
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
                tempValue = currentTemp->stNode()->variableValue();
                currentST->setVariableValue(&tempValue);
                currentTemp = currentTemp->rightSibling();
            }
        }
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
        if(currentTemp->type() != "IDENTIFIER" || (currentTemp->type() == "IDENTIFIER" && currentTemp->stNode()->identifierType() != "function" && currentTemp->stNode()->identifierType() != "procedure"))
        {
            tempValue = std::to_string(evaluatePostfixInt(currentTemp));
            _currentAST->stNode()->setVariableValue(&tempValue);
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

            // Otherwise, it must be an operator
        else
        {
            int val1 = intStack.top();
            intStack.pop();
            int val2 = intStack.top();
            intStack.pop();

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

            // Otherwise, it must be an operator
        else
        {
            int val1 = intStack.top();
            intStack.pop();
            int val2 = intStack.top();
            intStack.pop();
            bool bool1;
            int bool2;
            if(!boolStack.empty())
            {
                bool1 = boolStack.top();
                boolStack.pop();
                bool2 = boolStack.top();
                boolStack.pop();
            }

            if (val == ">")
            {
                boolStack.push(val2 > val1);
            } else if (val == "<")
            {
                boolStack.push(val2 < val1);
            } else if (val == "<=")
            {
                boolStack.push(val2 <= val1);
            } else if (val == ">=")
            {
                boolStack.push(val2 < val1);
            } else if (val == "&&")
            {
                boolStack.push(bool2 && bool1);
            } else if (val == "||")
            {
                boolStack.push(bool2 || bool1);
            }
        }
        currentTemp = currentTemp->rightSibling();
    }
    return boolStack.top();
}
