//
// Created by Nwseb on 2/9/2025.
//

#include <iostream>
#include <cstring>
#include "Constants.hpp"
#include "CommentRemover.hpp"

CommentRemover::CommentRemover(std::string inputFileName)
{
    _inputFileName = inputFileName;
    inputStream.open(inputFileName, std::ios::in);
}

void CommentRemover::removeComments()
{
    if( !inputStream.is_open() )
    {
        std::cout << "Input stream not open. Terminating...\n";
        exit(3);
    }
    int lineCount=1;
    int commentStartLine;
    char c;
    inputStream >> c;  // skips whitespace first, reads the first
    // non-whitespace character and stores it
    // in c. Whitespace is one of space-char,
    // new-line char, tab, and vertical tab.
    if(inputStream.eof())
    {
        std::cerr << errorMessages[E_BAD_INPUT_FILE] << "empty file" << std::endl;
        exit(E_BAD_INPUT_FILE);
    }
    while(!inputStream.eof())
    {
        if(c == '*')
        {
            inputStream.get(c);
            if(c == '/')
            {
                std::cerr << errorMessages[E_UNOPENED_COMMENT] << lineCount << std::endl;
                exit(E_UNOPENED_COMMENT);
            }
            else
            {
                fileBuffer.push_back('*');
            }
        }
        if(c == '"')
        {
            fileBuffer.push_back(c);
            inputStream.get(c);
            while(c != '"' && !inputStream.eof()) {
                if(c == '\n')
                {
                    lineCount++;
                }
                fileBuffer.push_back(c);
                inputStream.get(c);
            }
            fileBuffer.push_back(c);
        }
        else if(c == '/')
        {
            char previous = c;
            inputStream.get(c);
            if(c == '/' || c == '*')
            {
                fileBuffer.push_back(' ');
                fileBuffer.push_back(' ');
            }
            if(c == '/')
            {
                inputStream.get(c);
                while(c != '\n' && !inputStream.eof()) {
                    fileBuffer.push_back(' ');
                    inputStream.get(c);
                }
                lineCount++;
                fileBuffer.push_back(c);
            }
            else if(c == '*')
            {
                commentStartLine = lineCount;
                inputStream.get(c);
                bool isComment = true;
                bool hasSecondStar = false;
                while(isComment && !inputStream.eof())
                {
                    if(c == '*')
                    {
                        hasSecondStar = true;
                        fileBuffer.push_back(' ');
                    }
                    else if(hasSecondStar && c == '/')
                    {
                        isComment = false;
                        fileBuffer.push_back(' ');
                    }
                    else if(c == '\n')
                    {
                        fileBuffer.push_back(c);
                        lineCount++;
                    }
                    else
                    {
                        fileBuffer.push_back(' ');
                    }
                    inputStream.get(c);
                }
                if(inputStream.eof())
                {
                    std::cerr << errorMessages[E_UNCLOSED_COMMENT] << commentStartLine << std::endl;
                    exit(E_UNCLOSED_COMMENT);
                }
                fileBuffer.push_back(c);

                if(c == '\n')
                {
                    lineCount++;
                }
            }
            else
            {
                if(c == '\n')
                {
                    lineCount++;
                }
                fileBuffer.push_back(previous);
                fileBuffer.push_back(c);
            }
        }
        else
        {
            if(c == '\n')
            {
                lineCount++;
            }
            fileBuffer.push_back(c);
        }
        inputStream.get(c);
    }
    //std::cout << "Total lines: " << lineCount << std::endl;
}

void CommentRemover::printCommentFreeFile() {
    std::ofstream newFile;
    char *addStart = (char*)"Output/";
    const char *copy = _inputFileName.c_str();
    char newCopy[strlen(copy)];
    std::strcpy(newCopy, copy);
    newCopy[strlen(newCopy)-2] = '\0';
    char *addEnding = (char*)"-comments_replaced_with_whitespace.c";
    const unsigned int newWordSize = 120;
    char newFileName[newWordSize];
    std::strcpy(newFileName, addStart);
    std::strcat(newFileName, newCopy);
    std::strcat(newFileName, addEnding);

    std::cout << newFileName << std::endl;
    newFile.open (newFileName);
    for(size_t i=0;i<fileBuffer.size();i++){
        newFile << fileBuffer.at(i);
    }
    newFile.close();
}