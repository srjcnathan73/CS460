//
// Created by Nwseb on 2/9/2025.
//
#include <string>
#include <utility>
#include "Token.hpp"

Token::Token(std::string tokenValue, std::string tokenType, int lineNumber) : _token(std::move(tokenValue)), _type(std::move(tokenType)), _lineNumber(lineNumber), _next(nullptr) {}

Token *Token::next() {
    return _next;
}

void Token::next(Token *token) {
    _next = token;
}

std::string Token::tokenValue() {
    return _token;
}

std::string Token::tokenType() {
    return _type;
}

int Token::lineNumber() {
    return _lineNumber;
}

