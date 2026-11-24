#!/usr/bin/env python3
"""
20261121: bit-exact USB playback test, analyzer side. Runs on the PC that holds the DAC's debug UART, whichever
computer plays (tools/play_test.py). Close Tera Term first: the COM port can only be open once.

It turns the firmware's prime check on (sends 'O' until the reply is 1), then follows the UART:
  [            USB takes the input: a session starts. Then B or C (USB plug), the rate digit and ,16 ,24 or ,32
  < >          prime breaks. A '>' followed by a newline is not a break but USB giving the input away
  ] q ) >\\n   USB gives the input away: the session ends
  + - . : * /  feedback reports, ignored
For each session it prints the format as the terminal reported it, the play time and the prime breaks. The player
streams primes with no loop point, so a session passes with no breaks at all. Breaks in the first --grace seconds
(stream start) are reported but don't fail the session.

Needs a firmware built with FEATURE_PRIME_CHECK:
  make clean audio-widget-84mhz OEM_MANUFACTURER="Manufacturer" OEM_PRODUCT="Product" EXPERIMENT=-DFEATURE_PRIME_CHECK

Usage:
  python tools/analyze_test.py --port COM3          (pip install pyserial)
Stop with Ctrl+C. A summary follows, and the raw UART text with time stamps is kept in a log file.
"""

import argparse
import re
import sys
import time

import serial

RATE_DIGITS = {'1': '44.1k', '2': '48k', '3': '88.2k', '4': '96k', '5': '176.4k', '6': '192k',
               '7': '352.8k', '8': '384k'}
END_CHARS = ']q)'


class Session:
    def __init__(self, t):
        self.start = t
        self.end = None
        self.text = ''              # Everything the UART printed during the session
        self.breaks = []            # (time, char)

    def format(self):
        plug = re.search(r'\[([BC])', self.text)
        m = re.search(r'([1-8]),(\d\d)', self.text)
        rate = RATE_DIGITS.get(m.group(1), '?') if m else '?'
        bits = m.group(2) if m else '??'
        return '%s %s/%s' % (plug.group(1) if plug else '?', rate, bits)

    def report(self, grace):
        end = self.end if self.end is not None else time.time()
        early = [b for b in self.breaks if b[0] - self.start < grace]
        late = [b for b in self.breaks if b[0] - self.start >= grace]
        known = re.search(r'[1-8],\d\d', self.text) is not None
        verdict = 'PASS' if (known and not late) else 'FAIL'
        note = ''
        if not known:
            note = ' (no rate/resolution report seen)'
        elif late:
            note = ' first at +%.1fs' % (late[0][0] - self.start)
        return '%s  %-14s %7.1fs  breaks %3d (start %d)  %s%s' % (
            time.strftime('%H:%M:%S', time.localtime(self.start)), self.format(), end - self.start,
            len(late), len(early), verdict, note)


def set_prime_mode(port, log):
    """Send 'O' until the firmware replies 1 (prime check on, pulse pattern off)"""
    for _ in range(5):
        port.write(b'O')
        time.sleep(0.5)
        text = port.read(port.in_waiting or 1).decode('latin-1')
        log.write(text)
        replies = re.findall(r'([0-3-])\n', text)
        if replies and replies[-1] == '-':
            sys.exit('The firmware has no prime check. Build it with EXPERIMENT=-DFEATURE_PRIME_CHECK')
        if replies and replies[-1] == '1':
            return
    sys.exit('No reply 1 to O. Is the UART connected, and is the baud rate right?')


def main():
    parser = argparse.ArgumentParser(description='Bit-exact USB playback test, analyzer side (UART).')
    parser.add_argument('--port', default='COM3', help='debug UART port (default COM3)')
    parser.add_argument('--baud', type=int, default=57600, help='default 57600, as in the firmware')
    parser.add_argument('--grace', type=float, default=2.0, help='seconds after [ where breaks are only reported')
    parser.add_argument('--log', default=time.strftime('prime_test_%Y%m%d_%H%M%S.log'))
    args = parser.parse_args()

    port = serial.Serial(args.port, args.baud, timeout=0.1)
    log = open(args.log, 'w', encoding='latin-1', newline='')
    set_prime_mode(port, log)
    print('Prime check on. Listening on %s, log in %s. Ctrl+C to stop.\n' % (args.port, args.log))

    sessions = []
    current = None
    pending_gt = None               # A '>' waits for the next character: newline means USB gives
    try:
        while True:
            data = port.read(256).decode('latin-1')
            now = time.time()
            if data:
                log.write('%.3f %r\n' % (now, data))
                log.flush()
            for c in data:
                if pending_gt is not None:
                    gt_time = pending_gt
                    pending_gt = None
                    if c == '\n':                       # USB gives: the session ends
                        if current is not None:
                            current.end = gt_time
                            print(current.report(args.grace))
                            current = None
                        continue
                    if current is not None:             # A real prime break
                        current.breaks.append((gt_time, '>'))
                if c == '[':
                    if current is not None:             # A new start without an end
                        current.end = now
                        print(current.report(args.grace))
                    current = Session(now)
                    sessions.append(current)
                if current is None:
                    continue
                current.text += c
                if c == '>':
                    pending_gt = now
                elif c == '<':
                    current.breaks.append((now, '<'))
                elif c in END_CHARS:
                    current.end = now
                    print(current.report(args.grace))
                    current = None
    except KeyboardInterrupt:
        pass
    finally:
        port.close()
        log.close()

    if current is not None and current.end is None:
        current.end = time.time()
        print(current.report(args.grace) + '  (still open)')
    print('\nSummary, %d sessions:' % len(sessions))
    for s in sessions:
        print('  ' + s.report(args.grace))


if __name__ == '__main__':
    main()
