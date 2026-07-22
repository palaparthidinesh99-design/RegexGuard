#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <stdexcept>

using namespace std;

enum TokenType {
    TOK_CHAR,        // Exact literal character
    TOK_WILDCARD,    // Wildcard '.' (matches any character except newline)
    TOK_DIGIT,       // \d [0-9]
    TOK_WORD,        // \w [a-zA-Z0-9_]
    TOK_SPACE,       // \s whitespace
    TOK_CHARSET,     // [a-z0-9]
    TOK_LPAREN,      // (
    TOK_RPAREN,      // )
    TOK_ALT,         // |
    TOK_CONCAT,      // Explicit concatenation (internal)
    TOK_STAR,        // * (0 or more)
    TOK_PLUS,        // + (1 or more)
    TOK_QUESTION     // ? (0 or 1)
};

struct Token {
    TokenType type;
    char val = 0;
    string charSet = "";
    bool negated = false;
};

// Check operator precedence
inline int prec(TokenType t) {
    if (t == TOK_STAR || t == TOK_PLUS || t == TOK_QUESTION) return 3;
    if (t == TOK_CONCAT) return 2;
    if (t == TOK_ALT) return 1;
    return 0;
}

inline bool isOperandToken(TokenType t) {
    return t == TOK_CHAR || t == TOK_WILDCARD || t == TOK_DIGIT || 
           t == TOK_WORD || t == TOK_SPACE || t == TOK_CHARSET ||
           t == TOK_STAR || t == TOK_PLUS || t == TOK_QUESTION || 
           t == TOK_RPAREN;
}

inline bool isLeftOperandToken(TokenType t) {
    return t == TOK_CHAR || t == TOK_WILDCARD || t == TOK_DIGIT || 
           t == TOK_WORD || t == TOK_SPACE || t == TOK_CHARSET || 
           t == TOK_LPAREN;
}

// Preprocess infix regex string into a vector of Tokens
inline vector<Token> tokenizeRegex(const string &pat) {
    vector<Token> tokens;
    size_t i = 0;
    size_t n = pat.size();

    while (i < n) {
        char c = pat[i];

        if (c == '\\') {
            // Escape sequence
            if (i + 1 < n) {
                char next = pat[i + 1];
                if (next == 'd') {
                    tokens.push_back({TOK_DIGIT});
                } else if (next == 'w') {
                    tokens.push_back({TOK_WORD});
                } else if (next == 's') {
                    tokens.push_back({TOK_SPACE});
                } else {
                    // Escaped literal character (e.g., \., \*, \+, \?, \|, \(, \), \\, \[, \])
                    tokens.push_back({TOK_CHAR, next});
                }
                i += 2;
            } else {
                tokens.push_back({TOK_CHAR, '\\'});
                i++;
            }
        } else if (c == '[') {
            // Character class e.g. [a-z0-9] or [^abc]
            i++;
            bool negated = false;
            if (i < n && pat[i] == '^') {
                negated = true;
                i++;
            }
            string setChars = "";
            while (i < n && pat[i] != ']') {
                if (i + 2 < n && pat[i + 1] == '-') {
                    char startC = pat[i];
                    char endC = pat[i + 2];
                    for (char ch = startC; ch <= endC; ch++) {
                        setChars.push_back(ch);
                    }
                    i += 3;
                } else {
                    setChars.push_back(pat[i]);
                    i++;
                }
            }
            if (i < n && pat[i] == ']') i++; // skip closing ]
            tokens.push_back({TOK_CHARSET, 0, setChars, negated});
        } else if (c == '.') {
            tokens.push_back({TOK_WILDCARD});
            i++;
        } else if (c == '*') {
            tokens.push_back({TOK_STAR});
            i++;
        } else if (c == '+') {
            tokens.push_back({TOK_PLUS});
            i++;
        } else if (c == '?') {
            tokens.push_back({TOK_QUESTION});
            i++;
        } else if (c == '|') {
            tokens.push_back({TOK_ALT});
            i++;
        } else if (c == '(') {
            tokens.push_back({TOK_LPAREN});
            i++;
        } else if (c == ')') {
            tokens.push_back({TOK_RPAREN});
            i++;
        } else {
            tokens.push_back({TOK_CHAR, c});
            i++;
        }
    }
    return tokens;
}

// Insert explicit concatenation operators into token stream
inline vector<Token> addConcatTokens(const vector<Token> &tokens) {
    vector<Token> res;
    for (size_t i = 0; i < tokens.size(); ++i) {
        res.push_back(tokens[i]);
        if (i + 1 < tokens.size()) {
            TokenType currType = tokens[i].type;
            TokenType nextType = tokens[i + 1].type;
            if (isOperandToken(currType) && isLeftOperandToken(nextType)) {
                res.push_back({TOK_CONCAT});
            }
        }
    }
    return res;
}

// Shunting-Yard Algorithm: Infix Token Stream -> Postfix Token Stream using std::stack<Token>
inline vector<Token> toPostfixTokens(const string &pat) {
    vector<Token> rawTokens = tokenizeRegex(pat);
    vector<Token> infixTokens = addConcatTokens(rawTokens);
    vector<Token> postfix;
    stack<Token> opStack; // Operator stack for Shunting-Yard algorithm

    for (const auto &tok : infixTokens) {
        if (tok.type == TOK_CHAR || tok.type == TOK_WILDCARD || 
            tok.type == TOK_DIGIT || tok.type == TOK_WORD || 
            tok.type == TOK_SPACE || tok.type == TOK_CHARSET) {
            postfix.push_back(tok);
        } else if (tok.type == TOK_LPAREN) {
            opStack.push(tok);
        } else if (tok.type == TOK_RPAREN) {
            while (!opStack.empty() && opStack.top().type != TOK_LPAREN) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty() && opStack.top().type == TOK_LPAREN) {
                opStack.pop();
            }
        } else {
            // Operator (*, +, ?, ., |)
            int pCurr = prec(tok.type);
            while (!opStack.empty() && opStack.top().type != TOK_LPAREN) {
                int pTop = prec(opStack.top().type);
                if (pTop > pCurr || (pTop == pCurr && tok.type != TOK_STAR && tok.type != TOK_PLUS && tok.type != TOK_QUESTION)) {
                    postfix.push_back(opStack.top());
                    opStack.pop();
                } else {
                    break;
                }
            }
            opStack.push(tok);
        }
    }

    while (!opStack.empty()) {
        if (opStack.top().type != TOK_LPAREN && opStack.top().type != TOK_RPAREN) {
            postfix.push_back(opStack.top());
        }
        opStack.pop();
    }

    return postfix;
}

#endif
