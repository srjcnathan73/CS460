//
// Created by Nwseb on 4/12/2025.
//

#ifndef INTERPRETER_ASTNODE_HPP
#define INTERPRETER_ASTNODE_HPP
#include <string>
#include <vector>
#include <fstream>
#include "CSTNode.hpp"

class ASTNode {
public:
    ASTNode(CSTNode *cstNode): left(nullptr), right(nullptr), _token(cstNode->value()), _type(cstNode->type()), _lineNumber(cstNode->lineNumber()) {}
    ~ASTNode() {
        leftChild(nullptr);
        rightSibling(nullptr);
    }

    ASTNode *leftChild() { return left; }
    ASTNode *rightSibling() { return right; }

    void leftChild( ASTNode *newLeft ) { left = newLeft; }
    void rightSibling(ASTNode *newRight) { right = newRight; }

    std::string& value() { return _token; }
    std::string& type() { return _type; }
    int& lineNumber() { return _lineNumber; }

private:
    ASTNode *left, *right;
    std::string _token;
    std::string _type;
    int _lineNumber;
};


#endif //INTERPRETER_ASTNODE_HPP
