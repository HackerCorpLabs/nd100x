#!/bin/bash
#
# Test script: Run glass UI window unit tests headlessly via Chrome
#
# Tests the Line Printer and Paper Tape JS modules against a mock DOM.
#

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_HTML="$SCRIPT_DIR/test_glass_windows.html"
CHROME=$(which google-chrome 2>/dev/null || which chromium-browser 2>/dev/null || which chromium 2>/dev/null)

if [ -z "$CHROME" ]; then
    echo "ERROR: No Chrome/Chromium browser found"
    exit 1
fi

if [ ! -f "$TEST_HTML" ]; then
    echo "ERROR: Test HTML not found at $TEST_HTML"
    exit 1
fi

echo "Running Glass UI window tests..."

# Run Chrome headless, extract console output and page title
RESULT=$("$CHROME" --headless --disable-gpu --no-sandbox --dump-dom \
    --run-all-compositor-stages-before-draw \
    --virtual-time-budget=3000 \
    "file://$TEST_HTML" 2>/dev/null)

if [ $? -ne 0 ]; then
    echo "ERROR: Chrome headless failed"
    exit 1
fi

# Extract test results from the rendered DOM
# Match only actual test result lines (contain descriptive text, not JS source)
echo "$RESULT" | grep -oP '(PASS|FAIL): [^<]+' | grep -v "description" | while read -r line; do
    echo "  $line"
done

# Check summary
PASS_COUNT=$(echo "$RESULT" | grep -c 'class="pass">  PASS')
FAIL_COUNT=$(echo "$RESULT" | grep -c 'class="fail">  FAIL')

# Also check the title which contains PASS/FAIL
TITLE=$(echo "$RESULT" | grep -oP '<title>[^<]+' | sed 's/<title>//')

echo ""
echo "==============================="
echo "Results: $PASS_COUNT passed, $FAIL_COUNT failed"

if echo "$TITLE" | grep -q "^PASS"; then
    echo "All tests passed."
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
