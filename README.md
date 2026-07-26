# RegexGuard: Automata-Based Intrusion Detection Engine

> **Automata-based regex engine and intrusion detection system for pattern matching over large log streams.**  
> *GitHub Release | Nov. 2025*

---

## 🛠️ Key Highlights & Technologies Used

- **Technologies Used**: `C++ (C++11)`, `Automata Theory`, `Regex Parsing`, `NFA/DFA Construction`, `Log Analysis`.
- **Custom Regex Engine**: Built a high-performance custom regular expression compilation engine using **Thompson’s Construction**, supporting full regular expression syntax parsing and $\epsilon$-transition elimination.
- **NFA $\rightarrow$ DFA Subset Construction**: Implemented powerset/subset construction to transform non-deterministic graphs into deterministic finite automata for fast, $O(N)$-time, zero-backtracking pattern matching across large-scale log datasets.
- **Rule-Based Intrusion Detection Engine**: Designed a stream-based security monitoring engine configured to identify suspicious login failures, SQL injection attempts, and malicious access patterns.

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
          ▼  dfa.h & main.cpp (Deterministic Substring Matcher over Log Streams)
[ Security Threat Alert & Tabular Analysis ]
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

### 3. NFA $\rightarrow$ DFA Subset Construction (`dfa.h`)
- **Powerset / Subset Construction (`DFABuilder`)**: Traditional regex engines that rely on NFA backtracking suffer from exponential $O(2^M)$ worst-case execution times (ReDoS vulnerabilities). RegexGuard converts the $\epsilon$-free NFA into a **Deterministic Finite Automaton (DFA)** where each DFA state corresponds to a set of NFA states.
- **Deterministic Search (`RUN_DFASearch`)**: Traverses log streams line-by-line. At every step, state transitions take $O(1)$ time with no backtracking required, guaranteeing scalable log scanning performance even across massive security dataset streams.

### 4. Rule-Based Intrusion Detection System (`main.cpp` & `patterns.txt`)
RegexGuard loads signature definitions from `patterns.txt` (formatted as `<RegexPattern> <IntrusionType> <Severity>`) and monitors log feeds (`log.txt`) to classify threat events:

| Threat Category | Pattern Rule | Detection Capability |
| :--- | :--- | :--- |
| **Suspicious Login Failures** | `(FAIL\|INVALID)*(LOGIN\|ACCESS)*` | Detects brute-force authentication attacks, repeated access failures, and unauthorized privilege escalation. |
| **SQL Injection (SQLi)** | `(SELECT\|INSERT\|UPDATE)*(FROM\|WHERE)*` | Identifies malicious payload injections, data exfiltration patterns, and unauthorized database operations. |
| **Anomalous Alert Sequences** | `(WARN\|ERROR)*(ABCD)*(WARN\|ERROR)*` | Flags anomalous system log sequences and cascading warning/error spikes indicative of system instability or exploit payloads. |

---

## 📁 Repository File Overview

| File | Description | Key Responsibilities |
| :--- | :--- | :--- |
| [`main.cpp`](file:///Users/dinesh/Documents/projects/RegexGuard/main.cpp) | Driver & Security Analyzer | Rule initialization, log stream processing, alert table renderer |
| [`regex_parser.h`](file:///Users/dinesh/Documents/projects/RegexGuard/regex_parser.h) | Regex Compiler Frontend | Concatenation insertion & Shunting-Yard postfix conversion |
| [`nfa.h`](file:///Users/dinesh/Documents/projects/RegexGuard/nfa.h) | NFA Synthesizer | Thompson's construction, $\epsilon$-closures, $\epsilon$-elimination |
| [`dfa.h`](file:///Users/dinesh/Documents/projects/RegexGuard/dfa.h) | DFA State Engine | Powerset construction & deterministic $O(1)$ transition search |
| [`patterns.txt`](file:///Users/dinesh/Documents/projects/RegexGuard/patterns.txt) | Rule Database | Security signature patterns, intrusion types, severity levels |
| [`log.txt`](file:///Users/dinesh/Documents/projects/RegexGuard/log.txt) | Log Dataset | Simulated server/application log feed |

---

## 🚀 Building & Running

### Requirements
- C++11 compliant compiler (`g++` or `clang++`).

### Build Command
```bash
g++ -std=c++11 main.cpp -o regexguard
```

### Execution
```bash
./regexguard
```

### Sample Output Report
```
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
```
