#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cstring>
#include <iomanip>
#include <chrono>
#include <thread>
#include <csignal>
#include <fstream>
#include <ctime>
#include <map>
using namespace std;

#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int sig) {
    (void)sig;
    g_running = 0;
}

struct PatternInfo {
    string pattern;
    string intrusionType;
    string severity;
    DFABuilder* dfa;
};

static inline string trim_newline(string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s;
}

static string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &in_time_t);
#else
    localtime_r(&in_time_t, &tm_buf);
#endif
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return string(buf);
}

static void print_usage(const char* prog) {
    cout << "RegexGuard Real-Time Security Agent & Log Scanner\n"
         << "Usage: " << prog << " [options]\n\n"
         << "Options:\n"
         << "  -p, --patterns <file>    Path to signature rules file (default: patterns.txt)\n"
         << "  -l, --log <file>         Path to target log file (default: log.txt)\n"
         << "  -t, --tail               Enable real-time live log tailing mode\n"
         << "  -a, --alert-log <file>   Path to output alert log file (optional)\n"
         << "  -h, --help               Show this help message\n";
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    string patternsFile = "patterns.txt";
    string logFile = "log.txt";
    string alertLogFile = "";
    bool tailMode = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-p" || arg == "--patterns") && i + 1 < argc) {
            patternsFile = argv[++i];
        } else if ((arg == "-l" || arg == "--log") && i + 1 < argc) {
            logFile = argv[++i];
        } else if ((arg == "-a" || arg == "--alert-log") && i + 1 < argc) {
            alertLogFile = argv[++i];
        } else if (arg == "-t" || arg == "--tail") {
            tailMode = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
    }

    FILE* pf = fopen(patternsFile.c_str(), "r");
    if (!pf) {
        cerr << "[ERROR] Could not open patterns file: " << patternsFile << "\n";
        return 1;
    }

    vector<PatternInfo> patterns;
    char buf[1024];

    while (fgets(buf, sizeof(buf), pf)) {
        string line = trim_newline(string(buf));
        if (line.empty() || line[0] == '#') continue;

        char pat[300], intru[300], sev[300];
        if (sscanf(line.c_str(), "%s %s %s", pat, intru, sev) != 3) continue;

        string postfix = toPostfix(pat);
        NFABuilder nfa;
        NFAFragment frag = nfa.buildFromPostfix(postfix);
        if (frag.accept >= 0 && frag.accept < (int)nfa.states.size())
            nfa.states[frag.accept].isAccept = true;

        NFABuilder clean = removeEpsilons(nfa, frag.start, frag.accept);
        DFABuilder* dfa = new DFABuilder(clean, frag.start);

        patterns.push_back({pat, intru, sev, dfa});
    }
    fclose(pf);

    if (patterns.empty()) {
        cerr << "[ERROR] No valid intrusion signature patterns loaded from " << patternsFile << "\n";
        return 1;
    }

    FILE* lf = fopen(logFile.c_str(), "r");
    if (!lf) {
        cerr << "[ERROR] Could not open log file: " << logFile << "\n";
        for (auto &p : patterns) delete p.dfa;
        return 1;
    }

    ofstream alertOut;
    if (!alertLogFile.empty()) {
        alertOut.open(alertLogFile, ios::out | ios::app);
    }

    long long lineNo = 0;
    long long totalAlerts = 0;
    map<string, long long> alertCountsByType;

    cout << "[INFO] RegexGuard Agent active. Monitoring: " << logFile << " (" 
         << (tailMode ? "LIVE TAIL MODE" : "BATCH MODE") << ")\n";
    cout << left
         << setw(8)   << "LineNo"
         << setw(52)  << "Pattern"
         << setw(105) << "LogLine"
         << setw(18)  << "Type"
         << setw(10)  << "Severity"
         << "\n";

    while (g_running) {
        if (fgets(buf, sizeof(buf), lf)) {
            lineNo++;
            string logLine = trim_newline(string(buf));
            for (auto &p : patterns) {
                if (p.dfa->RUN_DFASearch(logLine)) {
                    totalAlerts++;
                    alertCountsByType[p.intrusionType]++;

                    cout << left
                         << setw(8)   << lineNo
                         << setw(52)  << p.pattern
                         << setw(105) << logLine
                         << setw(18)  << p.intrusionType
                         << setw(10)  << p.severity
                         << "\n";

                    if (alertOut.is_open()) {
                        alertOut << "[" << get_current_timestamp() << "] "
                                 << "LINE:" << lineNo << " "
                                 << "TYPE:" << p.intrusionType << " "
                                 << "SEV:" << p.severity << " "
                                 << "PAT:" << p.pattern << " "
                                 << "LOG:\"" << logLine << "\"\n";
                        alertOut.flush();
                    }
                }
            }
        } else {
            if (tailMode && g_running) {
                clearerr(lf);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            } else {
                break;
            }
        }
    }

    fclose(lf);
    if (alertOut.is_open()) alertOut.close();

    cout << "\n=================== RegexGuard Session Summary ===================\n"
         << " Total Log Lines Processed : " << lineNo << "\n"
         << " Total Threat Alerts       : " << totalAlerts << "\n";
    for (auto &kv : alertCountsByType) {
        cout << "   - " << setw(15) << kv.first << ": " << kv.second << " alerts\n";
    }
    cout << "==================================================================\n";

    for (auto &p : patterns) {
        delete p.dfa;
    }

    return 0;
}
