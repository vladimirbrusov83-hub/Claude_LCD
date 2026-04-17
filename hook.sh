#!/bin/bash
# Claude Code Hook → LCD Monitor
# Reads tool JSON from stdin, sends status to Arduino via FIFO.

FIFO="/tmp/claude_lcd"
[ ! -p "$FIFO" ] && exit 0

# Read JSON from stdin
INPUT=$(cat)

# Extract tool_name
TOOL=$(echo "$INPUT" | python3 -c "import sys,json; print(json.load(sys.stdin).get('tool_name',''))" 2>/dev/null)
[ -z "$TOOL" ] && exit 0

# Map tool → display status
case "$TOOL" in
  Read)              STATUS="Reading" ;;
  Edit|Write)        STATUS="Writing" ;;
  Bash)              STATUS="Running" ;;
  Grep)              STATUS="Searching" ;;
  Glob)              STATUS="Spelunking" ;;
  WebFetch)          STATUS="Fetching" ;;
  WebSearch)         STATUS="Googling" ;;
  Agent)             STATUS="Collaborating" ;;
  NotebookEdit)      STATUS="Editing" ;;
  TaskCreate)        STATUS="Plotting" ;;
  TaskUpdate)        STATUS="Plotting" ;;
  ToolSearch)        STATUS="Rummaging" ;;
  Skill)             STATUS="Leveling Up" ;;
  *)
    MSGS=(
      "Procrastinating" "Overthinking" "Caffeinating" "Pondering"
      "Vibing" "Computing" "Topsy-turvying" "Daydreaming"
      "Noodling" "Philosophizing" "Wool-gathering" "Ruminating"
      "Meandering" "Spacecadeting" "Dilly-dallying" "Cogitating"
      "Staring blankly" "Snack-hunting" "Buffering" "Defragging"
      "Consulting oracle" "Asking the void" "Manifesting" "Yak-shaving"
      "Going down rabbit" "Counting sheep" "Muttering darkly"
    )
    IDX=$(( RANDOM % ${#MSGS[@]} ))
    STATUS="${MSGS[$IDX]}"
    ;;
esac

# Extract file_path or url for detail line
DETAIL=$(echo "$INPUT" | python3 -c "
import sys,json
d=json.load(sys.stdin).get('tool_input',{})
p=d.get('file_path','') or d.get('url','') or d.get('command','') or d.get('pattern','')
# Show just the filename or last 18 chars
if '/' in p:
    p=p.split('/')[-1]
print(p[:18])
" 2>/dev/null)

echo "STATUS:$STATUS" > "$FIFO"
[ -n "$DETAIL" ] && echo "DETAIL:$DETAIL" > "$FIFO"

exit 0
