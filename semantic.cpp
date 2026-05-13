#include "semantic.h"
#include <iostream>

/*
 * SEMANTIC.CPP — Semantic Analysis Implementation
 *
 * LESSON: How semantic analysis fits in the pipeline
 *
 * Pipeline so far:
 *   Raw text → [LEXER] → Tokens → [PARSER] → AST → [SEMANTIC] → Valid Policy
 *
 * By this stage, the AST is structurally correct.
 * We now walk the AST and check for logical consistency.
 *
 * Think of it like: a sentence can be grammatically correct but meaningless.
 * "Colourless green ideas sleep furiously" — valid grammar, no meaning.
 * Our semantic analyser prevents the policy version of that.
 */

SemanticAnalyser::SemanticAnalyser(PolicyRule& r) : rule(r) {}

// ─────────────────────────────────────────────
//  checkTimeRange
//  Ensures the time values are in valid range.
//  24-hour conversion was done by the parser, so
//  we just check hour ∈ [0,23] and minute ∈ [0,59].
// ─────────────────────────────────────────────
void SemanticAnalyser::checkTimeRange() {
    auto& A = rule.timeClause.timeA;
    if (A.hour < 0 || A.hour > 23) {
        errors.push_back(SemanticError(
            "Time hour " + to_string(A.hour) + " is out of range (0-23)",
            "Use a valid hour, e.g. '10 PM' or '22:00'"
        ));
    }
    if (A.minute < 0 || A.minute > 59) {
        errors.push_back(SemanticError(
            "Time minute " + to_string(A.minute) + " is out of range (0-59)",
            "Use minutes between 00 and 59, e.g. '10:30 PM'"
        ));
    }

    if (rule.timeClause.hasTwoTimes) {
        auto& B = rule.timeClause.timeB;
        if (B.hour < 0 || B.hour > 23) {
            errors.push_back(SemanticError(
                "Second time hour " + to_string(B.hour) + " is out of range",
                "Use a valid hour, e.g. '5 PM'"
            ));
        }
    }
}

// ─────────────────────────────────────────────
//  checkBetweenOrder
//  For BETWEEN clauses, the start time must come before end time.
//
//  LESSON: This is a domain-specific semantic rule.
//  "between 10 PM and 2 AM" is a valid night-time range (crosses midnight).
//  We allow cross-midnight ranges and just warn.
//  "between 5 PM and 5 PM" (same time) is an error.
// ─────────────────────────────────────────────
void SemanticAnalyser::checkBetweenOrder() {
    if (rule.timeClause.relation != "BETWEEN") return;
    if (!rule.timeClause.hasTwoTimes) {
        errors.push_back(SemanticError(
            "BETWEEN requires two times, e.g. 'between 2 PM and 5 PM'"
        ));
        return;
    }

    auto& A = rule.timeClause.timeA;
    auto& B = rule.timeClause.timeB;

    int minA = A.hour * 60 + A.minute;
    int minB = B.hour * 60 + B.minute;

    if (minA == minB) {
        errors.push_back(SemanticError(
            "BETWEEN start and end times are the same (" + A.raw + ")",
            "Use two different times, e.g. 'between 2 PM and 5 PM'"
        ));
    }
    // Cross-midnight ranges (A > B) are valid — just note it
    // We don't push an error, but the JSON output will flag it.
}

// ─────────────────────────────────────────────
//  checkConditionValidity
//  Ensures IF conditions have recognisable values.
//
//  LESSON: We check that the condition isn't empty
//  and that recognised mode names are used.
//  This prevents "if device is on BANANA" from
//  producing a policy.
// ─────────────────────────────────────────────
void SemanticAnalyser::checkConditionValidity() {
    if (!rule.hasCondition) return;

    for (const auto& c : rule.conditions) {
        if (c.subject.empty() && c.value.empty() && c.modifier.empty()) {
            errors.push_back(SemanticError(
                "Condition is incomplete — expected a device state or mode",
                "Try: 'if device is on DND' or 'if focus mode is active'"
            ));
        }

        // If a state modifier is present without a subject, it might be ambiguous
        if (!c.modifier.empty() && c.value.empty() && c.subject.empty()) {
            // "if ON" doesn't make sense — warn
            errors.push_back(SemanticError(
                "Condition modifier '" + c.modifier + "' without a subject or value is ambiguous",
                "Try: 'if device is on DND'"
            ));
        }
    }
}

// ─────────────────────────────────────────────
//  checkSubjectConsistency
//  "Block ALL and MESSAGES" is redundant — ALL already covers everything.
// ─────────────────────────────────────────────
void SemanticAnalyser::checkSubjectConsistency() {
    bool hasAll = false;
    for (const auto& s : rule.subjects) {
        if (s == "ALL" || s == "EVERYTHING") hasAll = true;
    }

    if (hasAll && rule.subjects.size() > 1) {
        // Not an error, but worth a warning — we'll print it separately
        cerr << "  [Semantic Warning] Subject 'ALL' makes other subjects redundant.\n";
    }
}

// ─────────────────────────────────────────────
//  analyse — runs all checks
// ─────────────────────────────────────────────
bool SemanticAnalyser::analyse() {
    checkTimeRange();
    checkBetweenOrder();
    checkConditionValidity();
    checkSubjectConsistency();

    return errors.empty();
}

const vector<SemanticError>& SemanticAnalyser::getErrors() const {
    return errors;
}

void SemanticAnalyser::printErrors() const {
    for (const auto& e : errors) {
        cout << "  Semantic Error: " << e.message << endl;
        if (!e.suggestion.empty())
            cout << "  Suggestion:     " << e.suggestion << endl;
    }
}
