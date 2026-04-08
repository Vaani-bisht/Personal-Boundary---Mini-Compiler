#include "lexer.h"
#include <sstream>
#include <cctype>
#include <iostream>

using namespace std;

TokenType identifyToken(string word) {

    if (word == "Do" || word == "Allow") return ACTION;
    if (word == "not") return NEGATION;
    if (word == "message" || word == "call" || word == "calls") return COMM_TYPE;
    if (word == "after" || word == "during") return TIME_REF;
    if (word == "anytime" || word == "emergency") return SPECIAL;
    if (word == "AM" || word == "PM") return MERIDIEM;

    bool isNumber = true;
    for (char c : word) {
        if (!isdigit(c)) {
            isNumber = false;
            break;
        }
    }

    if (isNumber) return NUMBER;

    return UNKNOWN;
}

vector<Token> lexer(string input) {

    vector<Token> tokens;
    stringstream ss(input);
    string word;

    while (ss >> word) {
        TokenType type = identifyToken(word);

        if (type == UNKNOWN) {
            cout << "Lexical Error: " << word << endl;
            exit(1);
        }

        tokens.push_back({type, word});
    }

    tokens.push_back({END, ""});
    return tokens;
}