#!/usr/bin/env python3
"""
Activity Watcher — writes CPU/RAM to LCD FIFO on any user input.

USAGE:
  python3 activity_watcher.py &
"""

import os
import time
import threading
import psutil
import Quartz
import CoreFoundation

FIFO_PATH       = "/tmp/claude_lcd"
THROTTLE        = 2.0
ACTIVITY_WINDOW = 3.0

last_activity   = [0.0]


def on_event(proxy, event_type, event, refcon):
    last_activity[0] = time.time()
    return event


def write_sysinfo():
    try:
        cpu = psutil.cpu_percent(interval=None)
        mem = psutil.virtual_memory().percent
        line = f"SYSINFO:CPU:{cpu:3.0f}% MEM:{mem:3.0f}%\n"
        fd = os.open(FIFO_PATH, os.O_WRONLY | os.O_NONBLOCK)
        os.write(fd, line.encode())
        os.close(fd)
    except Exception as e:
        pass


def writer_loop():
    while True:
        time.sleep(THROTTLE)
        if time.time() - last_activity[0] < ACTIVITY_WINDOW:
            write_sysinfo()


def main():
    psutil.cpu_percent(interval=None)

    t = threading.Thread(target=writer_loop, daemon=True)
    t.start()

    mask = (
        Quartz.CGEventMaskBit(Quartz.kCGEventKeyDown) |
        Quartz.CGEventMaskBit(Quartz.kCGEventMouseMoved) |
        Quartz.CGEventMaskBit(Quartz.kCGEventLeftMouseDown) |
        Quartz.CGEventMaskBit(Quartz.kCGEventRightMouseDown) |
        Quartz.CGEventMaskBit(Quartz.kCGEventScrollWheel)
    )

    tap = Quartz.CGEventTapCreate(
        Quartz.kCGSessionEventTap,
        Quartz.kCGHeadInsertEventTap,
        Quartz.kCGEventTapOptionListenOnly,
        mask,
        on_event,
        None,
    )

    if not tap:
        print("[activity] CGEventTap failed — add Terminal to Accessibility in System Settings")
        return

    source = Quartz.CFMachPortCreateRunLoopSource(None, tap, 0)
    loop = CoreFoundation.CFRunLoopGetCurrent()
    CoreFoundation.CFRunLoopAddSource(loop, source, CoreFoundation.kCFRunLoopDefaultMode)
    Quartz.CGEventTapEnable(tap, True)
    print("[activity] Watching — move mouse to test")
    CoreFoundation.CFRunLoopRun()


if __name__ == "__main__":
    main()
