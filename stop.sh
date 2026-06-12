#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PID_FILE="$SCRIPT_DIR/.clouddisk.pid"

if [ ! -f "$PID_FILE" ]; then
    echo "[WARN] No .clouddisk.pid found — server is not running."
    exit 0
fi

read -r SERVER_PID CONSUMER_PID < "$PID_FILE"

echo "=== Stopping Server (pid: $SERVER_PID) ==="
if kill -0 "$SERVER_PID" 2>/dev/null; then
    kill "$SERVER_PID"
    echo "  server stopped"
else
    echo "  server was not running"
fi

echo "=== Stopping Consumer (pid: $CONSUMER_PID) ==="
if kill -0 "$CONSUMER_PID" 2>/dev/null; then
    kill "$CONSUMER_PID"
    echo "  consumer stopped"
else
    echo "  consumer was not running"
fi

rm -f "$PID_FILE"
echo "=== Done ==="
