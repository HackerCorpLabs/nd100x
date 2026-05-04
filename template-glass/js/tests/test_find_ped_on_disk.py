#!/usr/bin/env python3
"""
Decode PED-ENG-J.BPUN and search the disk image for its bytes.

BPUN format (from /Users/ronny/rh/nd100x/src/ndlib/load_bpun.c):
  1. Preamble (ASCII 7-bit):
       <start_addr_octal>/<boot_addr_octal>!
       Numbers are ASCII octal, separated by '/', terminated by '!'.
       (Multiple <number>/<number> pairs are possible — the LAST one before '!'
        gives the actual start/boot.)
  2. Address (2 bytes BE): destination memory word-address for data
  3. Count   (2 bytes BE): number of 16-bit words to follow
  4. Data    (count × 2 bytes BE): the actual words
  5. Checksum (2 bytes BE): sum of data words
  6. Action  (2 bytes BE): action code (JMP to start, or similar)

If address=count=checksum=0, FloMon format follows (different format).

This script:
  1. Parses PED-ENG-J.BPUN
  2. Prints the load address, count, first few words
  3. Searches SMD0.IMG for the exact byte pattern of PED's data
  4. Reports any matches with LBA + offset

Output is FACTUAL — only what we observe.
"""

import struct
import os
import sys

BPUN  = '/Users/ronny/rh/PED-ENG-J.BPUN'
DISK  = '/Users/ronny/rh/nd100x/SMD0.IMG'

if not os.path.exists(BPUN):
    print(f"FATAL: {BPUN} not found"); sys.exit(1)
if not os.path.exists(DISK):
    print(f"FATAL: {DISK} not found"); sys.exit(1)

with open(BPUN, 'rb') as f: bpun = f.read()
with open(DISK, 'rb') as f: disk = f.read()

print(f"BPUN file: {len(bpun):,} bytes")
print(f"Disk:      {len(disk):,} bytes ({len(disk)//1024:,} sectors)")
print()

# Phase 1: parse preamble
print("="*72)
print("PHASE 1: Parse BPUN preamble (ASCII)")
print("="*72)
print()

i = 0
preamble_text = ''
last_number = None
start_addr = None
boot_addr = None
current_num = ''

while i < len(bpun):
    b = bpun[i] & 0x7F
    c = chr(b) if 32 <= b < 127 or b in (0x0A, 0x0D, ord('/'), ord('!')) else None
    if b == ord('!'):
        preamble_text += '!'
        # Last number before '!' is the boot address (or load address)
        if current_num:
            try:
                last_number = int(current_num, 8)
                boot_addr = last_number
            except ValueError: pass
        i += 1
        break
    elif b == ord('/'):
        preamble_text += '/'
        if current_num:
            try:
                n = int(current_num, 8)
                if start_addr is None:
                    start_addr = n
            except ValueError: pass
            current_num = ''
    elif ord('0') <= b <= ord('9'):
        current_num += chr(b)
        preamble_text += chr(b)
    elif b == 0x0D or b == 0x0A:
        preamble_text += '\\n'
    else:
        preamble_text += '?'
    i += 1

print(f"Preamble bytes (first 100 chars): {preamble_text[:100]!r}")
print(f"  start_addr (octal): {oct(start_addr) if start_addr is not None else 'None'}  (decimal {start_addr})")
print(f"  boot_addr  (octal): {oct(boot_addr)  if boot_addr  is not None else 'None'}  (decimal {boot_addr})")
print()

# Phase 2: parse the binary record
print("="*72)
print("PHASE 2: Parse BPUN data record")
print("="*72)
print()

if i + 4 > len(bpun):
    print("Not enough bytes for address+count")
    sys.exit(1)

address = (bpun[i] << 8) | bpun[i+1]
count   = (bpun[i+2] << 8) | bpun[i+3]
i += 4
data_start = i
data_bytes = count * 2

print(f"Load address (word): {address} = 0o{address:o}")
print(f"Count (words):       {count} = 0o{count:o}")
print(f"Data bytes:          {data_bytes} bytes")
print(f"Last loaded word at: {address + count - 1} = 0o{address + count - 1:o}")
print()

if i + data_bytes + 4 > len(bpun):
    print("File too short for data + checksum + action")
    sys.exit(1)

data = bpun[i : i+data_bytes]
i += data_bytes

checksum_stored = (bpun[i] << 8) | bpun[i+1]
action          = (bpun[i+2] << 8) | bpun[i+3]
i += 4

# Compute checksum
checksum_calc = 0
for j in range(count):
    w = (data[j*2] << 8) | (data[j*2+1])
    checksum_calc = (checksum_calc + w) & 0xFFFF

print(f"Stored checksum:     0o{checksum_stored:o} (= {checksum_stored})")
print(f"Calculated checksum: 0o{checksum_calc:o} (= {checksum_calc})")
print(f"Match: {'YES ✓' if checksum_stored == checksum_calc else 'NO ✗'}")
print(f"Action: 0o{action:o}")
print()

# Show first 32 words loaded
print("First 32 words of loaded data:")
for j in range(min(32, count)):
    w = (data[j*2] << 8) | (data[j*2+1])
    addr = address + j
    print(f"  [{j:4d}] addr={oct(addr):>8s}  word=0o{w:06o} (0x{w:04x})")
print()

# Phase 3: search the disk for this exact byte pattern
print("="*72)
print("PHASE 3: Search disk for the exact loaded byte pattern")
print("="*72)
print()

# Try matching ALL data first
matches_full = []
i_search = 0
while True:
    idx = disk.find(data, i_search)
    if idx < 0: break
    matches_full.append(idx)
    i_search = idx + 1
    if len(matches_full) > 10: break
print(f"FULL data match (all {data_bytes} bytes): {len(matches_full)} location(s)")
for m in matches_full[:10]:
    lba = m // 1024
    off = m % 1024
    print(f"  byte {m:>10,} → LBA {lba:6d} byte_offset_in_sector={off}")
print()

# Now try matching just the first 64 words (128 bytes) — more likely to find partial copies
match_size = min(128, data_bytes)
target = data[:match_size]
matches_partial = []
i_search = 0
while True:
    idx = disk.find(target, i_search)
    if idx < 0: break
    matches_partial.append(idx)
    i_search = idx + 1
    if len(matches_partial) > 30: break
print(f"PARTIAL match (first {match_size} bytes = first {match_size//2} words): {len(matches_partial)} location(s)")
for m in matches_partial[:30]:
    lba = m // 1024
    off = m % 1024
    in_first_64w = (off == 0)
    flag = ' ← at sector boundary' if in_first_64w else ''
    print(f"  byte {m:>10,} → LBA {lba:6d} byte_offset_in_sector={off}{flag}")
print()

# Try matching at SECTOR BOUNDARIES only (where a segment image would actually start)
print("="*72)
print("PHASE 4: Match at sector boundaries (where a segment IMAGE would start)")
print("="*72)
print()
sector_starts = [m for m in matches_partial if m % 1024 == 0]
print(f"Matches at sector boundary: {len(sector_starts)}")
for m in sector_starts:
    lba = m // 1024
    print(f"  LBA {lba}")

print()
# If no sector-boundary match for the full data, try matching aligned to known SINTRAN
# layout: a segment IMAGE starts at sector boundary, page 0 of segment.
# Word `address` corresponds to virtual address. For a segment whose page 0 is at
# logical word `LOGAD * 1024`, the BPUN's `address` divided by 1024 gives the page index
# within the segment. The first sector of the IMAGE area is page 0 of the segment.
print("Inferring segment LOGAD from BPUN address:")
logad_word = address & ~0x3FF       # round down to page boundary
logad_page = address // 1024
print(f"  BPUN load addr 0o{address:o} = page {logad_page} (i.e. LOGAD = {logad_page})")
print(f"  word offset within page: {address & 0x3FF}")

# If the segment IMAGE on disk has the data starting at relPage 0, and BPUN starts
# at virtual address X, then the first sector of the segment may have data at a
# word offset matching (address mod 1024).
# So if BPUN address = 0o170000 = 61440, the load address is at word 0 of page 60.
# If a segment with LOGAD=60 has its IMAGE on disk, sector 0 of that IMAGE starts
# with the first 1024 words.
print()
print(f"If this BPUN is for a reentrant subsystem written via DUMP-REENTRANT,")
print(f"its segment in memory will have first valid word at addr 0o{address:o}.")
print(f"The bytes following are loaded contiguously.")

# Verify: if the segment is loaded at LOGAD = address // 1024, and the disk
# IMAGE area has the bytes contiguously starting at relPage 0, then we'd expect
# bytes to appear at sector boundaries on disk where sector 0 matches the first 1024
# words of the BPUN data (at offset (address & 0x3FF) within that sector).
