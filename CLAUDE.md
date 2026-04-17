# Claude LCD Monitor

Arduino Uno + SainSmart LCD2004 (20×4 I2C) showing Claude Code activity in real time.

## Start / stop

```bash
python3 ~/Documents/Display/monitor.py &   # start
pkill -f monitor.py                         # stop
```

Run once per session. Auto-detects Arduino on `/dev/cu.usbmodem*`. Creates FIFO at `/tmp/claude_lcd`.

---

## Architecture

```
Claude Code hooks → shell scripts → FIFO /tmp/claude_lcd → monitor.py → serial → Arduino → LCD
```

### Files

| File | Purpose |
|------|---------|
| `monitor.py` | Daemon. Holds serial open, reads FIFO, forwards to Arduino. |
| `hook_submit.sh` | UserPromptSubmit hook. Kills old cycler, starts new one, records start time. |
| `hook_stop.sh` | Stop hook. Kills cycler, shows "Churned Xm Xs". |
| `hook.sh` | PreToolUse hook. Maps tool name → status string. Writes STATUS:/DETAIL: to FIFO. |
| `cycler.sh` | Background loop. Sends random words to FIFO every 6–10s while Claude thinks. |
| `claude_monitor/claude_monitor.ino` | Arduino firmware v1.2. Reads serial, renders on LCD. |
| `lcd_designer.html` | Browser tool to design custom chars + animations. Open with `open lcd_designer.html`. |
| `i2c_scanner.ino` | Utility sketch to find LCD I2C address (0x27 or 0x3F). |
| `activity_watcher.py` | Optional. Sends CPU/RAM to LCD on mouse/keyboard activity. |

### Hook wiring (`~/.claude/settings.json`)

- `UserPromptSubmit` → `hook_submit.sh`
- `PreToolUse` → `hook.sh`
- `Stop` → `hook_stop.sh`

---

## Serial protocol (Python → Arduino)

| Command | Effect |
|---------|--------|
| `STATUS:Thinking` | Centered on row 1 with animated dots |
| `DETAIL:filename.py` | Row 2 |
| `SYSINFO:CPU: 4% MEM:60%` | Row 3 (always visible) |
| `CLEAR` | Show idle screen |

Arduino I2C address: `0x3F` · Baud: `9600`

---

## Tool → status mapping (`hook.sh`)

| Tool | Display |
|------|---------|
| Read | Reading |
| Edit / Write | Writing |
| Bash | Running |
| Grep | Searching |
| Glob | Spelunking |
| WebSearch | Googling |
| WebFetch | Fetching |
| Agent | Collaborating |
| TaskCreate / TaskUpdate | Plotting |
| ToolSearch | Rummaging |
| Skill | Leveling Up |
| anything else | Random from fun pool |

### Fun word pool (cycler + unknown tools)

Thinking, Pondering, Overthinking, Caffeinating, Daydreaming, Procrastinating,
Wool-gathering, Ruminating, Noodling, Philosophizing, Spacecadeting, Cogitating,
Staring blankly, Consulting oracle, Asking the void, Manifesting, Yak-shaving,
Counting sheep, Muttering darkly, Buffering, Defragging, Dilly-dallying,
Meandering, Topsy-turvying, Snack-hunting, Going down rabbit,
Gesticulating, Shimmying, Zesting, Whatchamacalliting, Whirlpooling,
Bootstrapping, Ebbing, Cross-referencing, Double-checking, Almost there,
Pacing in circles, Reticulating splines, Ionizing, Weighing approaches

---

## Hardware

- **Arduino Uno** via USB-B cable
- **SainSmart LCD2004** (20×4) with I2C backpack
- Wiring: SDA→A4, SCL→A5, VCC→5V, GND→GND
- I2C address: `0x3F` (try `0x27` if blank)
- Blue pot on back of I2C board adjusts contrast

## Flash firmware

Arduino IDE → open `claude_monitor/claude_monitor.ino` → Board: Uno, Port: `/dev/cu.usbmodem*` → Upload.  
Press the **reset button** on the Uno right when IDE shows "Uploading..." if you get errors.

## Python dependency

```bash
pip3 install pyserial psutil
```
