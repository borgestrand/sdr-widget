#!/usr/bin/env python3
"""
20261104: test WAV files for the USB audio path. Python 3 with numpy (pip install numpy).
20261122: all rates up to 384k, 16/24/32 bit, pulse files at all resolutions, numpy for speed. 10 s files, written
as FLAC by default at 16 and 24 bit: each WAV is encoded with ffmpeg (on the PATH, or --ffmpeg), decoded again and
compared sample for sample, then deleted. 32-bit files stay WAV unless --flac32: 32-bit FLAC needs ffmpeg's
experimental encoder and a player with 32-bit FLAC support (libFLAC 1.4 or later), and libsndfile 1.2 refuses it.
--wav keeps every file as WAV. Prime files hardly compress, pulse files shrink to almost nothing.

Play every file bit-perfect: exclusive mode or ASIO, 100% digital volume, no resampling, no DSP.

Prime files (bit-exact test, firmware built with -DFEATURE_PRIME_CHECK):
  Each channel counts up by a fixed prime per frame, modulo 2^bits, starting at
  one prime (so frame 0 is not silence). The firmware checks every step where
  USB samples are assembled in uac2_device_audio_task.c and prints '<' or '>'
  on a break. A loop of the file gives one break. Keep PRIMES equal to
  PRIME_L_16 etc. in that file, and in tools/play_test.py.
  WARNING: this is full-scale sawtooth-like noise. Don't listen to it, keep the
  amplifier off or the volume at zero.

Pulse files (scope test of the whole chain to the I2S output):
  Every sample in L and R is one fixed pattern, chosen by rate so that SDATA
  pulses are about 0.33 us wide at all rates: 0x55555555 (1-bit pulses) at
  44.1/48k, 0x33333333 (2-bit) at 88.2/96k, 0x0F0F0F0F (4-bit) at 176.4/192k,
  0x3FC03FC0 (8-bit, off the byte boundaries) at 352.8/384k. 16- and 24-bit
  files hold the top 16 or 24 bits. A firmware built with
  FEATURE_PULSING_ZERO_PAD fills the missing low bits with the same pattern, so
  the I2S word is the whole pattern at any resolution. Trigger the scope on an
  SDATA pulse width outside the expected range. Keep the words equal to
  PULSE_WORDS in tools/play_test.py. WARNING: a large DC level at the DAC
  output. Amplifier off.

Square files (scope test of the time domain):
  Both channels from 0 to 0.8 FS. High and low last a whole number of
  samples each: 34 at 44.1k, 37 at 48k, times 2, 4 or 8 at the higher rates
  (about 648.5 Hz in the 44.1k family, 648.6 Hz in the 48k family). R lags L
  by 20 samples, scaled the same way. Every period is sample-for-sample
  identical, and the file holds whole periods only (close to 10 s), so a
  gapless loop (e.g. foobar2000 repeat track) never gives a short or long
  period. Edges are raised-cosine ramps of SQUARE_EDGE samples (scaled by
  rate) so the reconstruction filter doesn't ring (no Gibbs). Each edge is
  centred on the period boundary, so half level to half level is exactly the
  high or low count.

Sine files (scope test, only with --sine):
  L = 0.4 FS DC + 0.1 FS 999 Hz sine
  R = -0.7 FS DC + 0.15 FS 1001 Hz sine
  The file holds a whole number of cycles of both, so a gapless loop is seamless.
  No dither: the files are exactly reproducible.

Usage:
  python tools/make_test_wavs.py [output_dir] [--sine] [--wav] [--ffmpeg PATH]      (default: test_wavs)
"""

import argparse
import math
import os
import shutil
import struct
import subprocess

import numpy as np

RATES = [44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000]
BITS = [16, 24, 32]

# (L, R) per resolution. Keep equal to PRIME_*_16, PRIME_*_24 and PRIME_*_32 in src/uac2_device_audio_task.c
# 32 bit (20261114): the largest prime below 2^24 and the smallest above, the same pace as the 24-bit pair
PRIMES = {16: (251, 241), 24: (65521, 65537), 32: (16777213, 16777259)}

PRIME_SECONDS = 10
PULSE_SECONDS = 10
# Keep equal to PULSE_WORDS in tools/play_test.py and the words of FEATURE_PULSING_ZERO_PAD and PRIME_MODE_PULSE in
# src/uac2_device_audio_task.c
PULSE_WORDS = {44100: 0x55555555, 48000: 0x55555555, 88200: 0x33333333, 96000: 0x33333333,
               176400: 0x0F0F0F0F, 192000: 0x0F0F0F0F, 352800: 0x3FC03FC0, 384000: 0x3FC03FC0}
SINE_SECONDS = 10
SINE_L = (0.4, 0.1, 999.0)      # DC, amplitude, frequency, all relative to full scale
SINE_R = (-0.7, 0.15, 1001.0)

SQUARE_SECONDS = 10
SQUARE_HIGH = 0.8               # Relative to full scale, low level is 0
SQUARE_HALF = {44100: 34, 48000: 37}   # Samples high and samples low at the base rates
SQUARE_OFFSET = 20              # Samples R lags L at the base rates
SQUARE_EDGE = 5                 # Edge length in samples at the base rates, about 0.1 ms

CHUNK_FRAMES = 1 << 20          # Frames generated and written at a time


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


def pack(left, right, bits):
    """Interleave two int64 arrays and return little-endian two's complement bytes, bits/8 bytes per sample"""
    data = np.empty(2 * len(left), dtype=np.int64)
    data[0::2] = left
    data[1::2] = right
    data &= (1 << bits) - 1
    if bits == 16:
        return data.astype('<u2').tobytes()
    if bits == 32:
        return data.astype('<u4').tobytes()
    return data.astype('<u4').view(np.uint8).reshape(-1, 4)[:, :3].tobytes()   # 24 bit in 3 bytes


def write_wav(path, rate, bits, frames, generate):
    """generate(n) gets the frame numbers of one chunk (int64 array) and returns (left, right) as int64 arrays"""
    with open(path, 'wb') as f:
        f.write(wav_header(rate, bits, frames))
        for start in range(0, frames, CHUNK_FRAMES):
            n = np.arange(start, min(start + CHUNK_FRAMES, frames), dtype=np.int64)
            f.write(pack(*generate(n), bits))


def write_prime(path, rate, bits):
    p_l, p_r = PRIMES[bits]
    write_wav(path, rate, bits, rate * PRIME_SECONDS, lambda n: ((n + 1) * p_l, (n + 1) * p_r))  # Frame 0 is one prime


def write_pulse(path, rate, bits):
    word = PULSE_WORDS[rate] >> (32 - bits)         # The top bits of the pattern
    write_wav(path, rate, bits, rate * PULSE_SECONDS,
              lambda n: (np.full(len(n), word, dtype=np.int64), np.full(len(n), word, dtype=np.int64)))


def write_sine(path, rate, bits):
    full = (1 << (bits - 1)) - 1
    w_l = 2.0 * math.pi * SINE_L[2] / rate
    w_r = 2.0 * math.pi * SINE_R[2] / rate
    write_wav(path, rate, bits, rate * SINE_SECONDS,
              lambda n: (np.round(full * (SINE_L[0] + SINE_L[1] * np.sin(w_l * n))).astype(np.int64),
                         np.round(full * (SINE_R[0] + SINE_R[1] * np.sin(w_r * n))).astype(np.int64)))


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
    """(half, offset, edge) in samples: the base values for 44.1k or 48k, times 1, 2, 4 or 8."""
    base = 44100 if rate % 44100 == 0 else 48000
    mult = rate // base
    return SQUARE_HALF[base] * mult, SQUARE_OFFSET * mult, SQUARE_EDGE * mult


def write_square(path, rate, bits):
    full = (1 << (bits - 1)) - 1
    half, offset, edge = square_params(rate)
    period = 2 * half
    frames = round(SQUARE_SECONDS * rate / period) * period     # Whole periods only
    table = np.array([round(full * SQUARE_HIGH * square_level(k, half, edge)) for k in range(period)], dtype=np.int64)
    write_wav(path, rate, bits, frames, lambda n: (table[n % period], table[(n - offset) % period]))   # R lags L


def to_flac(ffmpeg, wav, bits):
    """Encode wav to FLAC next to it, decode the FLAC and compare with the WAV's samples. Delete the WAV if equal"""
    flac = wav[:-4] + '.flac'
    pcm = {16: 's16le', 24: 's24le', 32: 's32le'}[bits]
    quiet = [ffmpeg, '-hide_banner', '-loglevel', 'error', '-y']
    # Experimental: 32-bit FLAC. 4096-frame blocks: ffmpeg's own FLAC reader found no frames in a 32-bit 352.8k file
    # written with its default (larger) block size
    subprocess.run(quiet + ['-i', wav, '-c:a', 'flac', '-strict', 'experimental',
                            '-sample_fmt', 's16' if bits == 16 else 's32', '-bits_per_raw_sample', str(bits),
                            '-compression_level', '8', '-frame_size', '4096', flac], check=True)
    ref = subprocess.run(quiet + ['-i', wav, '-f', pcm, '-'], check=True, capture_output=True).stdout
    dec = subprocess.run(quiet + ['-i', flac, '-f', pcm, '-'], check=True, capture_output=True).stdout
    if ref != dec:
        raise RuntimeError('%s does not decode to the samples of %s' % (flac, wav))
    os.remove(wav)
    return flac


def main():
    parser = argparse.ArgumentParser(description='Test WAV files for the USB audio path.')
    parser.add_argument('out', nargs='?', default='test_wavs', help='output directory (default test_wavs)')
    parser.add_argument('--sine', action='store_true', help='also write the sine files')
    parser.add_argument('--wav', action='store_true', help='keep WAV files, no FLAC')
    parser.add_argument('--flac32', action='store_true',
                        help='FLAC for 32 bit too. Off by default: few players read 32-bit FLAC (libsndfile 1.2 '
                             'refuses it), and 32-bit primes don\'t compress anyway')
    parser.add_argument('--ffmpeg', default=shutil.which('ffmpeg'), help='path to ffmpeg (default: from the PATH)')
    args = parser.parse_args()
    if not args.wav and not args.ffmpeg:
        parser.error('ffmpeg not found for FLAC. Give --ffmpeg PATH, or use --wav')
    os.makedirs(args.out, exist_ok=True)

    for bits, (p_l, p_r) in PRIMES.items():
        assert is_prime(p_l) and is_prime(p_r) and p_l != p_r, 'PRIMES must be two different primes'
    # Both sines must complete whole cycles in the file for a seamless loop
    assert (SINE_L[2] * SINE_SECONDS).is_integer() and (SINE_R[2] * SINE_SECONDS).is_integer()
    # Every rate must be 1, 2, 4 or 8 times a base rate of the square files
    assert all(r // (44100 if r % 44100 == 0 else 48000) in (1, 2, 4, 8) for r in RATES)

    kinds = [('prime', write_prime), ('pulse', write_pulse), ('square', write_square)]
    if args.sine:
        kinds.append(('sine_dc', write_sine))
    for kind, write in kinds:
        for bits in BITS:
            for rate in RATES:
                name = os.path.join(args.out, '%s_%02dbit_%d.wav' % (kind, bits, rate))
                write(name, rate, bits)
                if (not args.wav) and (bits != 32 or args.flac32):
                    name = to_flac(args.ffmpeg, name, bits)
                print(name)


if __name__ == '__main__':
    main()
