#include <iostream>
#include <fstream>
#include "Constants.hpp"
#include "CommentRemover.hpp"
#include "Tokenizer.hpp"
#include "RDParser.hpp"


void verifyReadableInputFile(char* fileName);

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 2) {
        std::cerr << errorMessages[E_WRONG_NUMBER_ARGS] << std::endl;
        exit(E_WRONG_NUMBER_ARGS);
    }
    // Verify that the command-line argument is a legitimate, readable file
    // Exit with error if not.
    verifyReadableInputFile(argv[argc - 1]);
    CommentRemover commentRemover(argv[argc - 1]);
    commentRemover.removeComments();
    //commentRemover.printCommentFreeFile();
    std::vector<char>* fileBuffer = commentRemover.getFileBuffer();
    RDParser RCParser(fileBuffer, argv[argc - 1]);
    RCParser.createCST();
    //RDParser.breadthFirstPrint();
    //RDParser.breadthFirstFilePrint(argv[argc - 1]);
    return 0;
}

void verifyReadableInputFile(char* fileName) {
    std::fstream inputStream;
    inputStream.open(fileName, std::ios::in);

    if (!inputStream.is_open()) {
        std::cerr << errorMessages[E_BAD_INPUT_FILE] << fileName << std::endl;
        exit(E_BAD_INPUT_FILE);
    }
    inputStream.close(); // close file! Parser will reopen it later.
}