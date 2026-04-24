# Claude Code LCD Monitor

Physical 20×4 LCD that shows what Claude Code is doing in real time — tool status, animated mascot, CPU/RAM usage.

![Hardware: Arduino Uno + SainSmart LCD2004]

## What it shows

**While idle:**
```
    CLAUDE CODE
      MONITOR
    [mascot walks ←→, freezes, runs off screen]
CPU: 4%  MEM: 61%
```

**While Claude works:**
```
[mascot] CLAUDE CODE
       Reading...
   monitor.py
CPU: 4%  MEM: 61%
```

Status words cycle through: Reading, Writing, Running, Searching, Googling, Collaborating, Plotting... and 40+ fun filler words when thinking.

---

## Hardware

| Part | Notes |
|------|-------|
| Arduino Uno | Any clone works |
| SainSmart LCD2004 | 20×4 with I2C backpack |
| 4× female-to-female jumper wires | |
| USB-B cable | Standard printer cable |

## Wiring

| LCD Pin | Arduino |
|---------|---------|
| GND | GND |
| VCC | 5V |
| SDA | A4 |
| SCL | A5 |

---

## Setup

### 1. Install Arduino library

Arduino IDE → Tools → Manage Libraries → search **"LiquidCrystal I2C"** by Frank de Brabander → Install

### 2. Flash the firmware

**This Uno does not respond to the standard reset-button upload trick.**  
Use the power-cycle method instead:

```bash
# Kill monitor if running
pkill -f monitor.py

# Compile
arduino-cli compile --fqbn arduino:avr:uno --output-dir /tmp/claude_build ~/Documents/Display/claude_monitor/

# Unplug USB cable, then run this command:
avrdude -q -q -p atmega328p -c arduino -P /dev/cu.usbmodem143201 -b 115200 -D \
  -U flash:w:/tmp/claude_build/claude_monitor.ino.hex:i

# Plug USB back in IMMEDIATELY after hitting Enter (within ~1 second)
```

The Uno enters bootloader mode for ~1 second on power-up. That is the upload window.

> **Port may differ.** Check yours with: `ls /dev/cu.usbmodem*`

> **Blank screen?** Turn the blue potentiometer on the I2C backpack until text appears.  
> **Still nothing?** Change `0x3F` to `0x27` in line 22 of `claude_monitor.ino`.

### 3. Install Python dependencies

```bash
pip3 install pyserial psutil
```

### 4. Start the monitor daemon

```bash
python3 ~/Documents/Display/monitor.py &
```

Auto-detects Arduino port. Sends CPU/RAM to LCD every few seconds. Runs in background.

### 5. Wire up Claude Code hooks

In `~/.claude/settings.json` add:

```json
{
  "hooks": {
    "UserPromptSubmit": [{"command": "bash ~/Documents/Display/hook_submit.sh"}],
    "PreToolUse":       [{"command": "bash ~/Documents/Display/hook.sh"}],
    "Stop":             [{"command": "bash ~/Documents/Display/hook_stop.sh"}]
  }
}
```

---

## Files

| File | Purpose |
|------|---------|
| `claude_monitor/claude_monitor.ino` | Arduino firmware v2.0 — upload once |
| `monitor.py` | Daemon — holds serial open, reads FIFO, sends to Arduino |
| `hook.sh` | PreToolUse — maps tool name → status word |
| `hook_submit.sh` | UserPromptSubmit — starts word cycler, timestamps session |
| `hook_stop.sh` | Stop — kills cycler, shows "Churned Xm Xs" |
| `cycler.sh` | Background loop — sends fun words to LCD while Claude thinks |
| `lcd_animator.html` | Browser tool — draw keyframes, export to Claude for animation |
| `lcd_designer.html` | Browser tool — design individual custom chars |
| `i2c_scanner.ino` | Utility — finds your LCD's I2C address |

## Serial protocol

| Command | Effect |
|---------|--------|
| `STATUS:Reading` | Row 1: centered with animated dots |
| `DETAIL:file.py` | Row 2: filename or detail |
| `SYSINFO:CPU: 4% MEM: 61%` | Row 3: always visible |
| `CLEAR` | Return to idle screen |

Arduino I2C: `0x3F` · Baud: `9600`

## Tool → display mapping

| Tool | LCD Shows |
|------|-----------|
| Read | Reading |
| Edit / Write | Writing |
| Bash | Running |
| Grep | Searching |
| WebSearch | Googling |
| WebFetch | Fetching |
| Agent | Collaborating |
| TaskCreate / TaskUpdate | Plotting |
| ToolSearch | Rummaging |
| Skill | Leveling Up |
| Thinking / unknown | Random fun word |

## Stop

```bash
pkill -f monitor.py
```
