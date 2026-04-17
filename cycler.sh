#!/bin/bash
# Cycles random words on LCD while Claude is thinking.
# Killed by hook_stop.sh when Claude finishes.

FIFO="/tmp/claude_lcd"

MSGS=(
  "Thinking" "Pondering" "Overthinking" "Caffeinating"
  "Daydreaming" "Procrastinating" "Wool-gathering" "Ruminating"
  "Noodling" "Philosophizing" "Spacecadeting" "Cogitating"
  "Staring blankly" "Consulting oracle" "Asking the void"
  "Manifesting" "Yak-shaving" "Counting sheep" "Muttering darkly"
  "Buffering" "Defragging" "Dilly-dallying" "Meandering"
  "Topsy-turvying" "Snack-hunting" "Going down rabbit"
  "Gesticulating" "Shimmying" "Zesting" "Whatchamacalliting"
  "Whirlpooling" "Bootstrapping" "Ebbing" "Cross-referencing"
  "Double-checking" "Almost there" "Pacing in circles"
  "Reticulating splines" "Ionizing" "Weighing approaches"
)

while true; do
  if [ -p "$FIFO" ]; then
    IDX=$(( RANDOM % ${#MSGS[@]} ))
    echo "STATUS:${MSGS[$IDX]}" > "$FIFO"
  fi
  # Sleep 6-10 seconds so tool statuses stay visible
  sleep $(( RANDOM % 5 + 6 ))
done
