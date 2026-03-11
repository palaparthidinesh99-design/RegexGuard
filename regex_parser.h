#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H
#include <string>
#include <stack>
using namespace std;

inline bool isOp(char c){ return c=='|'||c=='.'||c=='*'; }
inline bool isLit(char c){ return c!='|'&&c!='.'&&c!='*'&&c!='('&&c!=')'; }

inline string addConcat(const string &s){
    string r;
    for(size_t i=0;i<s.size();++i){
        char c=s[i];
        r.push_back(c);
        if(i+1<s.size()){
            char d=s[i+1];
            bool L = isLit(c) || c=='*' || c==')';
            bool R = isLit(d) || d=='(';
            if(L && R) r.push_back('.');
        }
    }
    return r;
}

inline int prec(char c){
    if(c=='*') return 3;
    if(c=='.') return 2;
    if(c=='|') return 1;
    return 0;
}

inline string toPostfix(const string &e){
    string a = addConcat(e);
    string p;
    stack<char> st;
    for(char c : a){
        if(c=='(') st.push(c);
        else if(c==')'){
            while(!st.empty() && st.top()!='('){ p.push_back(st.top()); st.pop(); }
            if(!st.empty()) st.pop();
        }
        else if(isOp(c)){
            while(!st.empty() && (prec(st.top())>prec(c) || (prec(st.top())==prec(c) && c!='*'))){
                p.push_back(st.top()); st.pop();
            }
            st.push(c);
        }
        else p.push_back(c);
    }
    while(!st.empty()){ p.push_back(st.top()); st.pop(); }
    return p;
}

#endif
