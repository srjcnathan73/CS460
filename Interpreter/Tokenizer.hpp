//
// Created by Nwseb on 2/15/2025.
//

#ifndef INTERPRETER_TOKENIZER_HPP
#define INTERPRETER_TOKENIZER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "Token.hpp"

class Tokenizer {
public:
    Tokenizer(std::vector<char> *fileBuffer): _fileBuffer(fileBuffer),head{nullptr}, tail{nullptr} {}
    void createTokens();
    void createSingleCharToken(char c);
    void createDoubleCharToken(char c, char d);
    void addToken(std::string tokenValue,std::string tokenType);
    std::string determineType(std::string tokenValue);
    bool isBracketOrOperator(const char current);
    bool isNotBracketOrOperator(const char current);
    bool isNumber(const std::string& s);
    bool containsFirstDigit(const std::string& s);
    bool empty();
    void print();
    void outputToFile(std::string inputFileName);
    Token* getHeadOfTokenList(){return head;};

private:
    std::vector<char> *_fileBuffer;
    Token *head, *tail;
    int currentLineNumber;
};


#endif //INTERPRETER_TOKENIZER_HPP
