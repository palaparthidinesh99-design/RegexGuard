#!/bin/bash

# RegexGuard Native System Installation Script
set -e

echo "=========================================================="
echo "      🛡️ Installing RegexGuard Intrusion Detection      "
echo "=========================================================="

# 1. Compile project
echo "[+] Compiling C++ state machine engine..."
make clean && make

# 2. Setup user configuration directory
CONFIG_DIR="$HOME/.config/regexguard"
mkdir -p "$CONFIG_DIR"

echo "[+] Copying intrusion rules to $CONFIG_DIR/patterns.txt..."
cp patterns.txt "$CONFIG_DIR/patterns.txt"

# 3. Target PATH directory
TARGET_DIR="$HOME/.gemini/antigravity-ide/bin"
mkdir -p "$TARGET_DIR"

TARGET_BIN="$TARGET_DIR/regexguard"
echo "[+] Installing binary to $TARGET_BIN..."
cp regexguard "$TARGET_BIN"
chmod +x "$TARGET_BIN"

echo "=========================================================="
echo " ✅ Installation Complete!"
echo " RegexGuard installed to: $TARGET_BIN"
echo " Run 'regexguard patterns.txt /var/log/system.log'"
echo "=========================================================="
