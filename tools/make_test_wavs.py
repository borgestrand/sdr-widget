#!/usr/bin/env python3
"""
20261104: test WAV files for the USB audio path. Standard library only, Python 3.

Prime files (bit-exact test, firmware built with -DFEATURE_PRIME_CHECK):
  Each channel counts up by a fixed prime per frame, modulo 2^bits, starting at
  one prime (so frame 0 is not silence). The firmware checks every step where
  USB samples are assembled in uac2_device_audio_task.c and prints '<' or '>'
  on a break. Keep PRIMES equal to PRIME_L_16 etc. in that file.
  WARNING: this is full-scale sawtooth-like noise. Don't listen to it, keep the
  amplifier off or the volume at zero. Play it bit-perfect: exclusive mode or
  ASIO, 100% digital volume, no resampling, no DSP.

Sine files (scope test):
  L = 0.4 FS DC + 0.1 FS 999 Hz sine
  R = -0.7 FS DC + 0.15 FS 1001 Hz sine
  40 s holds a whole number of cycles of both, so a gapless loop is seamless.
  No dither: the files are exactly reproducible.

Square files (scope test of the time domain):
  Both channels from 0 to 0.8 FS. High and low last a whole number of
  samples each: 34 at 44.1k, 37 at 48k, doubled at 88.2/96k and
  quadrupled at 176.4/192k (about 648.5 Hz at 44.1k family, 648.6 Hz at
  48k family). R lags L by 20 samples, scaled the same way. Every period is
  sample-for-sample identical, and the file holds whole periods only
  (close to 40 s), so a gapless loop (e.g. foobar2000 repeat track) never
  gives a short or long period.
  Edges are raised-cosine ramps of SQUARE_EDGE samples (scaled by rate) so
  the reconstruction filter doesn't ring (no Gibbs). Each edge is centred
  on the period boundary, so half level to half level is exactly the
  high or low count.

Pulse files (scope test of the I2S output, 32-bit only, 20261114):
  Every sample in L and R is one fixed word, chosen by rate so that SDATA
  pulses are about 0.33 us wide at all rates: 0x55555555 (1-bit pulses) at
  44.1/48k, 0x33333333 (2-bit) at 88.2/96k, 0x0F0F0F0F (4-bit) at
  176.4/192k. A scope trigger on SDATA low for more than ~0.5 us then
  catches any zero word. Play in the 32-bit ALT: at 16 or 24 bits the zero
  low bits would break the pattern. These are the words of the firmware's
  pulse mode ('O' mode 2, PRIME_MODE_PULSE). WARNING: a large DC level at
  the DAC output. Amplifier off.

Usage:
  python tools/make_test_wavs.py [output_dir]      (default: test_wavs)
"""

import math
import os
import struct
import sys

RATES = [44100, 48000, 88200, 96000, 176400, 192000]
BITS = [16, 24, 32]

# (L, R) per resolution. Keep equal to PRIME_*_16, PRIME_*_24 and PRIME_*_32 in src/uac2_device_audio_task.c
# 32 bit (20261114): the largest prime below 2^24 and the smallest above, the same pace as the 24-bit pair
PRIMES = {16: (251, 241), 24: (65521, 65537), 32: (16777213, 16777259)}

PRIME_SECONDS = 20
PULSE_SECONDS = 20
# Keep equal to the pulse words in src/uac2_device_audio_task.c (PRIME_MODE_PULSE)
PULSE_WORDS = {44100: 0x55555555, 48000: 0x55555555, 88200: 0x33333333, 96000: 0x33333333,
               176400: 0x0F0F0F0F, 192000: 0x0F0F0F0F}
SINE_SECONDS = 40
SINE_L = (0.4, 0.1, 999.0)      # DC, amplitude, frequency, all relative to full scale
SINE_R = (-0.7, 0.15, 1001.0)

SQUARE_SECONDS = 40
SQUARE_HIGH = 0.8               # Relative to full scale, low level is 0
SQUARE_HALF = {44100: 34, 48000: 37}   # Samples high and samples low at the base rates
SQUARE_OFFSET = 20              # Samples R lags L at the base rates
SQUARE_EDGE = 5                 # Edge length in samples at the base rates, about 0.1 ms


def is_prime(n):
    if n < 2:
        return False
    d = 2
    while d * d <= n:
        if n % d == 0:
            return False
        d += 1
    return True


def wav_header(rate, bits, frames):
    """RIFF header for 2-channel PCM. 24 and 32 bit use WAVE_FORMAT_EXTENSIBLE, as the spec asks for more than 16 bits."""
    channels = 2
    block = channels * bits // 8
    data_bytes = frames * block
    if bits == 16:
        fmt = struct.pack('<HHIIHH', 1, channels, rate, rate * block, block, bits)
    else:
        # cbSize 22, valid bits, channel mask FL|FR, KSDATAFORMAT_SUBTYPE_PCM
        subformat = struct.pack('<IHH8s', 0x00000001, 0x0000, 0x0010, bytes([0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71]))
        fmt = struct.pack('<HHIIHHHHI', 0xFFFE, channels, rate, rate * block, block, bits, 22, bits, 0x3) + subformat
    header = b'fmt ' + struct.pack('<I', len(fmt)) + fmt
    header += b'data' + struct.pack('<I', data_bytes)
    return b'RIFF' + struct.pack('<I', 4 + len(header) + data_bytes) + b'WAVE' + header


def sample_bytes(value, bits):
    """Two's complement bit pattern of value, little endian, bits/8 bytes."""
    return (value & ((1 << bits) - 1)).to_bytes(bits // 8, 'little')


def write_prime(path, rate, bits):
    p_l, p_r = PRIMES[bits]
    mask = (1 << bits) - 1
    nbytes = bits // 8
    frames = rate * PRIME_SECONDS
    data = bytearray()
    v_l, v_r = p_l, p_r                 # Frame 0 is one prime, not 0
    for _ in range(frames):
        data += v_l.to_bytes(nbytes, 'little')
        data += v_r.to_bytes(nbytes, 'little')
        v_l = (v_l + p_l) & mask
        v_r = (v_r + p_r) & mask
    with open(path, 'wb') as f:
        f.write(wav_header(rate, bits, frames))
        f.write(data)


def write_sine(path, rate, bits):
    full = (1 << (bits - 1)) - 1        # Full scale: 32767 or 8388607
    frames = rate * SINE_SECONDS
    data = bytearray()
    w_l = 2.0 * math.pi * SINE_L[2] / rate
    w_r = 2.0 * math.pi * SINE_R[2] / rate
    for n in range(frames):
        l = round(full * (SINE_L[0] + SINE_L[1] * math.sin(w_l * n)))
        r = round(full * (SINE_R[0] + SINE_R[1] * math.sin(w_r * n)))
        data += sample_bytes(l, bits)
        data += sample_bytes(r, bits)
    with open(path, 'wb') as f:
        f.write(wav_header(rate, bits, frames))
        f.write(data)


def square_level(k, half, edge):
    """Level 0..1 at sample k of a square wave, half samples high then half samples low, with raised-cosine
    edges of edge samples. The rising edge is centred on sample 0, the falling edge on sample half."""
    k %= 2 * half
    x = k if k < half else k - 2 * half                 # Distance from the rising edge
    if abs(x) < edge / 2.0:
        return 0.5 + 0.5 * math.sin(math.pi * x / edge)
    y = k - half                                        # Distance from the falling edge
    if abs(y) < edge / 2.0:
        return 0.5 - 0.5 * math.sin(math.pi * y / edge)
    return 1.0 if 0 < x < half else 0.0


def square_params(rate):
    """(half, offset, edge) in samples: the base values for 44.1k or 48k, times 1, 2 or 4."""
    base = 44100 if rate % 44100 == 0 else 48000
    mult = rate // base
    return SQUARE_HALF[base] * mult, SQUARE_OFFSET * mult, SQUARE_EDGE * mult


def write_square(path, rate, bits):
    full = (1 << (bits - 1)) - 1
    half, offset, edge = square_params(rate)
    period = 2 * half
    frames = round(SQUARE_SECONDS * rate / period) * period     # Whole periods only
    table = [round(full * SQUARE_HIGH * square_level(k, half, edge)) for k in range(period)]
    data = bytearray()
    for n in range(frames):
        data += sample_bytes(table[n % period], bits)
        data += sample_bytes(table[(n - offset) % period], bits)    # R lags L
    with open(path, 'wb') as f:
        f.write(wav_header(rate, bits, frames))
        f.write(data)


def write_pulse(path, rate):
    word = PULSE_WORDS[rate].to_bytes(4, 'little')
    frames = rate * PULSE_SECONDS
    with open(path, 'wb') as f:
        f.write(wav_header(rate, 32, frames))
        f.write(word * (2 * frames))


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'test_wavs'
    os.makedirs(out, exist_ok=True)

    for bits, (p_l, p_r) in PRIMES.items():
        assert is_prime(p_l) and is_prime(p_r) and p_l != p_r, 'PRIMES must be two different primes'

    # Both sines must complete whole cycles in the file for a seamless loop
    assert (SINE_L[2] * SINE_SECONDS).is_integer() and (SINE_R[2] * SINE_SECONDS).is_integer()
    # Every rate must be 1, 2 or 4 times a base rate of the square files
    assert all(r // (44100 if r % 44100 == 0 else 48000) in (1, 2, 4) for r in RATES)

    for bits in BITS:
        for rate in RATES:
            name = os.path.join(out, 'prime_%02dbit_%d.wav' % (bits, rate))
            print(name)
            write_prime(name, rate, bits)
            name = os.path.join(out, 'sine_dc_%02dbit_%d.wav' % (bits, rate))
            print(name)
            write_sine(name, rate, bits)
            name = os.path.join(out, 'square_%02dbit_%d.wav' % (bits, rate))
            print(name)
            write_square(name, rate, bits)

    for rate in RATES:
        name = os.path.join(out, 'pulse_32bit_%d.wav' % rate)
        print(name)
        write_pulse(name, rate)


if __name__ == '__main__':
    main()
