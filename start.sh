#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PID_FILE="$SCRIPT_DIR/.clouddisk.pid"

# check if already running
if [ -f "$PID_FILE" ]; then
    echo "[WARN] .clouddisk.pid found — server may already be running. Run ./stop.sh first if you need to restart."
    exit 1
fi

echo "=== Starting Server (port 8888) ==="
nohup "$SCRIPT_DIR/server" > "$SCRIPT_DIR/logs/server.log" 2>&1 &
SERVER_PID=$!
echo "  server pid: $SERVER_PID"

echo "=== Starting Consumer ==="
nohup "$SCRIPT_DIR/consumer" > "$SCRIPT_DIR/logs/consumer.log" 2>&1 &
CONSUMER_PID=$!
echo "  consumer pid: $CONSUMER_PID"

# persist PIDs
echo "$SERVER_PID $CONSUMER_PID" > "$PID_FILE"
echo "=== Done. PIDs saved to $PID_FILE ==="
