#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "lexer.h"

using namespace std;

struct ASTNode {
    string action;
    string communication;
    string relation;
    string time;
    bool emergency = false;
};

class Parser {
private:
    vector<Token> tokens;
    int current;

public:
    Parser(vector<Token> t);

    Token peek();
    Token advance();
    bool match(TokenType type);
    void error(string msg);

    ASTNode parseRule();
};

#endif