#ifndef DFA_H
#define DFA_H

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <cstring>
#include <algorithm>

#include "nfa.h"

struct DFAState {
    int trans[256]; // Direct O(1) transition lookup table per character
    bool isAccept = false;

    DFAState() {
        memset(trans, -1, sizeof(trans));
    }
};

struct MatchResult {
    bool found = false;
    int startPos = -1;
    int endPos = -1;
    string matchedText = "";
};

struct DFABuilder {
    vector<DFAState> states;
    int startState = -1;

    int newState() {
        states.push_back(DFAState());
        return (int)states.size() - 1;
    }

    DFABuilder(const NFABuilder &nfa, int nfaStart, int nfaAccept) {
        map<set<int>, int> stateMap;
        queue<set<int>> q;

        // Compute epsilon closure of initial NFA start state using stack-based traversal
        vector<int> initialClosureVec = epsilonClosure(nfa, {nfaStart});
        set<int> startSet(initialClosureVec.begin(), initialClosureVec.end());

        int d0 = newState();
        stateMap[startSet] = d0;
        startState = d0;
        states[d0].isAccept = isAccepting(startSet, nfa, nfaAccept);
        q.push(startSet);

        while (!q.empty()) {
            set<int> curr = q.front();
            q.pop();
            int cid = stateMap[curr];

            // Evaluate transitions for all ASCII characters (0 to 255)
            for (int c = 0; c < 256; ++c) {
                char ch = (char)c;
                vector<int> moveTargets;

                for (int st : curr) {
                    for (const auto &tr : nfa.states[st].trans) {
                        if (tr.first.type != MATCH_EPSILON && tr.first.matches(ch)) {
                            moveTargets.push_back(tr.second);
                        }
                    }
                }

                if (!moveTargets.empty()) {
                    vector<int> nxtClosureVec = epsilonClosure(nfa, moveTargets);
                    set<int> nextSet(nxtClosureVec.begin(), nxtClosureVec.end());

                    if (stateMap.find(nextSet) == stateMap.end()) {
                        int nid = newState();
                        stateMap[nextSet] = nid;
                        states[nid].isAccept = isAccepting(nextSet, nfa, nfaAccept);
                        q.push(nextSet);
                    }

                    states[cid].trans[c] = stateMap[nextSet];
                }
            }
        }
    }

    bool isAccepting(const set<int> &subset, const NFABuilder &nfa, int nfaAccept) const {
        for (int s : subset) {
            if (s == nfaAccept) return true;
            if (s >= 0 && s < (int)nfa.states.size() && nfa.states[s].isAccept) return true;
        }
        return false;
    }

    // High-performance DFA search over input log lines
    MatchResult RUN_DFASearch(const string &text) const {
        MatchResult res;
        int L = (int)text.size();
        for (int i = 0; i < L; ++i) {
            int cur = startState;
            if (cur < 0 || cur >= (int)states.size()) continue;

            for (int j = i; j < L; ++j) {
                unsigned char c = (unsigned char)text[j];
                int nxt = states[cur].trans[c];
                if (nxt == -1) break;
                cur = nxt;

                if (states[cur].isAccept) {
                    res.found = true;
                    res.startPos = i;
                    res.endPos = j;
                    res.matchedText = text.substr(i, j - i + 1);
                    return res;
                }
            }
        }
        return res;
    }
};

#endif
