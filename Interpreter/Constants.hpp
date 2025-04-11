//
// Created by Nwseb on 2/9/2025.
//

#ifndef INTERPRETER_CONSTANTS_HPP
#define INTERPRETER_CONSTANTS_HPP
#include <string>
enum Errors {
    E_OK = 0,
    // errors in main
    E_WRONG_NUMBER_ARGS = 1,  // wrong number of command-line args
    E_BAD_INPUT_FILE = 2, // provided input .json file can't be opened/read
    // tokenizer errors
    E_BAD_TOKEN = 3, // non-tokenizable character found in input

    // parser errors
    E_EXPECTED_OPEN_BRACKET = 4,  // parser expected [ and got something else
    E_EXPECTED_CLOSE_BRACKET = 5, // parser expected ] and got something else
    E_EXPECTED_OPEN_BRACE = 6, // parser expected { and got something else
    E_EXPECTED_CLOSE_BRACE = 7, // parser expected } and got something else
    E_EXPECTED_COMMA = 8, // parser expected , and got something else
    E_EXPECTED_COLON = 9, // parser expected : and got something else
    E_EXPECTED_EOF = 10, // parser expected end of file but there was more!
    E_EXPECTED_KEY = 11, // expected a quoted string as the key of a k-v pair
    E_EXPECTED_VALUE = 12, // expected quoted string in values of a k-v pair
    // shouldn't have comma after last item in a value vector or
    // after the last k-v pair
    E_COMMA_AFTER_LAST_ITEM = 13,
    E_DUPLICATE_KEYS = 14,
    E_UNCLOSED_COMMENT = 15,
    E_UNOPENED_COMMENT = 16,
    E_SYNTAX_ERROR_INTEGER = 17,
    E_SYNTAX_ERROR = 18
};

/* ****************************************************************************
 * Array of error messages
 *
 * Instead of having string literals all over your code, use these instead.
 * They work together with the enums above: you can use the enum to index
 * into this array.
 *
 * For example, instead of
 * std::cerr << "Usage: ./parser.x name-of-json-file\n"
 * do
 * std::cerr << errorMessages[E_WRONG_NUMBER_ARGS];
 *
 * This is the same thing as
 * std::cerr << errorMessages[1];
 * but easier to understand.
 *
 * Why do this instead of interspersing error messages throughout the code?
 * 1. You can see all the error messages at once.
 * 2. It makes it easier to change the look of all of the error messages.
 *    For example, if you decide you want them all to start with "!!ERROR!!!",
 *    it's easy to make that change in one place.
 * 3. It's a baby step toward the professional software engineering practice
 *    of localization, a.k.a. internationalization. Imagine that we wanted to
 *    make (for example) a Spanish-language version of this program. We'd
 *    want all of these strings in one place so we could easily swap them out
 *    for their Spanish equivalents.
 *
 * In industry, there are more full-featured tools and practices for this than
 * what we're doing here -- this is just to give you the general idea
 * and hopefully make your code a little easier to read and write.
 * ************************************************************************* */
const std::string ERROR_MSG_EXPECTED_TOKEN = "Error: Expected token ";
const std::string ERROR_MSG_GOT_TOKEN = " but got ";

const std::string errorMessages[] = {
        "", // no error
        "Usage: Wrong number of arguments",
        "Error: Unable to open the input file: ",

        "Error: Tokenizer found unexpected character in input file: ",

        // Parser errors
        ERROR_MSG_EXPECTED_TOKEN + "[" + ERROR_MSG_GOT_TOKEN,
        ERROR_MSG_EXPECTED_TOKEN + "]" + ERROR_MSG_GOT_TOKEN,
        ERROR_MSG_EXPECTED_TOKEN + "{" + ERROR_MSG_GOT_TOKEN,
        ERROR_MSG_EXPECTED_TOKEN + "}" + ERROR_MSG_GOT_TOKEN,
        ERROR_MSG_EXPECTED_TOKEN + "," + ERROR_MSG_GOT_TOKEN,
        ERROR_MSG_EXPECTED_TOKEN + ":" + ERROR_MSG_GOT_TOKEN,
        "Error: Expected end of file, but there was more file! Specifically: ",
        "Error: Expected quoted string for a key" + ERROR_MSG_GOT_TOKEN,
        "Error: Expected quoted string for the value vector" + ERROR_MSG_GOT_TOKEN,
        "Error: Unexpected comma after ",
        "Error: Duplicate key: ",
        "ERROR: Program contains C-style, unterminated comment on line: ",
        "ERROR: Program contains C-style, unopened comment terminator on line: ",
        "ERROR: Syntax error invalid integer on line: ",
        "ERROR: Syntax error on line "
};

#endif //INTERPRETER_CONSTANTS_HPP
