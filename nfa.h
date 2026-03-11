#ifndef NFA_H
#define NFA_H
#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;

struct NFAState {
    map<char, vector<int>> trans;
    bool isAccept = false;
};

struct NFAFragment {
    int start;
    int accept;
};

struct NFABuilder {
    vector<NFAState> states;
    int nextId = 0;

    int newState(){ states.emplace_back(); return nextId++; }

    NFAFragment buildFromPostfix(const string &postfix){
        vector<NFAFragment> st;
        for(char c : postfix){
            if(c=='.'){
                auto B = st.back(); st.pop_back();
                auto A = st.back(); st.pop_back();
                states[A.accept].trans[0].push_back(B.start);
                st.push_back({A.start, B.accept});
            }
            else if(c=='|'){
                auto B = st.back(); st.pop_back();
                auto A = st.back(); st.pop_back();
                int s = newState(), t = newState();
                states[s].trans[0].push_back(A.start);
                states[s].trans[0].push_back(B.start);
                states[A.accept].trans[0].push_back(t);
                states[B.accept].trans[0].push_back(t);
                st.push_back({s,t});
            }
            else if(c=='*'){
                auto A = st.back(); st.pop_back();
                int s = newState(), t = newState();
                states[s].trans[0].push_back(A.start);
                states[s].trans[0].push_back(t);
                states[A.accept].trans[0].push_back(A.start);
                states[A.accept].trans[0].push_back(t);
                st.push_back({s,t});
            }
            else {
                int s = newState(), t = newState();
                states[s].trans[c].push_back(t);
                st.push_back({s,t});
            }
        }
        return st.back();
    }
};

inline vector<int> epsilonClosure(const NFABuilder &nfa, const vector<int> &starts){
    vector<int> stackv = starts;
    vector<char> seen(nfa.states.size(), 0);
    for(int x : starts) if(x>=0 && x < (int)seen.size()) seen[x]=1;
    for(size_t i=0;i<stackv.size();++i){
        int s = stackv[i];
        auto it = nfa.states[s].trans.find(0);
        if(it==nfa.states[s].trans.end()) continue;
        for(int y : it->second){
            if(!seen[y]){ seen[y]=1; stackv.push_back(y); }
        }
    }
    return stackv;
}

inline NFABuilder removeEpsilons(const NFABuilder &inNfa, int start, int accept){
    NFABuilder nfa = inNfa;
    int N = nfa.states.size();
    vector<vector<int>> closures(N);
    for(int i=0;i<N;++i) closures[i] = epsilonClosure(nfa, {i});
    for(int i=0;i<N;++i){
        for(int y : closures[i]){
            if(y == accept || nfa.states[y].isAccept) nfa.states[i].isAccept = true;
            for(auto &p : nfa.states[y].trans){
                if(p.first == 0) continue;
                auto &v = nfa.states[i].trans[p.first];
                for(int d : p.second){
                    for(int z : closures[d])
                        if(find(v.begin(), v.end(), z) == v.end())
                            v.push_back(z);
                }

            }
        }
    }
    for(auto &st : nfa.states) st.trans.erase(0);
    return nfa;
}

#endif
