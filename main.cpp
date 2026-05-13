#include <iostream>
#include <iomanip>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

using namespace std;

// ─────────────────────────────────────────────
//  Print a divider line
// ─────────────────────────────────────────────
void divider(const string& title) {
    cout << "\n";
    cout << "══════════════════════════════════════════════════════\n";
    cout << "  " << title << "\n";
    cout << "══════════════════════════════════════════════════════\n";
}

int main() {
    cout << "\n";
    cout << "╔════════════════════════════════════════════════════╗\n";
    cout << "║       Personal Boundary Compiler  v2.0             ║\n";
    cout << "║       Lexical + Syntax + Semantic Analysis         ║\n";
    cout << "╚════════════════════════════════════════════════════╝\n";
    cout << "\n  Example rules you can try:\n";
    cout << "    Do not message after 10 PM\n";
    cout << "    Shut notifications before 9 AM\n";
    cout << "    Block calls between 2 PM and 5 PM\n";
    cout << "    Mute all after 11 PM if device is on DND\n";
    cout << "    Silence notifications before 8 AM if focus mode is active\n";
    cout << "\n  Enter your rule:\n  > ";

    string input;
    getline(cin, input);

    // ════════════════════════════════════════════
    //  PHASE 1: LEXICAL ANALYSIS
    // ════════════════════════════════════════════
    divider("PHASE 1 — LEXICAL ANALYSIS");

    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();

    cout << "\n  Token stream:\n\n";
    cout << "  " << string(42, '-') << "\n";
    cout << "  " << left;
    cout << setw(20) << "Word" << setw(16) << "Token Type" << "\n";
    cout << "  " << string(42, '-') << "\n";
    for (const auto& t : tokens) {
        string status = (t.type == UNKNOWN) ? " <-- UNKNOWN" : "";
        cout << "  " << setw(20) << t.value
             << setw(16) << Lexer::typeToString(t.type)
             << status << "\n";
    }
    cout << "  " << string(42, '-') << "\n";

    bool hasUnknown = false;
    for (const auto& t : tokens) {
        if (t.type == UNKNOWN) { hasUnknown = true; break; }
    }
    if (hasUnknown) {
        cout << "\n  [Warning] Some words were not recognised.\n";
        cout << "  They will be skipped during parsing.\n";
    }

    // ════════════════════════════════════════════
    //  PHASE 2: SYNTAX ANALYSIS (PARSING)
    // ════════════════════════════════════════════
    divider("PHASE 2 — SYNTAX ANALYSIS (PARSING)");

    // Filter out UNKNOWN tokens before parsing
    vector<Token> filtered;
    for (const auto& t : tokens) {
        if (t.type != UNKNOWN) filtered.push_back(t);
    }

    Parser parser(filtered);
    PolicyRule rule = parser.parseRule();

    cout << "\n  Syntax is correct.\n";
    cout << "\n  Abstract Syntax Tree:\n\n";
    rule.print();

    // ════════════════════════════════════════════
    //  PHASE 3: SEMANTIC ANALYSIS
    // ════════════════════════════════════════════
    divider("PHASE 3 — SEMANTIC ANALYSIS");

    SemanticAnalyser sem(rule);
    bool valid = sem.analyse();

    if (valid) {
        cout << "\n  All semantic checks passed.\n";
        cout << "\n  Interpretation:\n";

        cout << "    ACTION  : " << rule.action;
        if (rule.negated) cout << " (negated from " << rule.rawAction << ")";
        cout << "\n";

        cout << "    SUBJECTS: ";
        for (size_t i = 0; i < rule.subjects.size(); ++i) {
            cout << rule.subjects[i];
            if (i + 1 < rule.subjects.size()) cout << ", ";
        }
        cout << "\n";

        cout << "    WHEN    : " << rule.timeClause.relation
             << " " << rule.timeClause.timeA.hour << ":"
             << setfill('0') << setw(2) << rule.timeClause.timeA.minute;
        if (rule.timeClause.hasTwoTimes) {
            cout << " – " << rule.timeClause.timeB.hour << ":"
                 << setw(2) << rule.timeClause.timeB.minute;
        }
        cout << " (24-hour)\n";

        if (rule.hasCondition) {
            cout << "    CONDITION:";
            for (const auto& c : rule.conditions) {
                cout << " IF";
                if (!c.subject.empty())  cout << " " << c.subject;
                if (!c.stateVerb.empty()) cout << " " << c.stateVerb;
                if (!c.modifier.empty()) cout << " " << c.modifier;
                if (!c.value.empty())    cout << " " << c.value;
            }
            cout << "\n";
        }

    } else {
        cout << "\n  Semantic errors found:\n\n";
        sem.printErrors();
        cout << "\n  Policy not generated due to semantic errors.\n";
        return 1;
    }

    // ════════════════════════════════════════════
    //  PHASE 4: CODE GENERATION — JSON Policy
    // ════════════════════════════════════════════
    divider("PHASE 4 — OUTPUT: STRUCTURED POLICY (JSON)");
    cout << "\n" << rule.toJSON() << "\n";

    cout << "\n══════════════════════════════════════════════════════\n";
    cout << "  Compilation successful.\n";
    cout << "══════════════════════════════════════════════════════\n\n";

    return 0;
}
