#!/bin/bash
# UserPromptSubmit: record start time, kill old cycler, start new one.

PID_FILE="/tmp/claude_cycler.pid"

# Kill any existing cycler
if [ -f "$PID_FILE" ]; then
  kill "$(cat "$PID_FILE")" 2>/dev/null
  rm -f "$PID_FILE"
fi

# Record start time
date +%s > /tmp/claude_start_time

# Start cycler in background (sends first word immediately)
bash ~/Documents/Display/cycler.sh &
echo $! > "$PID_FILE"

exit 0
