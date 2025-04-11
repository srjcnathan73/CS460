//
// Created by Nwseb on 2/24/2025.
//

#ifndef INTERPRETER_CSTNODE_HPP
#define INTERPRETER_CSTNODE_HPP
#include <string>
#include <vector>
#include <fstream>
#include "Token.hpp"
class CSTNode {
public:
    CSTNode(Token *token): left(nullptr), right(nullptr), _token(token->tokenValue()), _type(token->tokenType()), _lineNumber(token->lineNumber()) {}
    ~CSTNode() {
        leftChild(nullptr);
        rightSibling(nullptr);
    }

    CSTNode *leftChild() { return left; }
    CSTNode *rightSibling() { return right; }

    void leftChild( CSTNode *newLeft ) { left = newLeft; }
    void rightSibling(CSTNode *newRight) { right = newRight; }

    std::string& value() { return _token; }
    std::string& type() { return _type; }
    int& lineNumber() { return _lineNumber; }

private:
    CSTNode *left, *right;
    std::string _token;
    std::string _type;
    int _lineNumber;
};


#endif //INTERPRETER_CSTNODE_HPP
