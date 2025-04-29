//
// Created by Nwseb on 4/12/2025.
//

#ifndef INTERPRETER_ASTNODE_HPP
#define INTERPRETER_ASTNODE_HPP
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include "CSTNode.hpp"
#include "STNode.hpp"

class ASTNode {
public:
    ASTNode(CSTNode *cstNode, STNode *stNode, std::string label, int scope): left(nullptr), right(nullptr), _stNode(stNode), _token(cstNode->value()), _type(cstNode->type()), _label(std::move(label)), _lineNumber(cstNode->lineNumber()), _scope(scope) {}
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
    std::string& label() { return _label; }
    STNode *stNode() { return _stNode; }
    int& lineNumber() { return _lineNumber; }
    int& scope() { return _scope; }

private:
    ASTNode *left, *right;
    STNode *_stNode;
    std::string _token;
    std::string _type;
    std::string _label;
    int _lineNumber;
    int _scope;
};


#endif //INTERPRETER_ASTNODE_HPP
