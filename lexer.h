#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>

using namespace std;

enum TokenType {
    ACTION, NEGATION, COMM_TYPE,
    TIME_REF, SPECIAL,
    NUMBER, MERIDIEM,
    END, UNKNOWN
};

struct Token {
    TokenType type;
    string value;
};

vector<Token> lexer(string input);

#endif

