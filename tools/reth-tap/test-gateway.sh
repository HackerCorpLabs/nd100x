#!/bin/bash
#
# test-gateway.sh - run test-gateway against a throwaway gateway.
#
# Ports are high and unusual on purpose: this must never collide with a gateway
# the developer is already running, and it must never touch the repository's
# own gateway.conf.json. The config is written to a temp file and removed.
#
# The repo root is derived from this script's own location - no absolute paths.

set -e
cd "$(dirname "$0")"

GW="../nd100-gateway/gateway.js"
ETH_PORT=39400
CONF="$(mktemp -t reth-tap-gw-XXXXXX.json)"
GWLOG="$(mktemp -t reth-tap-gw-XXXXXX.log)"

cat > "$CONF" <<JSON
{
  "websocket": { "port": 39401 },
  "staticDir": "",
  "terminals": { "port": 39402, "welcome": "reth-tap test" },
  "hdlc": [],
  "ethernet": [{ "name": "ETH-TEST", "segment": 0, "port": $ETH_PORT, "enabled": true }],
  "smd": { "images": [] },
  "floppy": { "images": [] },
  "scsi": { "images": [] }
}
JSON

node "$GW" --config "$CONF" > "$GWLOG" 2>&1 &
GW_PID=$!

cleanup() {
  kill "$GW_PID" 2>/dev/null || true
  wait "$GW_PID" 2>/dev/null || true
  rm -f "$CONF" "$GWLOG"
}
trap cleanup EXIT

# Wait for the segment to be listening rather than sleeping a fixed time: a
# fixed sleep passes on a quiet machine and fails on a busy one.
for i in $(seq 1 50); do
  if (exec 3<>/dev/tcp/127.0.0.1/$ETH_PORT) 2>/dev/null; then exec 3<&- 3>&-; break; fi
  sleep 0.1
done

./test-gateway "$ETH_PORT"
