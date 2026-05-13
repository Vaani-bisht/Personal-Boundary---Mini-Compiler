#ifndef SEMANTIC_H
#define SEMANTIC_H

/*
 * SEMANTIC.H — Semantic Analysis
 *
 * LESSON: What is Semantic Analysis?
 *
 * The parser checks SYNTAX — "does this sentence follow the grammar rules?"
 * The semantic analyser checks MEANING — "does this sentence make sense?"
 *
 * After parsing, we have a valid AST. But the AST might still be nonsensical:
 *   "Block messages between 5 PM and 2 PM"  ← syntactically valid, but start > end!
 *   "Block messages after 25 PM"            ← 25 is not a valid hour
 *   "Block all after 10 PM if DND is DND"   ← meaningless condition
 *
 * The semantic analyser catches these cases and reports semantic errors.
 *
 * LESSON: Difference from syntax errors:
 *   Syntax error:   "Block messages BANANA 10 PM" — BANANA is not a relation
 *   Semantic error: "Block messages between 5 PM and 2 PM" — time range invalid
 *
 * This is the same distinction compilers like GCC or Clang make.
 * C++ allows int x = "hello"; to be syntactically well-formed (it parses),
 * but the type mismatch is a SEMANTIC error.
 */

#include "parser.h"
#include <vector>
#include <string>

struct SemanticError {
    string message;
    string suggestion;

    SemanticError(string m, string s = "") : message(m), suggestion(s) {}
};

class SemanticAnalyser {
    PolicyRule& rule;
    vector<SemanticError> errors;

    void checkTimeRange();
    void checkBetweenOrder();
    void checkConditionValidity();
    void checkSubjectConsistency();

public:
    SemanticAnalyser(PolicyRule& r);

    // Returns true if semantically valid
    bool analyse();

    const vector<SemanticError>& getErrors() const;
    void printErrors() const;
    void printWarnings() const;
};

#endif
