# 🛡️ RegexGuard: Native System Intrusion Detection CLI

**RegexGuard** is a native, high-performance C++ Intrusion Detection CLI utility installed directly into your macOS/Linux system.

It uses **Stack Automata** (Shunting-Yard operator stack, Thompson NFA fragment stack, and DFS Epsilon-Closure state stack) to convert regular expression threat signatures into $O(1)$ Deterministic Finite Automata (DFA) state tables for live log scanning.

---

## 🛠️ Native System Installation

Run the installation script to build and install `regexguard` onto your laptop:

```bash
./install.sh
```

This installs:
- **Executable**: `~/.local/bin/regexguard` (or `/usr/local/bin/regexguard`)
- **Default Signatures Config**: `~/.config/regexguard/patterns.txt`

*(Optional)* Add `~/.local/bin` to your `~/.zshrc` or `~/.bashrc`:
```bash
export PATH="$HOME/.local/bin:$PATH"
```

---

## 💻 System Usage Examples

### 1. Scan System Log Files
```bash
regexguard ~/.config/regexguard/patterns.txt /var/log/system.log
```

### 2. Stream Live System Log Streams
```bash
tail -f /var/log/system.log | regexguard ~/.config/regexguard/patterns.txt -
```

### 3. Scan Any Log File (Default Rules Auto-Discovered)
If no rule path is specified, `regexguard` automatically uses `~/.config/regexguard/patterns.txt`:
```bash
regexguard ~/.config/regexguard/patterns.txt log.txt
```

---

## 📁 Project Files

```
RegexGuard/
├── install.sh        # System installer script
├── main.cpp          # Native C++ CLI entry point & system config loader
├── regex_parser.h    # Infix-to-postfix tokenizer and Shunting Yard operator stack
├── nfa.h             # Thompson's NFA construction & DFS epsilon closure state stack
├── dfa.h             # Subset construction & O(1) state lookup DFA search engine
├── patterns.txt      # Master intrusion signatures database
├── log.txt           # Sample system audit log file with attack vectors
├── Makefile          # Build compilation script
└── README.md         # Native system integration documentation
```
