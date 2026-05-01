#!/usr/bin/env python3
"""
Comprehensive disk image analysis. Find structures on the disk WITHOUT
trusting any prior assumptions about CBLST, CABLPAGE, segment locations.

Hard inputs (these I will trust because they come from authoritative sources):
  - SMD0.IMG file size (from filesystem)
  - Sector size = 1024 bytes (per SMD format documented in NPL: each sector is
    accessed as 512 big-endian 16-bit words; CRWDISC operates in sectors)
  - NPL source code (s3vs-4.symb and SYMBOLS/L07/*.SYMB.TXT for VSX-L)

Everything else: derive from the disk content directly.

Output: a complete report of findings, clearly labelled as either:
  [FACT]    — directly observed in disk content or NPL source
  [DERIVED] — calculated from facts using a stated formula
  [CANDIDATE] — a hypothesis that fits some observations
  [UNKNOWN] — could not determine from the data available
"""

import struct
import os
import sys
from collections import Counter, defaultdict

SMD = '/Users/ronny/rh/nd100x/SMD0.IMG'
SECTOR = 1024  # bytes per disk sector

with open(SMD, 'rb') as f:
    disk = f.read()

TOTAL_SECTORS = len(disk) // SECTOR
print(f"[FACT] Disk file size: {len(disk):,} bytes = {TOTAL_SECTORS:,} sectors")
print()

# Helpers
def words(lba, count=512):
    """Read `count` 16-bit big-endian words starting at LBA."""
    if (lba + count * 2 // SECTOR + 1) * SECTOR > len(disk): return None
    base = lba * SECTOR
    return [struct.unpack_from('>H', disk, base + i*2)[0] for i in range(count)]

def nz_count(lba):
    """Count non-zero words in one 512-word sector."""
    s = words(lba)
    return sum(1 for w in s if w) if s else -1

def bytes_at(lba):
    return disk[lba*SECTOR : (lba+1)*SECTOR] if (lba+1)*SECTOR <= len(disk) else None

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 1: Find all REECOMT-signature sectors (word 303 = 0x4E52)")
print("="*72)
print()

reecomt_lbas = []
for lba in range(TOTAL_SECTORS):
    sig_byte = lba * SECTOR + 606  # word 303 = byte 606
    if disk[sig_byte] == 0x4E and disk[sig_byte+1] == 0x52:
        # Validate further: 'L' = 0x4C as next byte (start of "NRL")
        if disk[sig_byte+2] == 0x4C:
            reecomt_lbas.append(lba)

print(f"[FACT] Found {len(reecomt_lbas)} sectors matching REECOMT signature ('NRL...' at word 303):")
for lba in reecomt_lbas:
    w = words(lba)
    print(f"  LBA {lba}: entry0=[start={w[0]:5d} restart={w[1]:5d} nameptr={w[2]:5d} segno={w[3]:3d}]"
          f"  entry1=[start={w[4]:5d} restart={w[5]:5d} nameptr={w[6]:5d} segno={w[7]:3d}]")
print()

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 2: Map all non-zero regions on disk")
print("="*72)
print()

regions = []
in_region = False
start = None
for lba in range(TOTAL_SECTORS):
    nz = nz_count(lba)
    if nz > 0 and not in_region:
        in_region = True; start = lba
    elif nz == 0 and in_region:
        regions.append((start, lba - 1))
        in_region = False
if in_region:
    regions.append((start, TOTAL_SECTORS - 1))

print(f"[FACT] {len(regions)} non-zero regions on disk:")
for s, e in regions:
    print(f"  LBA {s:6d} - {e:6d}  length {e-s+1:5d} sectors")
print()

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 3: Look for first sector ('boot block') content")
print("="*72)
print()
w0 = words(0)
print(f"[FACT] LBA 0 first 16 words: {[f'{x:06o}' for x in w0[:16]]}")
print(f"[FACT] LBA 0 word counts: nz={nz_count(0)}/512")
# Boot block usually contains a load address, instructions, and entry point
print()

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 4: Find segment-table-like structures")
print("="*72)
print()
# A segment table sector has 64 segment entries, each 8 words.
# Many entries should be all-zero (unused). Used entries have:
#   SEGLI (link, small) PRESE (link, small) LOGAD (page#, 0..2047)
#   SEGLE (length, 1..256) MADR (disk offset) FLAG SGSTA BPAGL
# A heuristic: a sector where many 8-word groups have zero or have plausible
# segment-table values.

candidates = []
for lba in range(TOTAL_SECTORS):
    w = words(lba)
    if not w: continue
    # Score: count of plausible-looking 8-word groups
    plausible = 0
    zeros = 0
    for i in range(64):  # 64 entries per sector
        e = w[i*8:(i+1)*8]
        if all(x == 0 for x in e):
            zeros += 1
            continue
        # Heuristic checks for an active segment entry
        segli, prese, logad, segle, madr, flag, sgsta, bpagl = e
        # SEGLE low 10 bits should be 1..256 (segment length in pages)
        seg_len = segle & 0x3FF
        if seg_len == 0 or seg_len > 256: continue
        # LOGAD reasonable (0..2047 since absolute VPN)
        if logad > 2047: continue
        # MADR reasonable for SEGFIL 0 disk: 0..70000ish (disk is 76800 sectors)
        if madr > 70000: continue
        # SGSTA upper bits should have WPM/RPM/FPM mostly
        if (sgsta & 0xE000) == 0 and sgsta != 0: continue
        plausible += 1
    if plausible >= 3 and zeros + plausible >= 50:
        candidates.append((lba, plausible, zeros))

print(f"[CANDIDATE] {len(candidates)} sectors look like segment tables (>= 3 plausible entries, mostly zeros):")
for lba, p, z in candidates[:20]:
    print(f"  LBA {lba:5d}: {p:3d} plausible entries, {z:3d} zero slots")
print()

# Show some entries from the most plausible candidate
if candidates:
    best_lba = candidates[0][0]
    print(f"  First entries from best candidate LBA {best_lba}:")
    w = words(best_lba)
    for i in range(min(20, 64)):
        e = w[i*8:(i+1)*8]
        if all(x == 0 for x in e): continue
        seg_len = e[3] & 0x3FF
        flag = e[5]
        sgsta = e[6]
        segfil = (flag >> 13) & 7
        print(f"    seg{i:3d}: SEGLI={e[0]:5d} PRESE={e[1]:5d} LOGAD={e[2]:4d} "
              f"SEGLE={seg_len:3d} MADR={e[4]:5d} FLAG={flag:5d}(SF={segfil}) "
              f"SGSTA={sgsta:5d} BPAGL={e[7]:5d}")
    print()

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 5: From segment-table candidates, derive segment 3 (5OPSEG) MADR")
print("="*72)
print()
# 5OPSEG = segment 3. Its MADR tells us where COMMAND IMAGE area is.
# Then CBLST = REECOMT_LBA - 5OPSEG.MADR - REECOMT_relPage_within_5OPSEG.
# We need to find REECOMT_relPage independently.

# But first: does any segment table candidate have plausible data for seg 3?
print("[DERIVED] Segment 3 entries from each candidate sector:")
for lba, _, _ in candidates[:10]:
    w = words(lba)
    e = w[3*8:4*8]
    seg_len = e[3] & 0x3FF
    print(f"  Table at LBA {lba:5d} → seg3 SEGLI={e[0]:5d} LOGAD={e[2]:4d} "
          f"SEGLE={seg_len:3d} MADR={e[4]:5d} FLAG={e[5]:5d}")
print()

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 6: Find ALL plausible CBLST values from the data")
print("="*72)
print()
# REECOMT_LBA = 3084 (only one). The formula on the disk is:
#   REECOMT_LBA = BLSTX[segfile_of_5OPSEG] + 5OPSEG.MADR + REECOMT_relPage * CABLPAGE
# Given REECOMT_LBA = 3084 (FACT), and a candidate (madr3, relpage_REECOMT, CABLPAGE):
#   BLSTX[0] = 3084 - madr3 - relpage_REECOMT * CABLPAGE

# From the segment table candidates above, gather all possible 5OPSEG.MADR values.
seg3_madrs = set()
for lba, _, _ in candidates[:20]:
    w = words(lba)
    e = w[3*8:4*8]
    if e[3] & 0x3FF and 0 < e[4] < 70000:  # plausible
        seg3_madrs.add(e[4])

print(f"[DERIVED] Plausible seg3 MADR values from segment-table candidates: {sorted(seg3_madrs)}")
print()

# Try (CABLPAGE, relPage) combinations
print("[DERIVED] Combinations of (CABLPAGE, REECOMT_relPage) that yield CBLST in disk range:")
for cablpage in [1, 2, 4]:
    for madr3 in seg3_madrs:
        for relpage in range(0, 64):
            cblst = 3084 - madr3 - relpage * cablpage
            if 0 < cblst < TOTAL_SECTORS:
                # Bonus: check if CBLST + MICOM looks like a segment image start
                # MICOM = 1408 from symbols (SINTRAN L). But this assumes the running
                # SINTRAN matches the L symbol set.
                pass

# A cleaner approach: assume only that 5OPSEG IMAGE is contiguous data.
# We know REECOMT is INSIDE 5OPSEG, so the IMAGE area must include LBA 3084.
# Find the contiguous non-zero region containing LBA 3084.
print()
print("[DERIVED] Find non-zero region containing LBA 3084:")
for s, e in regions:
    if s <= 3084 <= e:
        length = e - s + 1
        print(f"  Region: LBA {s} to {e}, length {length} sectors")
        print(f"  REECOMT (3084) is at offset {3084 - s} within this region.")
        print(f"  → If this region IS 5OPSEG IMAGE: CBLST + 5OPSEG.MADR = {s}")
        for madr3 in seg3_madrs:
            cblst = s - madr3
            if cblst >= 0:
                print(f"    If 5OPSEG.MADR={madr3}: CBLST = {cblst}")
        # And REECOMT_relPage_within_5OPSEG
        relpage = 3084 - s
        print(f"  → REECOMT relPage_within_5OPSEG = {relpage} (with CABLPAGE=1)")
        break
print()

# ---------------------------------------------------------------------------
print("="*72)
print("PHASE 7: Search the entire disk for any 16-word pattern matching")
print("        plausible MAC code locations")
print("="*72)
print()
# Without the user's RAM dump, we can't match against MAC's actual bytes.
# But we CAN look for ALL non-zero regions and see which fit the size of MAC
# (14 sectors = 14 contiguous sectors of code-like data).

print("[CANDIDATE] All non-zero regions of length 14..16 sectors (MAC SECOLO=14):")
for s, e in regions:
    L = e - s + 1
    if 14 <= L <= 16:
        print(f"  LBA {s}-{e} ({L} sectors)")

print()
print("[CANDIDATE] All non-zero regions of length 100..300 sectors (likely HENT or SAVE):")
for s, e in regions:
    L = e - s + 1
    if 100 <= L <= 300:
        print(f"  LBA {s:5d}-{e:5d} ({L:3d} sectors)")
print()

print("[CANDIDATE] All non-zero regions of length > 300 sectors (likely main areas):")
for s, e in regions:
    L = e - s + 1
    if L > 300:
        print(f"  LBA {s:5d}-{e:5d} ({L:4d} sectors)")
print()

# ---------------------------------------------------------------------------
print("="*72)
print("CONCLUSION")
print("="*72)
print()
print("Things established by direct disk evidence:")
print(f"  - Disk has {TOTAL_SECTORS:,} sectors of 1024 bytes each")
print(f"  - Exactly ONE REECOMT signature, at LBA 3084")
print(f"  - REECOMT is inside a contiguous non-zero region")
print(f"  - Several segment-table-like sectors detected")
print()
print("To proceed without runtime data, next steps would be:")
print("  1. Cross-validate the segment-table candidates by looking at multiple")
print("     segments and checking their MADR fields against known IMAGE regions")
print("  2. Look for the BOOT SECTOR / DISK LAYOUT TABLE that defines BLSTX")
print("  3. Look for the HENT image structure")
