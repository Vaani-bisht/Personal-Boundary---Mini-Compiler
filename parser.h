#ifndef PARSER_H
#define PARSER_H

/*
 * PARSER.H — AST Node definitions and Parser interface
 *
 * LESSON: What is an AST (Abstract Syntax Tree)?
 *
 * After the lexer gives us a flat list of tokens, the parser's job is
 * to build a TREE that represents the grammatical structure of the sentence.
 *
 * Example sentence:
 *   "Block notifications after 10 PM if device is on DND"
 *
 * The AST for this looks like:
 *
 *              PolicyRule
 *             /     |      \
 *         Action  Subject  TimeClause   Condition
 *          |        |        |            |
 *        BLOCK  NOTIFICATIONS AFTER     IF device DND
 *                           10:00 PM
 *
 * LESSON: Why a tree instead of a flat list?
 * A tree captures NESTING and HIERARCHY. A flat list of tokens can't tell
 * you that "10 PM" belongs to the time clause, not the condition.
 * The tree makes that structure explicit, which makes semantic analysis easy.
 *
 * LESSON: Recursive Descent Parsing (your existing approach, now expanded)
 * Each grammar rule gets its own function:
 *   parseRule()       → top-level
 *   parseAction()     → ACTION + optional NEGATION
 *   parseSubject()    → one or more SUBJECT tokens
 *   parseTimeClause() → RELATION + time expression(s)
 *   parseCondition()  → IF + one or more conditions
 * Each function calls the others — that's the "recursive" part.
 */

#include "lexer.h"
#include <vector>
#include <string>

// ─────────────────────────────────────────────
//  AST NODE TYPES
//  These structs represent the nodes in our tree.
// ─────────────────────────────────────────────

// Represents one time expression: e.g. "10 PM", "9:30 AM", "midnight"
struct TimeExpr {
    int hour;       // 0-23 (24-hour format, converted during parsing)
    int minute;     // 0-59
    string raw;     // original text, e.g. "10 PM"

    TimeExpr() : hour(-1), minute(0), raw("") {}
    TimeExpr(int h, int m, string r) : hour(h), minute(m), raw(r) {}
};

// Represents the time clause: RELATION + one or two time expressions
// e.g. "after 10 PM" or "between 2 PM and 5 PM"
struct TimeClause {
    string relation;    // "AFTER", "BEFORE", "BETWEEN", "DURING"
    TimeExpr timeA;     // primary time
    TimeExpr timeB;     // secondary time (only used for BETWEEN)
    bool hasTwoTimes;   // true for BETWEEN

    TimeClause() : relation(""), hasTwoTimes(false) {}
};

// Represents one condition: e.g. "device is on DND"
// Conditions are optional. Multiple conditions can be joined by AND/OR.
struct ConditionExpr {
    string subject;   // "DEVICE", "FOCUS", etc.
    string stateVerb; // "IS", "ARE"
    string modifier;  // "ON", "OFF", "ACTIVE"
    string value;     // "DND", "SILENT", etc. (may be empty)
    string connector; // "AND" or "OR" linking to next condition (if any)

    ConditionExpr() : subject(""), stateVerb(""), modifier(""), value(""), connector("") {}
};

// The top-level AST node — one complete policy rule
struct PolicyRule {
    // Action side
    string action;      // normalised: "BLOCK" or "ALLOW"
    bool negated;       // true if "DO NOT", "NO", "NOT" present
    string rawAction;   // original action word, e.g. "MUTE", "SHUT"

    // Subject
    vector<string> subjects;  // e.g. ["NOTIFICATIONS", "CALLS"]

    // Time
    TimeClause timeClause;

    // Condition (optional)
    bool hasCondition;
    vector<ConditionExpr> conditions;

    PolicyRule() : action(""), negated(false), rawAction(""), hasCondition(false) {}

    // Pretty-print the AST for terminal display
    void print() const;

    // Emit JSON-like structured output
    string toJSON() const;
};

// ─────────────────────────────────────────────
//  PARSER CLASS
// ─────────────────────────────────────────────
class Parser {
    vector<Token> tokens;
    int current;

    Token peek();
    Token advance();
    bool check(TokenType t);
    bool match(TokenType t);
    void error(string msg);

    // Grammar rule functions (each matches one grammar production)
    void parseAction(PolicyRule& rule);
    void parseSubject(PolicyRule& rule);
    TimeClause parseTimeClause();
    TimeExpr parseTimeExpr();
    vector<ConditionExpr> parseCondition();

public:
    Parser(vector<Token> t);
    PolicyRule parseRule();
};

#endif
