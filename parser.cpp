#include "parser.h"
#include <iostream>

using namespace std;

Parser::Parser(vector<Token> t) {
    tokens = t;
    current = 0;
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::advance() {
    return tokens[current++];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::error(string msg) {
    cout << "Syntax Error: " << msg << endl;
    exit(1);
}

ASTNode Parser::parseRule() {

    ASTNode node;

    // ACTION
    if (match(ACTION)) {
        node.action = tokens[current - 1].value;

        if (node.action == "Do") {
            if (!match(NEGATION))
                error("Expected 'not'");
            node.action = "DoNot";
        }
    } else error("Expected action");

    // OPTIONAL EMERGENCY
    if (match(SPECIAL)) {
        if (tokens[current - 1].value == "emergency")
            node.emergency = true;
    }

    // COMMUNICATION
    if (match(COMM_TYPE)) {
        node.communication = tokens[current - 1].value;
    } else error("Expected communication");

    // TIME CONDITION
    if (match(TIME_REF)) {
        node.relation = tokens[current - 1].value;

        if (match(NUMBER)) {
            node.time = tokens[current - 1].value;

            if (match(MERIDIEM)) {
                node.time += " " + tokens[current - 1].value;
            } else error("Expected AM/PM");

        } else error("Expected number");
    }
    else if (match(SPECIAL)) {
        if (tokens[current - 1].value == "anytime") {
            node.relation = "anytime";
        }
    }
    else error("Invalid condition");

    return node;
}