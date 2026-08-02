#ifndef DFA_H
#define DFA_H
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <unordered_map>
#include <cstring>
using namespace std;
#include "nfa.h"

struct DFAState {
    int trans[256];
    bool isAccept = false;

    DFAState() {
        memset(trans, -1, sizeof(trans));
    }
};

struct DFABuilder {
    vector<DFAState> states;
    int startState = -1;

    int newState(){
        states.push_back(DFAState());
        return states.size()-1;
    }

    DFABuilder(const NFABuilder &nfa, int nfaStart){
        map<set<int>, int> id;
        queue<set<int>> q;

        set<int> startSet = { nfaStart };
        int d0 = newState();
        id[startSet] = d0;
        states[d0].isAccept = isAccepting(startSet, nfa);
        startState = d0;
        q.push(startSet);

        while(!q.empty()){
            set<int> curr = q.front();
            q.pop();
            int cid = id[curr];

            map<char, set<int>> moves;

            for(int st : curr){
                for(auto &p : nfa.states[st].trans){
                    char c = p.first;
                    for(int nxt : p.second)
                        moves[c].insert(nxt);
                }
            }

            for(auto &m : moves){
                char c = m.first;
                set<int> nextSet = m.second;

                if(!id.count(nextSet)){
                    int nid = newState();
                    id[nextSet] = nid;
                    states[nid].isAccept = isAccepting(nextSet, nfa);
                    q.push(nextSet);
                }

                states[cid].trans[(unsigned char)c] = id[nextSet];
            }
        }
    }

    bool isAccepting(const set<int> &subset, const NFABuilder &nfa) const{
        for(int s : subset)
            if(s >= 0 && s < (int)nfa.states.size() && nfa.states[s].isAccept)
                return true;
        return false;
    }

    bool RUN_DFASearch(const string &text) const {
        int L = text.size();
        for(int i = 0; i < L; i++){
            int cur = startState;
            for(int j = i; j < L; j++){
                unsigned char c = (unsigned char)text[j];
                int nxt = states[cur].trans[c];
                if(nxt == -1) break;
                cur = nxt;
                if(states[cur].isAccept) return true;
            }
        }
        return false;
    }
};

#endif

