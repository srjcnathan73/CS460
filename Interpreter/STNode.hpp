//
// Created by Nwseb on 3/30/2025.
//

#ifndef INTERPRETER_STNODE_HPP
#define INTERPRETER_STNODE_HPP

#include <string>
#include <vector>

class STNode {
public:
    STNode(std::string identifierName, std::string identifierType, STNode *parameterList, std::string variableDataType, bool variableIsArray, int variableArraySize, std::string  variableValue, int scope);
    STNode *next();
    void next(STNode *newNode);
    std::string identifierName();
    std::string identifierType();
    STNode *parameterList();
    std::string variableDataType();
    bool variableIsArray();
    int variableArraySize();
    std::string variableValue();
    void setVariableValue(std::string *val);
    int scope();

private:
    STNode *_next;
    std::string _identifierName;
    std::string _identifierType;
    STNode *_parameterList;
    std::string _variableDataType;
    bool _variableIsArray;
    int _variableArraySize;
    std::string _variableValue;
    int _scope;
};


#endif //INTERPRETER_STNODE_HPP
