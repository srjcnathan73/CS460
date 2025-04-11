//
// Created by Nwseb on 2/9/2025.
//

#ifndef INTERPRETER_COMMENTREMOVER_HPP
#define INTERPRETER_COMMENTREMOVER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "Token.hpp"

class CommentRemover {
public:
    CommentRemover(std::string inputFileName);
    void removeComments();
    void printCommentFreeFile();
    std::vector<char>* getFileBuffer(){return &fileBuffer;};


private:
    std::fstream inputStream;
    std::vector<char> fileBuffer;
    std::string _inputFileName;
};

#endif //INTERPRETER_COMMENTREMOVER_HPP
