//
// Created by Nwseb on 2/9/2025.
//

#ifndef INTERPRETER_TOKEN_HPP
#define INTERPRETER_TOKEN_HPP

#include <string>

class Token {
public:
    Token(std::string tokenValue, std::string tokenType, int lineNumber);
    Token *next();
    void next(Token *token);
    std::string tokenValue();
    std::string tokenType();
    int lineNumber();

private:
    Token *_next;
    std::string _token;
    std::string _type;
    int _lineNumber;
};

#endif //INTERPRETER_TOKEN_HPP
