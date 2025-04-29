//
// Created by Nwseb on 3/30/2025.
//

#include "STNode.hpp"
#include <string>
STNode::STNode(std::string identifierName, std::string identifierType, STNode *parameterList, std::string variableDataType, bool variableIsArray, int variableArraySize, std::string variableValue, int scope) : _next(nullptr), _identifierName(identifierName), _identifierType(identifierType), _parameterList(parameterList), _variableDataType(variableDataType), _variableIsArray(variableIsArray), _variableArraySize(variableArraySize), _variableValue(variableValue), _scope(scope){}


STNode *STNode::next() {
    return _next;
}

void STNode::next(STNode *token) {
    _next = token;
}

std::string STNode::identifierName() {
    return _identifierName;
}

std::string STNode::identifierType() {
    return _identifierType;
}

std::string STNode::variableDataType() {
    return _variableDataType;
}

std::string STNode::variableValue() {
    return _variableValue;
}

void STNode::setVariableValue(std::string *val) {
    _variableValue = *val;
}

STNode* STNode::parameterList() {
    return _parameterList;
}

int STNode::variableArraySize() {
    return _variableArraySize;
}

bool STNode::variableIsArray() {
    return _variableIsArray;
}

int STNode::scope() {
    return _scope;
}
