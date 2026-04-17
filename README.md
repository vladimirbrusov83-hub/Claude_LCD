# Claude Code LCD Monitor

Physical 20x4 LCD that shows what Claude Code is doing in real time — Reading, Writing, Running, Searching, etc.

## Hardware
- Arduino Uno
- 20x4 LCD with I2C backpack
- 4 female-to-female jumper wires
- USB-B cable (printer cable)

## Wiring
| LCD Pin | Arduino Pin |
|---------|-------------|
| GND     | GND         |
| VCC     | 5V          |
| SDA     | A4          |
| SCL     | A5          |

## Step 1: Upload Arduino Code
1. Open Arduino IDE
2. Tools → Manage Libraries → search "LiquidCrystal I2C" by Frank de Brabander → Install
3. Open `claude_monitor.ino`
4. Tools → Board → Arduino Uno
5. Tools → Port → your Arduino port
6. Upload

If screen stays blank — turn the blue potentiometer on the back of the I2C board slowly until text appears.
If nothing shows at all, change `0x27` to `0x3F` in line 19 of the .ino file.

## Step 2: Install Python dependency
```
pip3 install pyserial
```

## Step 3: Start the monitor daemon
```
python3 ~/Documents/Display/monitor.py &
```
It auto-detects the Arduino port. Runs in background.

## Step 4: Add Claude Code hooks
Run this once — it tells Claude Code to send tool status to the LCD:
```
claude config set hooks.PreToolUse '[{"matcher": "", "command": "bash ~/Documents/Display/hook.sh"}]'
claude config set hooks.PostToolUse '[{"matcher": "", "command": "bash ~/Documents/Display/hook_done.sh"}]'
```

## That's it
Now every time Claude Code reads a file, runs a command, searches, writes, etc. — the LCD shows it in real time.

## Files
- `claude_monitor.ino` — Arduino firmware (upload once)
- `monitor.py` — Background daemon (reads FIFO, writes to Arduino serial)
- `hook.sh` — PreToolUse hook (detects what Claude is doing)
- `hook_done.sh` — PostToolUse hook (shows completion)

## Stopping
```
pkill -f monitor.py
```
