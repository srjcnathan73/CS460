//
// Created by Nwseb on 2/15/2025.
//

#include <iostream>
#include <utility>
#include <cstring>
#include "Tokenizer.hpp"
#include "Constants.hpp"

void Tokenizer::addToken(std::string tokenValue, std::string tokenType) {
    auto *newNode = new Token(std::move(tokenValue), std::move(tokenType), std::move(currentLineNumber));
    if( empty() ) {
        head = tail = newNode;
    } else {
        tail->next(newNode);
        tail = newNode;
    }
}

void Tokenizer::createSingleCharToken(char c) {
    std::string currentTokenValue, currentTokenType;
    currentTokenValue += c;
    if(c == '(')
    {
        currentTokenType = "L_PAREN";
    }
    else if(c== ')')
    {
        currentTokenType = "R_PAREN";
    }
    else if(c== '[')
    {
        currentTokenType = "L_BRACKET";
    }
    else if(c== ']')
    {
        currentTokenType = "R_BRACKET";
    }
    else if(c== '{')
    {
        currentTokenType = "L_BRACE";
    }
    else if(c== '}')
    {
        currentTokenType = "R_BRACE";
    }
    else if(c== 34)
    {
        currentTokenType = "DOUBLE_QUOTE";
    }
    else if(c== 39)
    {
        currentTokenType = "SINGLE_QUOTE";
    }
    else if(c== ';')
    {
        currentTokenType = "SEMICOLON";
    }
    else if(c== ',')
    {
        currentTokenType = "COMMA";
    }
    else if(c== '=')
    {
        currentTokenType = "ASSIGNMENT_OPERATOR";
    }
    else if(c== '%')
    {
        currentTokenType = "MODULO";
    }
    else if(c== '*')
    {
        currentTokenType = "ASTERISK";
    }
    else if(c== '+')
    {
        currentTokenType = "PLUS";
    }
    else if(c== '-')
    {
        currentTokenType = "MINUS";
    }
    else if(c== 47)
    {
        currentTokenType = "DIVIDE";
    }
    else if(c== '^')
    {
        currentTokenType = "CARET";
    }
    else if(c== '<')
    {
        currentTokenType = "LT";
    }
    else if(c== '>')
    {
        currentTokenType = "GT";
    }
    else if(c== '!')
    {
        currentTokenType = " ";
    }
    else
    {
        currentTokenType = "CHARACTER";
    }
    addToken(currentTokenValue, currentTokenType);
}

void Tokenizer::createDoubleCharToken(char c, char d) {
    std::string currentTokenValue, currentTokenType;
    currentTokenValue += c;
    currentTokenValue += d;
    if(d == '=')
    {
        if(c == '=')
        {
            currentTokenType = "<BOOLEAN_EQUAL>";
        }
        else if(c== '<')
        {
            currentTokenType = "LT_EQUAL";
        }
        else if(c== '>')
        {
            currentTokenType = "GT_EQUAL";
        }
        else if(c== '!')
        {
            currentTokenType = "BOOLEAN_NOT_EQUAL";
        }
    }
    else if(c== '&' && d== '&')
    {
        currentTokenType = "BOOLEAN_AND";
    }
    else if(c== '|' && d== '|')
    {
        currentTokenType = "BOOLEAN_OR";
    }
    else
    {
        currentTokenType = "IDENTIFIER";
    }
    addToken(currentTokenValue, currentTokenType);
}

void Tokenizer::createTokens() {
    char current;
    char last;
    std::string currentTokenValue, currentTokenType, numberTest;
    int i = 0;
    int lineCount = 1;
    currentLineNumber = lineCount;
    while(i < _fileBuffer->size())
    {
        current = _fileBuffer->at(i);
        if(current == '"')
        {
            createSingleCharToken(current);
            i++;
            last = current;
            current = _fileBuffer->at(i);
            currentTokenValue = "";
            while(current != '"'&& i < _fileBuffer->size()) {
                if(current == '\n')
                {
                    lineCount++;
                    currentLineNumber = lineCount;
                }
                currentTokenValue += current;
                last = current;
                i++;
                current = _fileBuffer->at(i);
                if(last == 92 && current == 34)
                {
                    std::cerr << errorMessages[E_SYNTAX_ERROR] << lineCount << ": unterminated string quote." << std::endl;
                    exit(E_SYNTAX_ERROR);
                }
            }
            currentTokenType = "STRING";
            addToken(currentTokenValue, currentTokenType);
            createSingleCharToken(current);
            i++;
        }
        else if(current == 39)
        {
            createSingleCharToken(current);
            i++;
            last = current;
            current = _fileBuffer->at(i);
            currentTokenValue = "";
            while(current != '"'&& i < _fileBuffer->size()&& current != 39) {
                if(current == '\n')
                {
                    lineCount++;
                    currentLineNumber = lineCount;
                }
                currentTokenValue += current;
                last = current;
                i++;
                current = _fileBuffer->at(i);
                if(last == 92 && current == 34)
                {
                    std::cerr << errorMessages[E_SYNTAX_ERROR] << lineCount << ": unterminated string quote." << std::endl;
                    exit(E_SYNTAX_ERROR);
                }
            }
            currentTokenType = "STRING";
            addToken(currentTokenValue, currentTokenType);
            createSingleCharToken(current);
            i++;
        }
        else if(current == '\a'||current=='\b'||current=='\f'||current=='\r'||current=='\t'||current=='\v'||current=='\\'||current=='\?'||current=='\''||current=='\"'||current==92)
        {
            last = current;
            i++;
            current = _fileBuffer->at(i);
            if(last!=92){
                currentTokenValue = "";
                currentTokenValue += last;
                addToken(currentTokenValue, "ESCAPED_CHARACTER");
            }
            else
            {
                createSingleCharToken(last);
                if(current == 'x' )
                {
                    currentTokenValue = "";
                    currentTokenValue += current;
                    std::string hexValue;
                    i++;
                    current = _fileBuffer->at(i);
                    while(current != ' ' && current != 39 && current != '\n' &&  isNotBracketOrOperator(current)  && i < _fileBuffer->size()) {
                        hexValue += current;
                        last = current;
                        i++;
                        current = _fileBuffer->at(i);
                    }
                    currentTokenType = determineType(hexValue);
                    if(current == '\n')
                    {
                        lineCount++;
                        currentLineNumber = lineCount;
                    }
                    if(currentTokenType=="HEX_DIGIT"){
                        currentTokenValue += hexValue;
                        addToken(currentTokenValue, "ESCAPED_CHARACTER");
                    }
                    else if(currentTokenType=="INTEGER"){
                        currentTokenValue += hexValue;
                        addToken(currentTokenValue, "INTEGER");
                    }
                    else
                    {
                        std::cerr << errorMessages[E_SYNTAX_ERROR_INTEGER] << lineCount << "  " << hexValue << std::endl;
                        exit(E_SYNTAX_ERROR_INTEGER);
                    }
                    if(current == 39)
                    {
                        createSingleCharToken(current);
                    }
                    last = current;
                    i++;
                }
            }
        }
        else if(isBracketOrOperator(current))
        {
            last = current;
            i++;
            current = _fileBuffer->at(i);
            if(current=='&'||current=='='||current=='|'){
                createDoubleCharToken(last,current);
                last = current;
                i++;
            }
            else if(last=='-' && current!=' '){
                currentTokenValue = "";
                currentTokenValue += last;
                numberTest = "";
                while(current != ' ' && current != '\n' &&  isNotBracketOrOperator(current)  && i < _fileBuffer->size()) {
                    numberTest += current;
                    last = current;
                    i++;
                    current = _fileBuffer->at(i);
                }
                if(current == '\n')
                {
                    lineCount++;
                    currentLineNumber = lineCount;
                }
                if(!isNumber(numberTest) && containsFirstDigit(numberTest))
                {
                    std::cerr << errorMessages[E_SYNTAX_ERROR_INTEGER] << lineCount << std::endl;
                    exit(E_SYNTAX_ERROR_INTEGER);
                }
                currentTokenType = determineType(numberTest);
                currentTokenValue += numberTest;
                addToken(currentTokenValue, currentTokenType);
            }
            else
            {
                createSingleCharToken(last);
                if(current != ' ' && current != '\n' && isNotBracketOrOperator(current))
                {
                    currentTokenValue = "";
                    while(current != ' ' && current != '\n' &&  isNotBracketOrOperator(current)  && i < _fileBuffer->size()) {
                        currentTokenValue += current;
                        last = current;
                        i++;
                        current = _fileBuffer->at(i);
                    }
                    if(current == '\n')
                    {
                        lineCount++;
                        currentLineNumber = lineCount;
                    }
                    if(!isNumber(currentTokenValue) && containsFirstDigit(currentTokenValue))
                    {
                        std::cerr << errorMessages[E_SYNTAX_ERROR_INTEGER] << lineCount << std::endl;
                        exit(E_SYNTAX_ERROR_INTEGER);
                    }
                    if(currentTokenType=="CHARACTER"){
                        createDoubleCharToken(last,currentTokenValue[0]);
                    }
                    currentTokenType = determineType(currentTokenValue);
                    addToken(currentTokenValue, currentTokenType);
                }
            }
        }
        else if(current != ' '&& current != '\n')
        {
            currentTokenValue = "";
            while(current != ' ' && current != '\n' && isNotBracketOrOperator(current) && i < _fileBuffer->size()) {
                currentTokenValue += current;
                last = current;
                i++;
                current = _fileBuffer->at(i);
            }

            if(!isNumber(currentTokenValue) && containsFirstDigit(currentTokenValue))
            {
                std::cerr << errorMessages[E_SYNTAX_ERROR_INTEGER] << lineCount << std::endl;
                exit(E_SYNTAX_ERROR_INTEGER);
            }
            currentTokenType = determineType(currentTokenValue);
            if(currentTokenType=="CHARACTER"){
                createDoubleCharToken(last,currentTokenValue[0]);
            }
            addToken(currentTokenValue, currentTokenType);
        }
        else
        {
            if(current == '\n')
            {
                lineCount++;
                currentLineNumber = lineCount;
            }
            i++;
        }
    }
}

std::string Tokenizer::determineType(std::string tokenValue) {
    const std::string value = tokenValue;
    if(isNumber(value)){
        return "INTEGER";
    }
    else if(tokenValue.size() > 1){
        for(size_t i=0;i<value.size();i++){
            if(!((value[i] >= 'a') && (value[i] <= 'f') || (value[i] >= 'A') && (value[i] <= 'F') || (value[i] >= '0') && (value[i] <= '9')))
            {
                return "IDENTIFIER";
            }
        }
        return "HEX_DIGIT";
    }
    else {
        return "IDENTIFIER";
    }
}

bool Tokenizer::empty() {
    return head == nullptr;
}

bool Tokenizer::isBracketOrOperator(const char current)
{
    return (current=='!'||current=='#'||current=='$'||current=='%'||current=='&'||current=='('||current==')'||current=='*'||current=='+'||current==','||current=='-'||current=='.'||
           current==':'||current==';'||current=='<'||current=='='||current=='>'||current=='?'||current=='@'||current=='['||current==']'||current=='^'||current=='_'||current=='`'||
           current=='{'||current=='|'||current=='}'||current== 47);
}

bool Tokenizer::isNotBracketOrOperator(const char current)
{
    return (current!='}' && current!=']' && current!=')' && current!='"' && current!=';' && current!='(' && current!='[' && current!='{' && current!= 47
    && current!= '*' && current!= '+' && current!= '-' && current!= '=' && current!= '>' && current!= '<' && current!= ',' && current!= '!');
}

bool Tokenizer::isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool Tokenizer::containsFirstDigit(const std::string& s)
{
    return std::isdigit(s[0]);
}

void Tokenizer::print() {
    for(auto cur = head; cur; cur = cur->next() )
    {
        std::cout << "Token type: " << cur->tokenType() << std::endl;
        std::cout << "Token value: " << cur->tokenValue() << std::endl;
        std::cout << "line count: " << cur->lineNumber() << std::endl;
        std::cout << std::endl;
    }

}

void Tokenizer::outputToFile(std::string inputFileName) {
    std::ofstream newFile;
    char *addStart = (char*)"Output/output-";
    const char *copy = inputFileName.c_str();
    char newCopy[strlen(copy)];
    std::strcpy(newCopy, copy);
    newCopy[strlen(newCopy)-2] = '\0';
    char *addEnding = (char*)".txt";
    const unsigned int newWordSize = 120;
    char newFileName[newWordSize];
    std::strcpy(newFileName, addStart);
    std::strcat(newFileName, newCopy);
    std::strcat(newFileName, addEnding);

    std::cout << "Tokenized output location: " << newFileName << std::endl;
    newFile.open (newFileName);
    newFile << std::endl;
    newFile << "Token List: " << std::endl;
    newFile << std::endl;
    for(auto cur = head; cur; cur = cur->next() )
    {
        newFile << "Token type: " << cur->tokenType() << std::endl;
        newFile << "Token value: " << cur->tokenValue() << std::endl;
        newFile << "line count: " << cur->lineNumber() << std::endl;
        newFile << std::endl;
    }
    newFile.close();

}