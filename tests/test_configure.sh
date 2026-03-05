#!/bin/bash
#
# Test script: Run TPE CONFIG and validate character device output
#
# Validates that Paper Tape Punch, Line Printer, and Paper Tape Reader
# are detected correctly by the CONFIGURE hardware test program.
#

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BINARY="$PROJECT_DIR/build/bin/nd100x"

PASS=0
FAIL=0
ERRORS=""

pass() {
    PASS=$((PASS + 1))
    echo "  PASS: $1"
}

fail() {
    FAIL=$((FAIL + 1))
    ERRORS="$ERRORS\n  FAIL: $1"
    echo "  FAIL: $1"
}

check_contains() {
    local description="$1"
    local pattern="$2"
    if echo "$OUTPUT" | grep -qP "$pattern"; then
        pass "$description"
    else
        fail "$description (pattern: $pattern)"
    fi
}

check_not_contains() {
    local description="$1"
    local pattern="$2"
    if echo "$OUTPUT" | grep -qP "$pattern"; then
        fail "$description (found unexpected: $pattern)"
    else
        pass "$description"
    fi
}

# Check binary exists
if [ ! -x "$BINARY" ]; then
    echo "ERROR: Binary not found at $BINARY"
    echo "Run 'make debug' first."
    exit 1
fi

echo "Running CONFIG hardware test..."
OUTPUT=$( (sleep 8; printf 'CONFIG\r'; sleep 3; printf 'RUN\r'; sleep 30) | timeout 50 "$BINARY" --boot=floppy 2>&1 )

if [ $? -ne 0 ] && ! echo "$OUTPUT" | grep -q "END OF INVESTIGATION"; then
    echo "ERROR: Emulator failed to produce CONFIG output"
    exit 1
fi

echo ""
echo "=== Device Creation Tests ==="
check_contains "Paper Tape Reader created" "Paper Tape Reader created:.*PAPER TAPE READER 1.*CODE\[2\].*ADDRESS\[400-403\]"
check_contains "Paper Tape Punch created" "Paper Tape Punch created:.*PAPER TAPE PUNCH 1.*CODE\[2\].*ADDRESS\[410-413\]"
check_contains "Line Printer created" "Line Printer device created:.*LINE PRINTER 1.*CODE\[3\].*ADDRESS\[430-433\]"

echo ""
echo "=== CONFIG Hardware Detection Tests ==="
check_contains "CONFIG found Paper Tape Reader" "PAPER TAPE READER\s+1\s+400\s+403"
check_contains "CONFIG found Paper Tape Punch" "PAPER TAPE PUNCH\s+1\s+410\s+413"
check_contains "CONFIG found Line Printer" "LINE PRINTER\s+1\s+430\s+433"

echo ""
echo "=== CONFIG Identcode Tests ==="
check_contains "Paper Tape Reader identcode 2 on level 12" "PAPER TAPE READER\s+1\s+400\s+403\s+2"
check_contains "Paper Tape Punch identcode 2 on level 10" "PAPER TAPE PUNCH\s+1\s+410\s+413\s+2"
check_contains "Line Printer identcode 3 on level 10" "LINE PRINTER\s+1\s+430\s+433\s+3"

echo ""
echo "=== CONFIG Error Tests ==="
# All three character devices should be detected without errors
check_not_contains "No error for Paper Tape Reader" "Device number : 000400B"
check_not_contains "No error for Paper Tape Punch" "Device number : 000410B"
check_not_contains "No error for Line Printer" "Device number : 000430B"

echo ""
echo "=== CONFIG Logical Device Number Tests ==="
# Check logical device numbers in the device table
# Format: DEVICE_NAME  N  FIRST  LAST  IDENTS...  LOGDEVNO
check_contains "Paper Tape Punch LD 3" "PAPER TAPE PUNCH\s+1\s+410\s+413\s+2\s+3"
check_contains "Line Printer LD 5" "LINE PRINTER\s+1\s+430\s+433\s+3\s+5"

echo ""
echo "=== Interrupt Priority Tests ==="
check_contains "Reader on level 12 in priority table" "12\s+2\s+PAPER TAPE READER\s+1"
check_contains "Punch on level 10 in priority table" "10\s+2\s+PAPER TAPE PUNCH\s+1"
check_contains "Printer on level 10 in priority table" "10\s+3\s+LINE PRINTER\s+1"

echo ""
echo "=== CONFIG Completion ==="
check_contains "CONFIG completed" "END OF INVESTIGATION"
# No errors expected - all devices should be detected correctly
check_contains "No errors detected" "NO ERRORS DETECTED"

echo ""
echo "==============================="
echo "Results: $PASS passed, $FAIL failed"
if [ $FAIL -gt 0 ]; then
    echo -e "\nFailures:$ERRORS"
    exit 1
else
    echo "All tests passed."
    exit 0
fi
