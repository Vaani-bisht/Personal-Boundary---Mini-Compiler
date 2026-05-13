#ifndef LEXER_H
#define LEXER_H

/*
 * LEXER.H — Token definitions for the Personal Boundary Compiler
 *
 * LESSON: What is a Token?
 * A token is the smallest meaningful unit of your language.
 * The lexer's job is to scan raw text and label each word with a "type".
 * Think of it like a parts-of-speech tagger in English grammar class,
 * except our "grammar" is the policy language we define.
 *
 * OLD design: only 6 types — ACTION, IDENTIFIER, AFTER, NUMBER, TIME, UNKNOWN
 * NEW design: we expand the token vocabulary to cover the full grammar.
 */

#include <iostream>
#include <vector>
#include <string>
using namespace std;

enum TokenType {
    // Verb-like: what to do
    // e.g. "BLOCK", "ALLOW", "MUTE", "SHUT", "SILENCE"
    ACTION,

    // Modifier: flips the action
    // e.g. "NOT", "NO", "DO" (as in "Do Not")
    NEGATION,

    // What to restrict
    // e.g. "MESSAGES", "CALLS", "NOTIFICATIONS", "ALL"
    SUBJECT,

    // Temporal relationship
    // e.g. "AFTER", "BEFORE", "BETWEEN", "DURING"
    RELATION,

    // Numeric part of time — e.g. "10", "9", "11"
    NUMBER,

    // AM / PM
    MERIDIEM,

    // Named time keywords — e.g. "MIDNIGHT", "NOON"
    TIME_KEYWORD,

    // The word "IF"
    IF_KW,

    // Connector inside conditions — "AND", "OR"
    CONNECTOR,

    // Condition subject — e.g. "DEVICE", "FOCUS", "SCREEN"
    COND_SUBJECT,

    // Condition state verbs — e.g. "IS", "ARE"
    COND_VERB,

    // Condition modifiers — e.g. "ON", "OFF", "ACTIVE", "ENABLED"
    COND_STATE,

    // Named modes/features — e.g. "DND", "FOCUS", "SILENT"
    COND_VALUE,

    // Catch-all for unrecognised words
    UNKNOWN
};

struct Token {
    string value;
    TokenType type;
    Token(string v, TokenType t) : value(v), type(t) {}
};

class Lexer {
    string input;
    string toUpper(const string& s);

public:
    Lexer(string in);
    vector<Token> tokenize();
    static string typeToString(TokenType t);
};

#endif
