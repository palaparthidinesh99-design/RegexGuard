# RegexGuard: Real-Time Automata Security Agent & Log Scanner

> **Automata-based real-time security agent and intrusion detection system for live stream monitoring and log pattern matching.**  
> *GitHub Release | Nov. 2025*

---

## 🛠️ Key Highlights & Technologies Used

- **Technologies Used**: `C++ (C++11)`, `Automata Theory`, `Regex Parsing`, `NFA/DFA Construction`, `Real-Time Log Tailing`, `Signal Handling`.
- **Custom Regex Engine**: Built a high-performance custom regular expression compilation engine using **Thompson’s Construction**, supporting full regular expression syntax parsing and $\epsilon$-transition elimination.
- **NFA $\rightarrow$ DFA Subset Construction**: Implemented powerset/subset construction to transform non-deterministic graphs into deterministic finite automata for fast, $O(N)$-time, zero-backtracking pattern matching across large-scale log datasets.
- **Real-Time Security Agent & Daemon**: Features live log file tailing (`-t` / `--tail`), ISO-8601 timestamped alert logging (`-a` / `--alert-log`), signal handling (`SIGINT`/`SIGTERM`), and session statistics summaries for DevOps & SRE environments.
- **Rule-Based Intrusion Detection Engine**: Designed to identify suspicious login failures (`LOGIN_FAIL`), SQL injection attempts (`SQL_INJECT`), and anomalous alert sequences (`ALERT_SEQ`).

---

## 🏗️ Architecture & Processing Pipeline

RegexGuard parses security rules into optimized deterministic state machines to scan log streams without performance degradation:

```
[ Infix Security Pattern ]  (e.g., "(FAIL|INVALID)*(LOGIN|ACCESS)*")
          │
          ▼  regex_parser.h (Shunting-Yard Infix to Postfix Parser & Concat Insertion)
[ Postfix Expression ]       (e.g., "FAIL.INVALID.|*LOGIN.ACCESS.|*.")
          │
          ▼  nfa.h (Thompson's Construction Engine)
[ Epsilon-NFA (ε-NFA) Graph ]
          │
          ▼  nfa.h (ε-Closure Traversal & Epsilon Transition Elimination)
[ Epsilon-Free NFA ]
          │
          ▼  dfa.h (Powerset / Subset Construction Algorithm)
[ Deterministic Finite Automaton (DFA) ]
          │
          ▼  dfa.h & main.cpp (Deterministic $O(1)$ Direct-Array Substring Matcher)
[ Real-Time Stream Scanner & ISO-8601 Alert Dispatcher ]
```

---

## 🔬 In-Depth System Components & Engineering

### 1. Custom Regex Parsing Engine (`regex_parser.h`)
The parsing module translates human-readable regular expressions into postfix form:
- **Implicit Concatenation Operator Insertion (`addConcat`)**: Standard regular expressions use implicit concatenation (e.g., `ab` instead of `a.b`). The parser analyzes character adjacency and explicitly injects concatenation operators (`.`) between adjacent literals, after `*` or `)`, and before literals or `(`.
- **Shunting-Yard Algorithm (`toPostfix`)**: Utilizes an operator stack to convert infix expressions into postfix notation (Reverse Polish Notation), maintaining strict operator precedence (`*` > `.` > `|`).

### 2. Automata Synthesis & $\epsilon$-Elimination (`nfa.h`)
- **Thompson’s Construction Algorithm (`buildFromPostfix`)**: Builds state transition graphs dynamically from postfix regex:  
  - **Concatenation (`.`)**: Joins the accept state of state machine $A$ directly to the start state of machine $B$ using an $\epsilon$-transition (`0`).
  - **Union / Alternation (`|`)**: Synthesizes a unified initial state branching into independent machines $A$ and $B$, combining their accept paths into a single final state.
  - **Kleene Star (`*`)**: Creates bypass and loopback $\epsilon$-transitions to support zero-or-more character repetitions.
- **$\epsilon$-Closure Traversal (`epsilonClosure`)**: Computes the set of all states reachable from any state $S$ by following only non-character-consuming $\epsilon$-transitions.
- **$\epsilon$-Transition Elimination (`removeEpsilons`)**: Shortcuts all $\epsilon$-paths into direct character transitions. Removing $\epsilon$-transitions drastically simplifies the NFA state space prior to DFA construction.

### 3. NFA $\rightarrow$ DFA Subset Construction & Direct-Array Transitions (`dfa.h`)
- **Powerset / Subset Construction (`DFABuilder`)**: Converts the $\epsilon$-free NFA into a **Deterministic Finite Automaton (DFA)** where each DFA state corresponds to a set of NFA states.
- **$O(1)$ Direct-Array Transitions (`DFAState`)**: Uses a contiguous 256-element array lookup (`int trans[256]`) per state instead of map traversals, delivering nanosecond-level state transition checks.
- **Deterministic Search (`RUN_DFASearch`)**: Traverses log streams line-by-line in $O(1)$ time per byte transition with no backtracking required.

### 4. Real-Time Security Agent & Rule Engine (`main.cpp` & `patterns.txt`)
RegexGuard loads signature definitions from `patterns.txt` (formatted as `<RegexPattern> <IntrusionType> <Severity>`) and monitors log feeds (`log.txt` or live stream) to classify threat events:

| Intrusion Type (Output Type) | Severity | Pattern Rule | Detection Capability |
| :--- | :--- | :--- | :--- |
| `LOGIN_FAIL` | `high` | `(FAIL\|INVALID)*(LOGIN\|ACCESS)*` | Detects suspicious login failures, brute-force authentication attempts, repeated access failures, and unauthorized privilege escalation. |
| `SQL_INJECT` | `medium` | `(SELECT\|INSERT\|UPDATE)*(FROM\|WHERE)*` | Identifies SQL injection attempts, malicious payload injections, data exfiltration patterns, and unauthorized database operations. |
| `ALERT_SEQ` | `low` | `(WARN\|ERROR)*(ABCD)*(WARN\|ERROR)*` | Flags anomalous system alert sequences and cascading warning/error spikes indicative of system instability or exploit payloads. |

---

## 📁 Repository File Overview

| File | Description | Key Responsibilities |
| :--- | :--- | :--- |
| [`main.cpp`](file:///Users/dinesh/Documents/projects/RegexGuard/main.cpp) | Driver & Real-Time Agent | CLI argument parser, signal handling, live log tailer, alert logger, session statistics |
| [`regex_parser.h`](file:///Users/dinesh/Documents/projects/RegexGuard/regex_parser.h) | Regex Compiler Frontend | Concatenation insertion & Shunting-Yard postfix conversion |
| [`nfa.h`](file:///Users/dinesh/Documents/projects/RegexGuard/nfa.h) | NFA Synthesizer | Thompson's construction, $\epsilon$-closures, $\epsilon$-elimination |
| [`dfa.h`](file:///Users/dinesh/Documents/projects/RegexGuard/dfa.h) | DFA State Engine | Powerset construction & $O(1)$ direct array transition search |
| [`patterns.txt`](file:///Users/dinesh/Documents/projects/RegexGuard/patterns.txt) | Rule Database | Security signature patterns, intrusion types, severity levels |
| [`log.txt`](file:///Users/dinesh/Documents/projects/RegexGuard/log.txt) | Log Dataset | Target server/application log feed |

---

## 🚀 Building & Running

### Requirements
- C++11 compliant compiler (`g++` or `clang++`).

### Build Command
```bash
g++ -std=c++11 main.cpp -o regexguard
```

### Command-Line Options
```text
RegexGuard Real-Time Security Agent & Log Scanner
Usage: ./regexguard [options]

Options:
  -p, --patterns <file>    Path to signature rules file (default: patterns.txt)
  -l, --log <file>         Path to target log file (default: log.txt)
  -t, --tail               Enable real-time live log tailing mode
  -a, --alert-log <file>   Path to output alert log file (optional)
  -h, --help               Show this help message
```

### Execution Examples

#### 1. Batch Log Scan
```bash
./regexguard -p patterns.txt -l log.txt
```

#### 2. Live Log Tailing & Alert Logging Mode
```bash
./regexguard -p patterns.txt -l /var/log/auth.log --tail -a regexguard_alerts.log
```

---

## 📊 Sample Output Report & Session Summary

### Console Alert Stream & Summary
```text
[INFO] RegexGuard Agent active. Monitoring: log.txt (BATCH MODE)
LineNo  Pattern                                           LogLine                                                               Type           Severity  
1       (FAIL|INVALID)*(LOGIN|ACCESS)*                    FAILFAILLOGIN user attempted login                                    LOGIN_FAIL     high      
2       (FAIL|INVALID)*(LOGIN|ACCESS)*                    INVALIDFAILACCESSACCESS system blocked access                         LOGIN_FAIL     high      
3       (SELECT|INSERT|UPDATE)*(FROM|WHERE)*              SELECTSELECTFROM table read attempt                                   SQL_INJECT     medium    
4       (SELECT|INSERT|UPDATE)*(FROM|WHERE)*              UPDATEWHEREWHERE admin forced update                                  SQL_INJECT     medium    
5       (SELECT|INSERT|UPDATE)*(FROM|WHERE)*              INSERTSELECTFROMFROM multi command sequence                           SQL_INJECT     medium    
6       (WARN|ERROR)*(ABCD)*(WARN|ERROR)*                 WARNWARNABCDERROR pattern detected                                    ALERT_SEQ      low       
7       (WARN|ERROR)*(ABCD)*(WARN|ERROR)*                 ERRORABCDABCDWARN multi warning                                       ALERT_SEQ      low       
8       (WARN|ERROR)*(ABCD)*(WARN|ERROR)*                 ABCD standalone pattern observed                                      ALERT_SEQ      low       
9       (WARN|ERROR)*(ABCD)*(WARN|ERROR)*                 WARNERRORERROR unusual alert sequence                                 ALERT_SEQ      low       

=================== RegexGuard Session Summary ===================
 Total Log Lines Processed : 12
 Total Threat Alerts       : 9
   - ALERT_SEQ      : 4 alerts
   - LOGIN_FAIL     : 2 alerts
   - SQL_INJECT     : 3 alerts
==================================================================
```

### Structured ISO-8601 Threat Log File (`regexguard_alerts.log`)
```text
[2026-08-03 02:22:38] LINE:1 TYPE:LOGIN_FAIL SEV:high PAT:(FAIL|INVALID)*(LOGIN|ACCESS)* LOG:"FAILFAILLOGIN user attempted login"
[2026-08-03 02:22:38] LINE:2 TYPE:LOGIN_FAIL SEV:high PAT:(FAIL|INVALID)*(LOGIN|ACCESS)* LOG:"INVALIDFAILACCESSACCESS system blocked access"
[2026-08-03 02:22:38] LINE:3 TYPE:SQL_INJECT SEV:medium PAT:(SELECT|INSERT|UPDATE)*(FROM|WHERE)* LOG:"SELECTSELECTFROM table read attempt"
```
