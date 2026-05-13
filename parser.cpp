#include "parser.h"
#include <sstream>
#include <iomanip>

/*
 * PARSER.CPP — Recursive Descent Parser
 *
 * LESSON: Recursive Descent Parsing
 *
 * Each function below corresponds to one rule in the grammar:
 *
 *   rule         → action_phrase subject time_clause condition?
 *   action_phrase → (NEGATION* ACTION) | (ACTION NEGATION*)
 *   subject      → SUBJECT+
 *   time_clause  → RELATION time_expr (CONNECTOR time_expr)?
 *   time_expr    → NUMBER MERIDIEM | TIME_KEYWORD
 *   condition    → IF_KW cond_expr (CONNECTOR cond_expr)*
 *   cond_expr    → COND_SUBJECT COND_VERB COND_STATE COND_VALUE?
 *
 * Each function reads tokens left-to-right and calls sub-functions
 * when it expects a nested rule. This "descent" into sub-rules is
 * why it's called Recursive Descent.
 *
 * LESSON: Error recovery
 * Your original parser used exit(1) on any error.
 * We keep that for now (it's fine for a terminal demo), but in a
 * real compiler you'd collect multiple errors and report them all.
 */

Parser::Parser(vector<Token> t) : tokens(t), current(0) {}

Token Parser::peek() {
    if (current < (int)tokens.size())
        return tokens[current];
    return Token("EOF", UNKNOWN);
}

Token Parser::advance() {
    if (current < (int)tokens.size())
        return tokens[current++];
    return Token("EOF", UNKNOWN);
}

bool Parser::check(TokenType t) {
    return peek().type == t;
}

bool Parser::match(TokenType t) {
    if (check(t)) { advance(); return true; }
    return false;
}

void Parser::error(string msg) {
    cout << "\nSyntax Error: " << msg << endl;
    if (current < (int)tokens.size())
        cout << "  at token: \"" << tokens[current].value << "\"" << endl;
    exit(1);
}

// ─────────────────────────────────────────────
//  parseAction
//  Grammar: (NEGATION* ACTION) | (ACTION NEGATION*)
//
//  LESSON: Why handle both orders?
//  "DO NOT block" has negation before the action.
//  "Block NOT" is unusual but still parseable.
//  We support the common case: negation comes first.
//  We also skip over the word "DO" when it precedes NOT.
// ─────────────────────────────────────────────
void Parser::parseAction(PolicyRule& rule) {
    rule.negated = false;

    // Skip "DO" if followed by "NOT" (handles "Do Not ...")
    if (check(NEGATION) && peek().value == "DO") {
        advance(); // consume DO
        if (check(NEGATION) && peek().value == "NOT") {
            advance(); // consume NOT
            rule.negated = true;
        }
    }
    // Consume standalone NOT / NO
    else if (check(NEGATION)) {
        advance();
        rule.negated = true;
    }

    if (!check(ACTION)) {
        error("Expected an action word (BLOCK, ALLOW, MUTE, SHUT, SILENCE)");
    }

    Token act = advance();
    rule.rawAction = act.value;

    // Normalise: all negative/blocking actions → "BLOCK", positive → "ALLOW"
    // LESSON: This is the first step of semantic normalisation.
    // "MUTE notifications" means the same as "BLOCK notifications".
    // By normalising here in the parser, the semantic analyser
    // doesn't have to know about synonym lists.
    if (act.value == "ALLOW" || act.value == "ENABLE") {
        rule.action = "ALLOW";
    } else {
        // BLOCK, MUTE, SHUT, SILENCE, DISABLE all → "BLOCK"
        rule.action = "BLOCK";
    }

    // Negation flips the action:  "Do NOT block" = ALLOW,  "Do NOT allow" = BLOCK
    if (rule.negated) {
        rule.action = (rule.action == "BLOCK") ? "ALLOW" : "BLOCK";
    }

    // Optionally consume trailing negation ("BLOCK NOT messages" — unusual but handled)
    if (check(NEGATION)) {
        advance();
        rule.negated = !rule.negated;
        rule.action = (rule.action == "BLOCK") ? "ALLOW" : "BLOCK";
    }
}

// ─────────────────────────────────────────────
//  parseSubject
//  Grammar: SUBJECT+
//
//  LESSON: The + means "one or more".
//  We loop until we no longer see a SUBJECT token.
//  This allows rules like "Block messages and calls after..."
//  where two subjects are listed.
// ─────────────────────────────────────────────
void Parser::parseSubject(PolicyRule& rule) {
    if (!check(SUBJECT)) {
        error("Expected a subject (MESSAGES, CALLS, NOTIFICATIONS, ALL)");
    }

    while (check(SUBJECT)) {
        rule.subjects.push_back(advance().value);
        // Skip "AND" between subjects if present
        if (check(CONNECTOR) && peek().value == "AND") {
            advance();
        }
    }
}

// ─────────────────────────────────────────────
//  parseTimeExpr
//  Grammar: NUMBER MERIDIEM | TIME_KEYWORD
//
//  LESSON: Time conversion to 24-hour format
//  We convert at parse time so the semantic analyser works
//  in a single unified time system. This prevents errors like
//  comparing "12 PM" (noon) with "12 AM" (midnight).
//
//  Handles both "10 PM" and "10:30 PM" formats.
// ─────────────────────────────────────────────
TimeExpr Parser::parseTimeExpr() {
    // Named keyword: midnight, noon, etc.
    if (check(TIME_KEYWORD)) {
        string kw = advance().value;
        if (kw == "MIDNIGHT") return TimeExpr(0, 0, "midnight");
        if (kw == "NOON")     return TimeExpr(12, 0, "noon");
        if (kw == "SUNRISE")  return TimeExpr(6, 0, "sunrise");
        if (kw == "SUNSET")   return TimeExpr(18, 0, "sunset");
    }

    if (!check(NUMBER)) {
        error("Expected a time (e.g. 10 PM, 9:30 AM, midnight)");
    }

    string numStr = advance().value;

    // Parse HH or HH:MM
    int hour = 0, minute = 0;
    size_t colon = numStr.find(':');
    if (colon != string::npos) {
        hour   = stoi(numStr.substr(0, colon));
        minute = stoi(numStr.substr(colon + 1));
    } else {
        hour = stoi(numStr);
    }

    string raw = numStr;

    // Consume AM/PM if present
    if (check(MERIDIEM)) {
        string meridiem = advance().value;
        raw += " " + meridiem;

        // Convert to 24-hour
        // LESSON: 12-hour → 24-hour conversion rules:
        //   12 AM = 0:00 (midnight),  1–11 AM = 1–11
        //   12 PM = 12:00 (noon),     1–11 PM = 13–23
        if (meridiem == "PM" && hour != 12) hour += 12;
        if (meridiem == "AM" && hour == 12) hour = 0;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        error("Time out of range: " + raw);
    }

    return TimeExpr(hour, minute, raw);
}

// ─────────────────────────────────────────────
//  parseTimeClause
//  Grammar: RELATION time_expr (CONNECTOR time_expr)?
//
//  The CONNECTOR here is "AND" used for BETWEEN clauses:
//   "between 2 PM AND 5 PM"
// ─────────────────────────────────────────────
TimeClause Parser::parseTimeClause() {
    TimeClause tc;

    if (!check(RELATION)) {
        error("Expected a time relation (AFTER, BEFORE, BETWEEN, DURING)");
    }

    tc.relation = advance().value;
    tc.timeA = parseTimeExpr();

    // BETWEEN requires a second time connected by AND
    if (tc.relation == "BETWEEN") {
        if (check(CONNECTOR) && peek().value == "AND") {
            advance(); // consume AND
        }
        tc.timeB = parseTimeExpr();
        tc.hasTwoTimes = true;
    }

    return tc;
}

// ─────────────────────────────────────────────
//  parseCondition
//  Grammar: IF_KW cond_expr (CONNECTOR cond_expr)*
//  cond_expr: COND_SUBJECT COND_VERB? COND_STATE? COND_VALUE?
//
//  LESSON: Optional tokens
//  Not all condition words are mandatory. The user might write:
//    "if device is on DND"           → subject + verb + state + value
//    "if DND"                        → just the value
//    "if focus mode is active"       → subject + subject + verb + state
//  We handle this by checking each token type in order and accepting
//  whatever is present.
// ─────────────────────────────────────────────
vector<ConditionExpr> Parser::parseCondition() {
    vector<ConditionExpr> conditions;

    if (!check(IF_KW)) {
        error("Expected IF to start a condition");
    }
    advance(); // consume IF

    // Parse one or more condition expressions joined by AND/OR
    do {
        ConditionExpr ce;

        // Subject (optional — may just say "if DND")
        while (check(COND_SUBJECT)) {
            ce.subject += advance().value + " ";
        }
        if (!ce.subject.empty() && ce.subject.back() == ' ')
            ce.subject.pop_back();

        // State verb (optional)
        if (check(COND_VERB)) {
            ce.stateVerb = advance().value;
        }

        // ON / OFF / ACTIVE (optional)
        if (check(COND_STATE)) {
            ce.modifier = advance().value;
        }

        // Named mode/value (optional)
        if (check(COND_VALUE)) {
            ce.value = advance().value;
        }

        // If we got nothing, error
        if (ce.subject.empty() && ce.stateVerb.empty() &&
            ce.modifier.empty() && ce.value.empty()) {
            error("Empty condition expression after IF");
        }

        conditions.push_back(ce);

        // Check for AND/OR connector between conditions
        if (check(CONNECTOR)) {
            ce.connector = peek().value;
            advance(); // consume AND/OR
        } else {
            break;
        }
    } while (true);

    return conditions;
}

// ─────────────────────────────────────────────
//  parseRule — Top-level entry point
//  Grammar: action_phrase subject time_clause condition?
// ─────────────────────────────────────────────
PolicyRule Parser::parseRule() {
    PolicyRule rule;

    parseAction(rule);
    parseSubject(rule);
    rule.timeClause = parseTimeClause();

    // Condition is optional
    if (check(IF_KW)) {
        rule.hasCondition = true;
        rule.conditions = parseCondition();
    }

    // After all this, we should be at end of input
    if (current < (int)tokens.size() && peek().type != UNKNOWN) {
        // Warn about leftover tokens (don't crash — just warn)
        cerr << "Warning: unexpected tokens at end of input starting at: \""
             << peek().value << "\"" << endl;
    }

    return rule;
}

// ─────────────────────────────────────────────
//  AST PRETTY PRINTER
//  LESSON: How to visualise a tree in ASCII
//  Each level of the tree is indented further.
//  We print the structure so you can see how your input was parsed.
// ─────────────────────────────────────────────
void PolicyRule::print() const {
    cout << "  PolicyRule" << endl;
    cout << "  ├─ Action:   " << action
         << " (raw: " << rawAction << ", negated: " << (negated ? "yes" : "no") << ")" << endl;

    cout << "  ├─ Subject:  ";
    for (size_t i = 0; i < subjects.size(); ++i) {
        cout << subjects[i];
        if (i + 1 < subjects.size()) cout << ", ";
    }
    cout << endl;

    cout << "  ├─ Time:     " << timeClause.relation
         << " " << timeClause.timeA.raw;
    if (timeClause.hasTwoTimes)
        cout << " AND " << timeClause.timeB.raw;
    cout << endl;

    // Format HH:MM properly using a helper lambda
    auto fmt = [](int h, int m) -> string {
        ostringstream os;
        os << setfill('0') << setw(2) << h << ":" << setw(2) << m;
        return os.str();
    };
    cout << "  │   └─ (24h): " << timeClause.relation
         << " " << fmt(timeClause.timeA.hour, timeClause.timeA.minute);
    if (timeClause.hasTwoTimes)
        cout << " – " << fmt(timeClause.timeB.hour, timeClause.timeB.minute);
    cout << endl;

    if (hasCondition) {
        cout << "  └─ Condition:" << endl;
        for (const auto& c : conditions) {
            cout << "      IF";
            if (!c.subject.empty())  cout << " " << c.subject;
            if (!c.stateVerb.empty()) cout << " " << c.stateVerb;
            if (!c.modifier.empty()) cout << " " << c.modifier;
            if (!c.value.empty())    cout << " " << c.value;
            cout << endl;
        }
    } else {
        cout << "  └─ Condition: (none)" << endl;
    }
}

// ─────────────────────────────────────────────
//  JSON emitter
//  Produces structured output that a backend/UI can consume.
//  This is the "code generation" phase of a compiler.
// ─────────────────────────────────────────────
string PolicyRule::toJSON() const {
    ostringstream js;
    js << "{\n";
    js << "  \"action\": \"" << action << "\",\n";

    js << "  \"subjects\": [";
    for (size_t i = 0; i < subjects.size(); ++i) {
        js << "\"" << subjects[i] << "\"";
        if (i + 1 < subjects.size()) js << ", ";
    }
    js << "],\n";

    js << "  \"timeClause\": {\n";
    js << "    \"relation\": \"" << timeClause.relation << "\",\n";
    js << "    \"timeA\": { \"hour\": " << timeClause.timeA.hour
       << ", \"minute\": " << timeClause.timeA.minute
       << ", \"raw\": \"" << timeClause.timeA.raw << "\" }";

    if (timeClause.hasTwoTimes) {
        js << ",\n    \"timeB\": { \"hour\": " << timeClause.timeB.hour
           << ", \"minute\": " << timeClause.timeB.minute
           << ", \"raw\": \"" << timeClause.timeB.raw << "\" }";
    }
    js << "\n  }";

    if (hasCondition) {
        js << ",\n  \"conditions\": [\n";
        for (size_t i = 0; i < conditions.size(); ++i) {
            const auto& c = conditions[i];
            // Build fields list, then join with ", " — avoids trailing commas
            vector<string> fields;
            if (!c.subject.empty())   fields.push_back("\"subject\": \"" + c.subject + "\"");
            if (!c.stateVerb.empty()) fields.push_back("\"verb\": \"" + c.stateVerb + "\"");
            if (!c.modifier.empty())  fields.push_back("\"state\": \"" + c.modifier + "\"");
            if (!c.value.empty())     fields.push_back("\"value\": \"" + c.value + "\"");
            js << "    { ";
            for (size_t k = 0; k < fields.size(); ++k) {
                js << fields[k];
                if (k + 1 < fields.size()) js << ", ";
            }
            js << " }";
            if (i + 1 < conditions.size()) js << ",";
            js << "\n";
        }
        js << "  ]";
    }

    js << "\n}";
    return js.str();
}
