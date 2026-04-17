#!/usr/bin/env python3
"""
Claude Code LCD Monitor — Background Daemon
Reads status from a FIFO pipe and sends to Arduino LCD.
Adds CPU/RAM sysinfo every few seconds.

USAGE:
  python3 monitor.py &
"""

import os
import sys
import signal
import select
import serial
import serial.tools.list_ports
import time

try:
    import psutil
    psutil.cpu_percent(interval=None)   # prime the measurement
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False
    print("[lcd] psutil not found — no CPU/RAM display. Run: pip3 install psutil")

FIFO_PATH        = "/tmp/claude_lcd"
ARDUINO_PORT     = None   # None = auto-detect
BAUD_RATE        = 9600
IDLE_TIMEOUT     = 10     # seconds → show idle screen
SYSINFO_INTERVAL = 3      # seconds → update CPU/RAM on row 3


def find_arduino():
    for p in serial.tools.list_ports.comports():
        desc = (p.description or "").lower()
        name = (p.device or "").lower()
        if any(k in desc or k in name for k in ("arduino", "usbmodem", "usbserial", "ch340", "ch341")):
            return p.device
    return None


def connect():
    while True:
        port = ARDUINO_PORT or find_arduino()
        if port:
            try:
                ser = serial.Serial(port, BAUD_RATE, timeout=1)
                time.sleep(2)   # Arduino resets on serial connect
                print(f"[lcd] Connected: {port}")
                return ser
            except Exception as e:
                print(f"[lcd] Connect failed: {e} — retrying in 3s")
        else:
            print("[lcd] No Arduino found — retrying in 3s")
        time.sleep(3)


def send(ser, msg):
    try:
        ser.write((msg + "\n").encode())
        ser.flush()
        print(f"[lcd] >> {msg}")
        return True
    except Exception as e:
        print(f"[lcd] Serial error: {e}")
        return False


def ensure_fifo():
    import stat
    if os.path.exists(FIFO_PATH):
        if not stat.S_ISFIFO(os.stat(FIFO_PATH).st_mode):
            os.remove(FIFO_PATH)
            os.mkfifo(FIFO_PATH)
    else:
        os.mkfifo(FIFO_PATH)


def get_sysinfo():
    if not HAS_PSUTIL:
        return None
    cpu = psutil.cpu_percent(interval=None)
    mem = psutil.virtual_memory().percent
    return f"CPU:{cpu:3.0f}% MEM:{mem:3.0f}%"


def main():
    ensure_fifo()
    ser = connect()
    send(ser, "STATUS:Monitor Ready")

    def cleanup(sig=None, frame=None):
        send(ser, "CLEAR")
        ser.close()
        try:
            os.remove(FIFO_PATH)
        except:
            pass
        sys.exit(0)

    signal.signal(signal.SIGINT, cleanup)
    signal.signal(signal.SIGTERM, cleanup)

    print(f"[lcd] Listening on {FIFO_PATH}")

    last_msg_time    = time.time()
    last_sysinfo_time = 0
    idle_sent        = False

    fd = os.open(FIFO_PATH, os.O_RDONLY | os.O_NONBLOCK)
    fifo = os.fdopen(fd, 'r')

    while True:
        readable, _, _ = select.select([fifo], [], [], 1.0)

        if readable:
            data = fifo.read()
            if data:
                for line in data.strip().split('\n'):
                    line = line.strip()
                    if line:
                        if not send(ser, line):
                            print("[lcd] Lost connection — reconnecting")
                            ser = connect()
                            send(ser, line)
                        last_msg_time = time.time()
                        idle_sent     = False
            else:
                # EOF — reopen FIFO
                fifo.close()
                fd = os.open(FIFO_PATH, os.O_RDONLY | os.O_NONBLOCK)
                fifo = os.fdopen(fd, 'r')

        now     = time.time()
        elapsed = now - last_msg_time

        # Idle after 10s — clear display, then show CPU/RAM
        if not idle_sent and elapsed > IDLE_TIMEOUT:
            send(ser, "CLEAR")
            idle_sent = True

        # Sysinfo every 3s
        if HAS_PSUTIL and now - last_sysinfo_time > SYSINFO_INTERVAL:
            last_sysinfo_time = now
            info = get_sysinfo()
            if info:
                if not send(ser, f"SYSINFO:{info}"):
                    print("[lcd] Lost connection — reconnecting")
                    ser = connect()
                    send(ser, f"SYSINFO:{info}")


if __name__ == "__main__":
    main()
