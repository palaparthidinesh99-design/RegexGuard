
#ifndef DFA_H
#define DFA_H
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <unordered_map>
using namespace std;
#include "nfa.h"

struct DFAState {
    map<char,int> trans;
    bool isAccept = false;
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

                states[cid].trans[c] = id[nextSet];
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
        for(int i=0;i<L;i++){
            int cur = startState;
            for(int j=i;j<L;j++){
                char c = text[j];
                auto it = states[cur].trans.find(c);
                if(it == states[cur].trans.end()) break;
                cur = it->second;
                if(states[cur].isAccept) return true;
            }
        }
        return false;
    }



};

#endif

