#ifndef NFA_H
#define NFA_H

#include <vector>
#include <string>
#include <stack>
#include <algorithm>

#include "regex_parser.h"

enum MatcherType {
    MATCH_EPSILON,
    MATCH_EXACT,
    MATCH_WILDCARD,
    MATCH_DIGIT,
    MATCH_WORD,
    MATCH_SPACE,
    MATCH_CHARSET
};

struct SymbolMatcher {
    MatcherType type = MATCH_EPSILON;
    char exactChar = 0;
    string charSet = "";
    bool negated = false;

    bool matches(char c) const {
        switch (type) {
            case MATCH_EPSILON: return false;
            case MATCH_EXACT: return c == exactChar;
            case MATCH_WILDCARD: return c != '\n';
            case MATCH_DIGIT: return (c >= '0' && c <= '9');
            case MATCH_WORD: return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
            case MATCH_SPACE: return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
            case MATCH_CHARSET: {
                bool found = (charSet.find(c) != string::npos);
                return negated ? !found : found;
            }
        }
        return false;
    }
};

struct NFAState {
    vector<pair<SymbolMatcher, int>> trans;
    bool isAccept = false;
};

struct NFAFragment {
    int start;
    int accept;
};

struct NFABuilder {
    vector<NFAState> states;
    int nextId = 0;

    int newState() {
        states.emplace_back();
        return nextId++;
    }

    void addEpsilonTransition(int from, int to) {
        states[from].trans.push_back({{MATCH_EPSILON}, to});
    }

    void addSymbolTransition(int from, int to, const SymbolMatcher &matcher) {
        states[from].trans.push_back({matcher, to});
    }

    // Thompson's Construction Algorithm using std::stack<NFAFragment>
    NFAFragment buildFromPostfix(const vector<Token> &postfix) {
        stack<NFAFragment> fragStack; // Fragment stack for Thompson's construction

        for (const auto &tok : postfix) {
            if (tok.type == TOK_CHAR || tok.type == TOK_WILDCARD || 
                tok.type == TOK_DIGIT || tok.type == TOK_WORD || 
                tok.type == TOK_SPACE || tok.type == TOK_CHARSET) {
                
                SymbolMatcher matcher;
                if (tok.type == TOK_CHAR) {
                    matcher.type = MATCH_EXACT;
                    matcher.exactChar = tok.val;
                } else if (tok.type == TOK_WILDCARD) {
                    matcher.type = MATCH_WILDCARD;
                } else if (tok.type == TOK_DIGIT) {
                    matcher.type = MATCH_DIGIT;
                } else if (tok.type == TOK_WORD) {
                    matcher.type = MATCH_WORD;
                } else if (tok.type == TOK_SPACE) {
                    matcher.type = MATCH_SPACE;
                } else if (tok.type == TOK_CHARSET) {
                    matcher.type = MATCH_CHARSET;
                    matcher.charSet = tok.charSet;
                    matcher.negated = tok.negated;
                }

                int s = newState();
                int t = newState();
                addSymbolTransition(s, t, matcher);
                fragStack.push({s, t});
            }
            else if (tok.type == TOK_CONCAT) {
                if (fragStack.size() < 2) continue;
                auto B = fragStack.top(); fragStack.pop();
                auto A = fragStack.top(); fragStack.pop();

                addEpsilonTransition(A.accept, B.start);
                fragStack.push({A.start, B.accept});
            }
            else if (tok.type == TOK_ALT) {
                if (fragStack.size() < 2) continue;
                auto B = fragStack.top(); fragStack.pop();
                auto A = fragStack.top(); fragStack.pop();

                int s = newState();
                int t = newState();

                addEpsilonTransition(s, A.start);
                addEpsilonTransition(s, B.start);
                addEpsilonTransition(A.accept, t);
                addEpsilonTransition(B.accept, t);

                fragStack.push({s, t});
            }
            else if (tok.type == TOK_STAR) {
                if (fragStack.empty()) continue;
                auto A = fragStack.top(); fragStack.pop();

                int s = newState();
                int t = newState();

                addEpsilonTransition(s, A.start);
                addEpsilonTransition(s, t);
                addEpsilonTransition(A.accept, A.start);
                addEpsilonTransition(A.accept, t);

                fragStack.push({s, t});
            }
            else if (tok.type == TOK_PLUS) {
                if (fragStack.empty()) continue;
                auto A = fragStack.top(); fragStack.pop();

                int s = newState();
                int t = newState();

                addEpsilonTransition(s, A.start);
                addEpsilonTransition(A.accept, A.start);
                addEpsilonTransition(A.accept, t);

                fragStack.push({s, t});
            }
            else if (tok.type == TOK_QUESTION) {
                if (fragStack.empty()) continue;
                auto A = fragStack.top(); fragStack.pop();

                int s = newState();
                int t = newState();

                addEpsilonTransition(s, A.start);
                addEpsilonTransition(s, t);
                addEpsilonTransition(A.accept, t);

                fragStack.push({s, t});
            }
        }

        if (fragStack.empty()) {
            int s = newState();
            return {s, s};
        }
        return fragStack.top();
    }
};

// Epsilon Closure computation using std::stack<int> for DFS graph traversal
inline vector<int> epsilonClosure(const NFABuilder &nfa, const vector<int> &starts) {
    stack<int> st; // State stack for DFS graph exploration
    vector<int> closure;
    vector<bool> visited(nfa.states.size(), false);

    for (int s : starts) {
        if (s >= 0 && s < (int)nfa.states.size() && !visited[s]) {
            visited[s] = true;
            st.push(s);
            closure.push_back(s);
        }
    }

    while (!st.empty()) {
        int u = st.top();
        st.pop();

        for (const auto &tr : nfa.states[u].trans) {
            if (tr.first.type == MATCH_EPSILON) {
                int v = tr.second;
                if (v >= 0 && v < (int)nfa.states.size() && !visited[v]) {
                    visited[v] = true;
                    st.push(v);
                    closure.push_back(v);
                }
            }
        }
    }
    return closure;
}

#endif
