#!/usr/bin/env python3
import argparse
import serial
import sys
import time

MARKER1 = b"END MAIN"
MARKER2 =b"CPU halted"

def reset_chip(ser):
    ser.dtr = False
    ser.rts = True
    time.sleep(0.1)
    ser.rts = False
    time.sleep(0.1)


def main():
    parser = argparse.ArgumentParser(description="Wait for marker, reset, repeat.")
    parser.add_argument("-p", "--port", required=True, 
                        help="Serial port, e.g. /dev/ttyUSB0")
    parser.add_argument("-b", "--baud", type=int, default=115200, 
                        help="Baud rate (default: 115200)")
    parser.add_argument("-l", "--log", default="full.log", 
                        help="Log file path for full logs(default: test.log)")
    parser.add_argument("--warn", default="warn.log", 
                        help="Log file path for warning and error msg(default: test.log)")
    args = parser.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=1)
    reset_chip(ser)

    with open(args.log, "wb") as l, \
            open(args.warn,"wb") as w:
        while True:
            line = ser.readline()
            if not line:
                continue
            sys.stdout.buffer.write(line)
            sys.stdout.buffer.flush()
            l.write(line)
            l.flush()

            if not line.startswith(b"I"):
                w.write(line)
                w.flush()
            
            if MARKER1 in line:
                break
            if MARKER2 in line:
                break


if __name__ == "__main__":
    main()
