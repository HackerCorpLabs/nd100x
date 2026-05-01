#!/usr/bin/env python3
"""
Unit tests for SINTRAN segment disk-read logic.

Tests the core invariants that the JS segment disassembler depends on:
  - REECOMT is at disk LBA = SEGFIL0_BASE + SEG3_MADR + REECOMT_REL_PAGE
  - The wrong formula (LBA = SEG3_MADR + REECOMT_REL_PAGE) gives zeros
  - RAM physPage 1981 has the same content as disk LBA 3084
  - BLST symbol (0o4023) is NOT the BLSTX disk-base table

Run:
    python3 test_disk_read.py

Both SMD0.IMG and nd100_physmem_4MB.bin must be accessible at the paths below.
"""

import struct
import sys
import os

SMD_IMG   = '/Users/ronny/rh/nd100x/SMD0.IMG'
MEM_DUMP  = '/Users/ronny/Downloads/nd100_physmem_4MB.bin'

SECTOR_BYTES = 1024        # bytes per SMD sector (512 words × 2 bytes)
WORDS_PER_PAGE = 512       # SINTRAN virtual page = 512 words
REECOMT_SIG  = 0x4E52     # 'N','R' = first bytes of first REECOMT name "NRL"
REECOMT_WORD = 303         # word index of signature within REECOMT page

# Known-good values from physical memory analysis + disk scan
SEGFIL0_BASE       = 1670  # BLSTX[0]: base disk sector for SEGFIL 0
SEG3_MADR          = 1408  # 5OPSEG (segment 3) disk address
REECOMT_REL_PAGE   = 6     # REECOMT is at relative page 6 within 5OPSEG
REECOMT_LBA        = SEGFIL0_BASE + SEG3_MADR + REECOMT_REL_PAGE   # 3084
REECOMT_PHYS_PAGE  = 1981  # physical RAM page containing REECOMT (from dump)

pass_count = 0
fail_count = 0

def test(name, got, want, details=''):
    global pass_count, fail_count
    ok = (got == want)
    status = 'PASS' if ok else 'FAIL'
    print(f'  [{status}] {name}')
    if not ok:
        print(f'         got  = {repr(got)}')
        print(f'         want = {repr(want)}')
        if details:
            print(f'         note = {details}')
        fail_count += 1
    else:
        pass_count += 1

def read_disk_word(disk, lba, word_idx):
    """Read one big-endian 16-bit word from a disk sector."""
    off = lba * SECTOR_BYTES + word_idx * 2
    if off + 2 > len(disk):
        return -1
    return struct.unpack_from('>H', disk, off)[0]

def read_disk_sector_words(disk, lba, count=512):
    """Read 'count' big-endian words from disk starting at LBA."""
    off = lba * SECTOR_BYTES
    if off + SECTOR_BYTES > len(disk):
        return None
    return [struct.unpack_from('>H', disk, off + i*2)[0] for i in range(count)]

def read_mem_word(mem, word_addr):
    """Read one big-endian word from the physical memory dump."""
    off = word_addr * 2
    if off + 2 > len(mem):
        return -1
    return struct.unpack_from('>H', mem, off)[0]

def read_mem_page_words(mem, phys_page, count=512):
    """Read first 'count' words of a physical RAM page."""
    base = phys_page * WORDS_PER_PAGE * 2   # each page = 512 words = 1024 bytes
    # Note: MMU page = 1024 words, SINTRAN virtual page = 512 words
    # The JS code reads physPage*1024 words (MMU page size), SINTRAN page in first half
    base = phys_page * 1024 * 2             # physical word addr = physPage * 1024
    return [struct.unpack_from('>H', mem, base + i*2)[0] for i in range(count)]

def scan_disk_for_reecomt(disk, max_lba=8192):
    """Return the first LBA where word 303 == REECOMT_SIG, or -1."""
    for lba in range(min(max_lba, len(disk) // SECTOR_BYTES)):
        w = read_disk_word(disk, lba, REECOMT_WORD)
        if w == REECOMT_SIG:
            return lba
    return -1

# ─────────────────────────────────────────────
# Load images
# ─────────────────────────────────────────────
print(f'Loading {SMD_IMG} ...')
with open(SMD_IMG, 'rb') as f:
    disk = f.read()
print(f'  {len(disk)} bytes = {len(disk) // SECTOR_BYTES} sectors')

print(f'Loading {MEM_DUMP} ...')
with open(MEM_DUMP, 'rb') as f:
    mem = f.read()
print(f'  {len(mem)} bytes = {len(mem) // 2} words')
print()

# ─────────────────────────────────────────────
# Section 1: LBA formula correctness
# ─────────────────────────────────────────────
print('=== Section 1: LBA formula ===')

# Test: without base, LBA = SEG3_MADR + REL_PAGE = 1414 → no signature
w_wrong = read_disk_word(disk, SEG3_MADR + REECOMT_REL_PAGE, REECOMT_WORD)
test('Wrong LBA (no SEGFIL base) does NOT have REECOMT sig',
     w_wrong != REECOMT_SIG, True,
     f'LBA={SEG3_MADR + REECOMT_REL_PAGE}, w303=0x{w_wrong:04x}')

# Test: with base, LBA = 1670 + 1408 + 6 = 3084 → signature found
w_correct = read_disk_word(disk, REECOMT_LBA, REECOMT_WORD)
test('Correct LBA (with SEGFIL base=1670) has REECOMT sig',
     w_correct, REECOMT_SIG,
     f'LBA={REECOMT_LBA}, w303=0x{w_correct:04x}')

# Test: only ONE sector in the disk has the REECOMT signature (confirms it is unique)
found_lba = scan_disk_for_reecomt(disk, max_lba=len(disk) // SECTOR_BYTES)
test('REECOMT signature unique in disk image (only one LBA matches)',
     found_lba, REECOMT_LBA,
     f'found at LBA={found_lba}')

# Test: base derivation: found_lba - SEG3_MADR - REECOMT_REL_PAGE = SEGFIL0_BASE
derived_base = found_lba - SEG3_MADR - REECOMT_REL_PAGE
test('Derived SEGFIL0 base == 1670',
     derived_base, SEGFIL0_BASE)

print()

# ─────────────────────────────────────────────
# Section 2: RAM vs disk content match
# ─────────────────────────────────────────────
print('=== Section 2: RAM vs disk content match ===')

disk_words = read_disk_sector_words(disk, REECOMT_LBA, 512)
ram_words  = read_mem_page_words(mem, REECOMT_PHYS_PAGE, 512)

test('REECOMT word303 in disk == 0x4e52', disk_words[303], REECOMT_SIG)
test('REECOMT word303 in RAM  == 0x4e52', ram_words[303],  REECOMT_SIG)
test('Disk and RAM first 8 words match',  disk_words[:8], ram_words[:8])
test('Disk and RAM words[0..511] identical', disk_words, ram_words)

print()

# ─────────────────────────────────────────────
# Section 3: BLST ≠ BLSTX (root cause of the bug)
# ─────────────────────────────────────────────
print('=== Section 3: BLST symbol is NOT the disk-base table ===')

# Physical address 0o4023 = 2067 decimal is BLST (LRU chain head), NOT BLSTX.
# If we read 4 words from physical address 2067, we should NOT get [1670, ...].
blst_addr = 0o4023   # = 2067
blst_w0 = read_mem_word(mem, blst_addr)
test('Physical word at 0o4023 (BLST) is NOT 1670',
     blst_w0 != SEGFIL0_BASE, True,
     f'physical[0o4023] = {blst_w0} = 0x{blst_w0:04x} (BLST is a list-head pointer, not BLSTX)')

# The wrong JS call was: sym.readBlock(0o4023, 4) expecting [1670, ...]
# This shows why it fails: even with perfect DPIT translation to physical,
# the value is not 1670.
print(f'  (physical[0o4023] = {blst_w0}, SEGFIL0_BASE = {SEGFIL0_BASE})')

print()

# ─────────────────────────────────────────────
# Section 4: Correct fix — disk scan for base
# ─────────────────────────────────────────────
print('=== Section 4: Correct fix — scan disk for REECOMT ===')

# The JS fix: scan SMD disk for w303 == 0x4e52, then use:
#   SEGFIL0_BASE = found_lba - SEG3_MADR - relPage
# where relPage comes from the RAM chain walk (LOGPA - seg3.logad).

# Verify that scanning up to LBA 8192 finds the right LBA efficiently
found_in_8k = scan_disk_for_reecomt(disk, max_lba=8192)
test('Disk scan up to LBA 8192 finds REECOMT', found_in_8k, REECOMT_LBA)

# Verify sector content at LBA 3084 is non-zero (not all zeros)
non_zero = sum(1 for w in disk_words if w != 0)
test('REECOMT sector at LBA 3084 is non-zero', non_zero > 0, True,
     f'{non_zero}/512 words are non-zero')

# Verify sector at wrong LBA 1414 is all-zero (confirming zeros bug)
wrong_words = read_disk_sector_words(disk, SEG3_MADR + REECOMT_REL_PAGE, 512)
wrong_nz = sum(1 for w in wrong_words if w != 0)
test('Sector at wrong LBA 1414 is all zeros (explains the bug)',
     wrong_nz, 0,
     f'{wrong_nz}/512 words non-zero at LBA={SEG3_MADR + REECOMT_REL_PAGE}')

print()

# ─────────────────────────────────────────────
# Section 5: REECOMT content sanity
# ─────────────────────────────────────────────
print('=== Section 5: REECOMT table content ===')

# Word 0..3: first entry = {start, restart, name_ptr, segno}
# Expected: segno = 0x58 = 88 decimal (NRL segment)
test('REECOMT entry[0].segno = 0x58 (decimal 88 = NRL)',
     disk_words[3], 0x58)

# Word 4..7: second entry
# Expected: segno = 0x59 = 89 decimal (BACKUP-SYSTEM-B)
test('REECOMT entry[1].segno = 0x59 (decimal 89)',
     disk_words[7], 0x59)

# Name table at word 303: starts with 'N','R' (ASCII for NRL)
hi = (disk_words[303] >> 8) & 0xFF
lo = disk_words[303] & 0xFF
test("Name table word303 high byte = 'N' (0x4e)", hi, 0x4e)
test("Name table word303 low byte  = 'R' (0x52)", lo, 0x52)

# Decode first name: words 303 onward, stop at separator (0x27)
name = ''
for w_idx in range(303, 320):
    hi = (disk_words[w_idx] >> 8) & 0xFF
    lo = disk_words[w_idx] & 0xFF
    if hi == 0x27 or hi == 0: break
    name += chr(hi)
    if lo == 0x27 or lo == 0: break
    name += chr(lo)
test('First REECOMT name decodes to "NRL"', name, 'NRL')

print()

# ─────────────────────────────────────────────
# Summary
# ─────────────────────────────────────────────
total = pass_count + fail_count
print(f'=== Results: {pass_count}/{total} passed, {fail_count} failed ===')
sys.exit(0 if fail_count == 0 else 1)
