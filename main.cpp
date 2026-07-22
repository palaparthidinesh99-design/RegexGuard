#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stack>
#include <algorithm>

#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"

using namespace std;

struct PatternInfo {
    string rawPattern;
    string intrusionType;
    string severity;
    int weight;
    unique_ptr<DFABuilder> dfa;
};

struct AlertRecord {
    int lineNo;
    string pattern;
    string intrusionType;
    string severity;
    int weight;
    string matchedPayload;
    string logLine;
};

static inline string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static int getSeverityWeight(const string &sev) {
    string s = sev;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "CRITICAL") return 100;
    if (s == "HIGH") return 75;
    if (s == "MEDIUM") return 50;
    if (s == "LOW") return 25;
    return 10;
}

vector<PatternInfo> loadPatterns(const string &filename) {
    vector<PatternInfo> patterns;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[-] Error opening rule file: " << filename << endl;
        return patterns;
    }

    string line;
    while (getline(file, line)) {
        string tline = trim(line);
        if (tline.empty() || tline[0] == '#') continue;

        stringstream ss(tline);
        vector<string> tokens;
        string token;
        while (ss >> token) {
            tokens.push_back(token);
        }

        if (tokens.size() < 3) continue;

        string severity = tokens.back(); tokens.pop_back();
        string intruType = tokens.back(); tokens.pop_back();

        string patStr = "";
        for (size_t i = 0; i < tokens.size(); ++i) {
            patStr += tokens[i];
            if (i + 1 < tokens.size()) patStr += " ";
        }

        vector<Token> postfix = toPostfixTokens(patStr);
        NFABuilder nfa;
        NFAFragment frag = nfa.buildFromPostfix(postfix);
        auto dfa = std::make_unique<DFABuilder>(nfa, frag.start, frag.accept);

        int weight = getSeverityWeight(severity);
        patterns.push_back({patStr, intruType, severity, weight, std::move(dfa)});
    }
    file.close();
    return patterns;
}

int main(int argc, char* argv[]) {
    string patternFile = "patterns.txt";
    string logFile = "log.txt";

    if (argc >= 2) patternFile = argv[1];
    if (argc >= 3) logFile = argv[2];

    cout << "========================================================================================\n";
    cout << "                    🛡️  REGEXGUARD INTRUSION DETECTION SYSTEM  🛡️                   \n";
    cout << "               Powered by Stack Automata & Deterministic State Engines                  \n";
    cout << "========================================================================================\n";
    cout << "[+] Loading Intrusion Signatures: " << patternFile << "\n";

    vector<PatternInfo> patterns = loadPatterns(patternFile);
    if (patterns.empty()) {
        cerr << "[-] Error loading intrusion patterns from: " << patternFile << "\n";
        return 1;
    }
    cout << "[+] Compiled " << patterns.size() << " intrusion signature DFAs successfully.\n";

    ifstream lf(logFile);
    if (!lf.is_open()) {
        cerr << "[-] Error opening log file: " << logFile << endl;
        return 1;
    }
    cout << "[+] Scanning system log file: " << logFile << " ...\n\n";

    stack<AlertRecord> alertStack;
    int lineNo = 0;
    int criticalCount = 0, highCount = 0, mediumCount = 0, lowCount = 0;
    int totalRiskPoints = 0;

    string logLine;
    while (getline(lf, logLine)) {
        lineNo++;
        string cleanLine = trim(logLine);
        if (cleanLine.empty()) continue;

        for (const auto &p : patterns) {
            MatchResult res = p.dfa->RUN_DFASearch(cleanLine);
            if (res.found) {
                int w = p.weight;
                totalRiskPoints += w;
                if (w == 100) criticalCount++;
                else if (w == 75) highCount++;
                else if (w == 50) mediumCount++;
                else lowCount++;

                alertStack.push({lineNo, p.rawPattern, p.intrusionType, p.severity, w, res.matchedText, cleanLine});
            }
        }
    }
    lf.close();

    cout << left
         << setw(8)  << "LineNo"
         << setw(18) << "Type"
         << setw(12) << "Severity"
         << setw(20) << "Payload"
         << setw(40) << "Log Line Snippet"
         << "\n";
    cout << string(98, '-') << "\n";

    vector<AlertRecord> alerts;
    while (!alertStack.empty()) {
        alerts.push_back(alertStack.top());
        alertStack.pop();
    }
    reverse(alerts.begin(), alerts.end());

    for (const auto &a : alerts) {
        string snippet = a.logLine;
        if (snippet.size() > 37) snippet = snippet.substr(0, 34) + "...";
        string payload = a.matchedPayload;
        if (payload.size() > 18) payload = payload.substr(0, 15) + "...";

        cout << left
             << setw(8)  << a.lineNo
             << setw(18) << a.intrusionType
             << setw(12) << a.severity
             << setw(20) << payload
             << setw(40) << snippet
             << "\n";
    }

    cout << string(98, '=') << "\n";
    cout << "                            📊 INTRUSION SECURITY REPORT 📊                             \n";
    cout << string(98, '=') << "\n";
    cout << " Total Log Lines Scanned  : " << lineNo << "\n";
    cout << " Total Intrusion Alerts   : " << alerts.size() << "\n";
    cout << " Critical Alerts          : " << criticalCount << "\n";
    cout << " High Severity Alerts     : " << highCount << "\n";
    cout << " Medium Severity Alerts   : " << mediumCount << "\n";
    cout << " Low Severity Alerts      : " << lowCount << "\n";

    double avgThreat = lineNo > 0 ? (double)totalRiskPoints / lineNo : 0.0;
    cout << " System Threat Score      : " << fixed << setprecision(1) << avgThreat << " / 100.0\n";

    if (criticalCount > 0 || highCount > 0) {
        cout << " Status                   : 🚨 THREAT DETECTED - IMMEDIATE ACTION REQUIRED 🚨\n";
    } else if (alerts.size() > 0) {
        cout << " Status                   : ⚠️  SUSPICIOUS ACTIVITY DETECTED ⚠️\n";
    } else {
        cout << " Status                   : ✅ SYSTEM SECURE - NO INTRUSIONS DETECTED ✅\n";
    }
    cout << string(98, '=') << "\n";

    return 0;
}
