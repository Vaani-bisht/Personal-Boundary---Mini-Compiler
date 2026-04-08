#include <iostream>
#include "lexer.h"
#include "parser.h"

using namespace std;

// 🔥 Function to print token type
string tokenTypeToString(TokenType type) {
    switch (type) {
        case ACTION: return "ACTION";
        case NEGATION: return "NEGATION";
        case COMM_TYPE: return "COMM_TYPE";
        case TIME_REF: return "TIME_REF";
        case SPECIAL: return "SPECIAL";
        case NUMBER: return "NUMBER";
        case MERIDIEM: return "MERIDIEM";
        default: return "UNKNOWN";
    }
}

int main() {

    string input;
    cout << "Enter rule: ";
    getline(cin, input);

    // Step 1: Lexer
    vector<Token> tokens = lexer(input);

    // 🔥 PRINT TOKENS
    cout << "\nTokens:\n";
    for (auto t : tokens) {
        if (t.type == END) continue;
        cout << "[" << tokenTypeToString(t.type) << ": " << t.value << "]\n";
    }

    // Step 2: Parser
    Parser parser(tokens);
    ASTNode ast = parser.parseRule();

    // Step 3: Output
    cout << "\nParsed Output:\n";
    cout << "Action: " << ast.action << endl;
    cout << "Type: " << ast.communication << endl;

    if (ast.relation == "anytime")
        cout << "Condition: Anytime\n";
    else
        cout << "Condition: " << ast.relation << " " << ast.time << endl;

    if (ast.emergency)
        cout << "Priority: Emergency\n";

    return 0;
}