#!/bin/bash
# Claude Code PostToolUse hook → sends "Done!" to LCD after each tool completes.

FIFO="/tmp/claude_lcd"
[ ! -p "$FIFO" ] && exit 0

INPUT=$(cat)
TOOL=$(echo "$INPUT" | python3 -c "import sys,json; print(json.load(sys.stdin).get('tool_name',''))" 2>/dev/null)
[ -z "$TOOL" ] && exit 0

# Brief flash of completion
echo "STATUS:Done!" > "$FIFO"

exit 0
