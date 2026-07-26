#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cstring>
#include<iomanip>
using namespace std;

#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"

struct PatternInfo {
    string pattern;
    string intrusionType;
    string severity;
    DFABuilder*dfa;
};

static inline string trim_newline(string s){
    while(!s.empty() && (s.back()=='\n')) s.pop_back();
    return s;
}

int main(){
    FILE* pf = fopen("patterns.txt", "r");
    if(!pf) return 1;

    vector<PatternInfo> patterns;
    char buf[1024];

    while(fgets(buf, sizeof(buf), pf)){
        string line = trim_newline(string(buf));
        if(line.empty()) continue;

        char pat[300], intru[300], sev[300];
        if(sscanf(line.c_str(), "%s %s %s", pat, intru, sev) != 3) continue;

        string postfix = toPostfix(pat);
        NFABuilder nfa;
        NFAFragment frag = nfa.buildFromPostfix(postfix);
        if(frag.accept >= 0 && frag.accept < (int)nfa.states.size())
            nfa.states[frag.accept].isAccept = true;

        NFABuilder clean = removeEpsilons(nfa, frag.start, frag.accept);
        DFABuilder* dfa = new DFABuilder(clean, frag.start);


        patterns.push_back({pat, intru, sev, dfa});
    }
    fclose(pf);

    FILE* lf = fopen("log.txt", "r");
    if(!lf) return 1;

    int lineNo = 0;
    cout << left
     << setw(8)  << "LineNo"
     << setw(50) << "Pattern"
     << setw(70) << "LogLine"
     << setw(15) << "Type"
     << setw(10) << "Severity"
     << "\n";

    while(fgets(buf, sizeof(buf), lf)){
        lineNo++;
        string logLine = trim_newline(string(buf));
        for(auto &p : patterns){
            if(p.dfa->RUN_DFASearch(logLine)){
                cout << left
                    << setw(8)  << lineNo
                    << setw(50) << p.pattern
                    << setw(70) << logLine
                    << setw(15) << p.intrusionType
                    << setw(10) << p.severity
                    << "\n";
            }
        }
    }
    fclose(lf);
    return 0;
}
