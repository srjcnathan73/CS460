//
// Created by Nwseb on 2/24/2025.
//

#ifndef INTERPRETER_RDPARSER_HPP
#define INTERPRETER_RDPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "Token.hpp"
#include "CSTNode.hpp"
#include "SymbolTable.hpp"

class RDParser {
public:
    RDParser(std::vector<char> *fileBuffer, std::string fileName);
    void createCST();
    void printCST();
    void printCSTUtility(CSTNode* node);
    void breadthFirstPrint();
    void breadthFirstFilePrint(std::string inputFileName);
    bool checkForReservedWords(const std::string& word);
    bool checkForNegativeInteger(const std::string& num);
    bool checkIsOperator(const std::string& type);
    CSTNode* getRootOfCST(){return root;};


private:
    Token *headTokenizer, *previous;
    CSTNode *root;
    int lastLine;
    std::vector<std::string> reservedWords = {"char","int","procedure","function","printf","void"};
    std::vector<std::string>* _reservedWords = &reservedWords;
    std::string _fileName;
};


#endif //INTERPRETER_RDPARSER_HPP
