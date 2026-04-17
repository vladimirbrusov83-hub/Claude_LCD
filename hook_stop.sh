#!/bin/bash
# Stop: kill cycler, show "Churned Xm Xs".

FIFO="/tmp/claude_lcd"
PID_FILE="/tmp/claude_cycler.pid"

# Kill cycler
if [ -f "$PID_FILE" ]; then
  kill "$(cat "$PID_FILE")" 2>/dev/null
  rm -f "$PID_FILE"
fi

[ ! -p "$FIFO" ] && exit 0

# Calculate elapsed time
START_FILE="/tmp/claude_start_time"
if [ -f "$START_FILE" ]; then
  START=$(cat "$START_FILE")
  NOW=$(date +%s)
  ELAPSED=$(( NOW - START ))
  rm -f "$START_FILE"

  MINS=$(( ELAPSED / 60 ))
  SECS=$(( ELAPSED % 60 ))

  if [ "$MINS" -gt 0 ]; then
    echo "STATUS:Churned ${MINS}m ${SECS}s" > "$FIFO"
  else
    echo "STATUS:Churned ${SECS}s" > "$FIFO"
  fi
else
  echo "STATUS:Done!" > "$FIFO"
fi

exit 0
