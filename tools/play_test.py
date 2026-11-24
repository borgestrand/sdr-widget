#!/usr/bin/env python3
"""
20261121: bit-exact USB playback test, player side. Runs on the Audio Host (Windows, Linux or macOS) and finds out
which one it is on. Plays a prime stream to the DAC in every format in turn: each channel counts up by a fixed prime
per frame, modulo 2^bits, as tools/make_test_wavs.py does, but generated on the fly with no loop point. So the
firmware's prime check (debug 'O' mode 1) must see no break at all. tools/analyze_test.py on the PC with the UART
reads the result. Between formats the player stays silent for a few seconds, so the firmware closes each session.

WARNING: full-scale sawtooth-like noise. Amplifier off, or volume at zero.

20261121: signal 'pulse' plays a fixed bit pattern instead, the top 16, 24 or 32 bits of one 32-bit word per rate,
so that SDATA pulses are about 0.33 us wide at all rates: 0x55555555 at 44.1/48k, 0x33333333 at 88.2/96k,
0x0F0F0F0F at 176.4/192k, 0x3FC03FC0 at 352.8/384k. A firmware built with FEATURE_PULSING_ZERO_PAD fills the low bits
the host doesn't send with the same pattern, so the I2S word is the whole pattern at any resolution. Judged with the
scope (SDATA pulse width outside the expected range), not by tools/analyze_test.py: the prime check reports breaks
all the time on a constant signal, so set 'O' to mode 0. Large DC level at the DAC output.

Usage:
  python tools/play_test.py DA256 20s
  python tools/play_test.py DA256 5m pulse
  python tools/play_test.py DA256 5m --rates 44100 192000 --bits 24 32 --pause 4

  device    part of the DAC's name as the OS shows it, case insensitive
  duration  play time in each configuration before moving to the next, e.g. 20s or 5m
  signal    prime (default) or pulse

Backends, all bit-perfect only as configured here:
  Windows  WASAPI exclusive mode through sounddevice (pip install sounddevice numpy)
  Linux    aplay to the ALSA hw: device, raw S16_LE / S24_3LE / S32_LE (apt install alsa-utils, pip install numpy).
           PipeWire or PulseAudio must not hold the device
  macOS    CoreAudio through sounddevice, which sets the device's rate. UNTESTED. CoreAudio passes samples as 32-bit
           float, so 32-bit primes are not expected to survive below bit 24. Set the bit depth in Audio MIDI Setup
"""

import argparse
import platform
import re
import subprocess
import sys
import time

import numpy as np

RATES = [44100, 48000, 88200, 96000, 176400, 192000]
BITS = [16, 24, 32]

# (L, R) per resolution. Keep equal to PRIMES in tools/make_test_wavs.py and PRIME_*_xx in src/uac2_device_audio_task.c
PRIMES = {16: (251, 241), 24: (65521, 65537), 32: (16777213, 16777259)}

# Pulse pattern per rate. Keep equal to the firmware: FEATURE_PULSING_ZERO_PAD and PRIME_MODE_PULSE in
# src/uac2_device_audio_task.c, and PULSE_WORDS in tools/make_test_wavs.py
PULSE_WORDS = {44100: 0x55555555, 48000: 0x55555555, 88200: 0x33333333, 96000: 0x33333333,
               176400: 0x0F0F0F0F, 192000: 0x0F0F0F0F, 352800: 0x3FC03FC0, 384000: 0x3FC03FC0}

BLOCK_SECONDS = 0.1


def parse_duration(text):
    m = re.fullmatch(r'(\d+(?:\.\d+)?)([sm])', text.strip().lower())
    if not m:
        raise argparse.ArgumentTypeError('duration must look like 20s or 5m')
    return float(m.group(1)) * (60 if m.group(2) == 'm' else 1)


def prime_blocks(rate, bits, seconds):
    """Yield the stream as little-endian interleaved bytes, BLOCK_SECONDS at a time. Frame n holds (n+1)*prime."""
    p_l, p_r = PRIMES[bits]
    mask = (1 << bits) - 1
    total = int(rate * seconds)
    block = int(rate * BLOCK_SECONDS)
    n = 0
    while n < total:
        count = min(block, total - n)
        frame = np.arange(n + 1, n + 1 + count, dtype=np.int64)
        data = np.empty(2 * count, dtype=np.int64)
        data[0::2] = (frame * p_l) & mask
        data[1::2] = (frame * p_r) & mask
        if bits == 16:
            yield data.astype('<u2').tobytes()
        elif bits == 32:
            yield data.astype('<u4').tobytes()
        else:   # 24 bit packed in 3 bytes, the format of the 24-bit ALT
            yield data.astype('<u4').view(np.uint8).reshape(-1, 4)[:, :3].tobytes()
        n += count


def pulse_blocks(rate, bits, seconds):
    """Yield the pulse pattern for rate, cut to its top bits, as little-endian interleaved bytes"""
    word = (PULSE_WORDS[rate] >> (32 - bits)).to_bytes(bits // 8, 'little')
    total = int(rate * seconds)
    block = int(rate * BLOCK_SECONDS)
    n = 0
    while n < total:
        count = min(block, total - n)
        yield word * (2 * count)
        n += count


def signal_blocks(signal, rate, bits, seconds):
    return (pulse_blocks if signal == 'pulse' else prime_blocks)(rate, bits, seconds)


# ---- Windows and macOS: sounddevice (PortAudio) ----

def sd_find_device(sd, needle, hostapi_name):
    hostapis = sd.query_hostapis()
    for index, dev in enumerate(sd.query_devices()):
        if (dev['max_output_channels'] >= 2 and needle.lower() in dev['name'].lower()
                and hostapis[dev['hostapi']]['name'] == hostapi_name):
            return index, dev['name']
    return None, None


def play_sounddevice(system, needle, signal, rate, bits, seconds):
    import sounddevice as sd
    if system == 'Windows':
        hostapi, extra = 'Windows WASAPI', sd.WasapiSettings(exclusive=True)
    else:
        hostapi, extra = 'Core Audio', sd.CoreAudioSettings(change_device_parameters=True)
    index, name = sd_find_device(sd, needle, hostapi)
    if index is None:
        raise RuntimeError('no %s output device with "%s" in its name' % (hostapi, needle))
    dtype = {16: 'int16', 24: 'int24', 32: 'int32'}[bits]
    with sd.RawOutputStream(samplerate=rate, channels=2, dtype=dtype, device=index, extra_settings=extra) as stream:
        for chunk in signal_blocks(signal, rate, bits, seconds):
            stream.write(chunk)
    return name


# ---- Linux: aplay to the ALSA hw: device ----

def alsa_find_device(needle):
    out = subprocess.run(['aplay', '-l'], capture_output=True, text=True).stdout
    for line in out.splitlines():
        m = re.match(r'card (\d+): .*?, device (\d+):', line)
        if m and needle.lower() in line.lower():
            return 'hw:%s,%s' % (m.group(1), m.group(2)), line.strip()
    return None, None


def play_alsa(needle, signal, rate, bits, seconds):
    device, name = alsa_find_device(needle)
    if device is None:
        raise RuntimeError('no ALSA card with "%s" in "aplay -l"' % needle)
    fmt = {16: 'S16_LE', 24: 'S24_3LE', 32: 'S32_LE'}[bits]
    proc = subprocess.Popen(['aplay', '-q', '-D', device, '-t', 'raw', '-f', fmt, '-r', str(rate), '-c', '2'],
                            stdin=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        for chunk in signal_blocks(signal, rate, bits, seconds):
            proc.stdin.write(chunk)
        proc.stdin.close()
    except BrokenPipeError:
        pass
    if proc.wait() != 0:
        raise RuntimeError('aplay failed: ' + proc.stderr.read().decode(errors='replace').strip())
    return '%s (%s)' % (device, name)


def main():
    parser = argparse.ArgumentParser(description='Bit-exact USB playback test, player side (prime or pulse streams).')
    parser.add_argument('device', help='part of the DAC name, e.g. DA256')
    parser.add_argument('duration', type=parse_duration,
                        help='play time in each configuration before moving to the next, e.g. 20s or 5m')
    parser.add_argument('signal', nargs='?', default='prime', choices=['prime', 'pulse'],
                        help='prime (default, judged by tools/analyze_test.py) or pulse (judged with the scope)')
    parser.add_argument('--rates', type=int, nargs='+', default=RATES)
    parser.add_argument('--bits', type=int, nargs='+', default=BITS, choices=BITS)
    parser.add_argument('--pause', type=float, default=4.0, help='silence between formats in seconds (default 4)')
    args = parser.parse_args()

    system = platform.system()
    print('Audio Host: %s %s, Python %s' % (system, platform.release(), platform.python_version()))
    if system == 'Darwin':
        print('macOS backend is untested. 32-bit primes are not expected to be exact below bit 24')
    elif system not in ('Windows', 'Linux'):
        sys.exit('Unsupported OS: ' + system)
    if args.signal == 'pulse':
        print('Signal: pulse pattern. Set debug O to mode 0, judge with the scope. Large DC level: amplifier off.\n')
    else:
        print('Signal: primes. WARNING: full-scale noise. Amplifier off.\n')

    results = []
    for bits in args.bits:
        for rate in args.rates:
            start = time.strftime('%H:%M:%S')
            print('%s  %6d Hz %2d bit  %gs ... ' % (start, rate, bits, args.duration), end='', flush=True)
            try:
                if system == 'Linux':
                    name = play_alsa(args.device, args.signal, rate, bits, args.duration)
                else:
                    name = play_sounddevice(system, args.device, args.signal, rate, bits, args.duration)
                print('played on ' + name)
                results.append((start, rate, bits, 'played'))
            except Exception as e:      # A refused format must not stop the run
                print('FAILED: %s' % e)
                results.append((start, rate, bits, 'FAILED: %s' % e))
            time.sleep(args.pause)

    print('\nPlayed, in this order (compare with tools/analyze_test.py on the UART PC):')
    for start, rate, bits, result in results:
        print('  %s  %6d Hz %2d bit  %s' % (start, rate, bits, result))


if __name__ == '__main__':
    main()
