#!/usr/bin/env python3
"""
Determine CBLST (start of SEGFIL 0) WITHOUT assumptions.

Background (from NPL source, NOT assumed):

  PH-P2-OPPSTART.NPL line 044745:
      SEGFSTART=:CBLST                          % START OF SEGFILE 0

  PH-P2-OPPSTART.NPL line 044757-044764 (cold-start, when HENTFLAG=0):
      X:=CBLST; T:="MSCOM"+1+X; A:="MICOM"+X
      X:="LOPCO"-1; CALL FAR CRWDISC            % COPY COMMAND SEGMENT
      X:=CBLST; T:="MSECO"+X;   A:="MIECO"+X
      X:="LSECO"; CALL FAR CRWDISC              % COPY EXT. COMMON
      ...

  This means: at cold start, boot code copies SAVE → IMAGE for several segments.
  Each segment has a fixed (MS_X, MI_X, LS_X / LOP_X) triple of constants.

Symbol values (from SYMBOLS/L07/SYMBOL-2-LIST.SYMB.TXT, all octal):
    MSCOM = 706 = 454    MICOM = 2600 = 1408   LOPCO = 65 = 53
    MSDPT = 100 = 64     MIDPT = ??            LSDPT = 55 = 45
    MSSGT = 501 = 321    MISGT = ??            LSSGT = 20 = 16
    MSECO = 161 = 113    MIECO = ??            LSECO = ?
    MSSYS = 155 = 109    MISYS = 2050 = 1064   LSSYS = 3
    MSPDF = 160 = 112    MIPDF = ??            LSPDF = ?

Approach:
  Test multiple candidate CBLST values. For each:
   (A) Check that disk content at LBA = CBLST + offset has expected sectors of data
       for each known SAVE/IMAGE pair.
   (B) Verify the boot copy invariant: SAVE+1..SAVE+N should be similar to IMAGE..IMAGE+N-1
       (only if cold-started, not HENT-restarted).

  If multiple independent pairs converge on the same CBLST → high confidence.
  Otherwise → report "cannot disambiguate without runtime measurement".
"""

import struct
import os
import sys

SMD = '/Users/ronny/rh/nd100x/SMD0.IMG'
SECTOR = 1024  # bytes per disk sector. Verified by REECOMT match earlier.

if not os.path.exists(SMD):
    print(f"FATAL: {SMD} not found"); sys.exit(1)
with open(SMD, 'rb') as f:
    disk = f.read()

print(f"Disk: {len(disk):,} bytes = {len(disk)//SECTOR} sectors")
print()

def sector_words(lba):
    if (lba+1)*SECTOR > len(disk): return None
    return [struct.unpack_from('>H', disk, lba*SECTOR + i*2)[0] for i in range(512)]

def nz(lba):
    s = sector_words(lba)
    return sum(1 for w in s if w) if s else -1

def compare_runs(lba_a, lba_b, n_sectors):
    """Compare n_sectors sectors at lba_a vs lba_b. Returns (matching_words, total_words)."""
    matches, total = 0, 0
    for i in range(n_sectors):
        a = sector_words(lba_a + i)
        b = sector_words(lba_b + i)
        if a is None or b is None: return matches, total
        for j in range(512):
            total += 1
            if a[j] == b[j]: matches += 1
    return matches, total

# Verified empirically earlier: REECOMT signature at LBA 3084 (only one on disk)
REECOMT_LBA = 3084
sig_words = sector_words(REECOMT_LBA)
assert sig_words[303] == 0x4E52, "REECOMT signature missing at LBA 3084 — disk changed?"
print(f"VERIFIED: REECOMT signature (0x4E52) at LBA {REECOMT_LBA}")
print()

# From NPL source (boot code, line 044757+), known SAVE→IMAGE pairs.
# Constants from SYMBOLS/L07/SYMBOL-2-LIST.SYMB.TXT:
PAIRS = [
    # (name, save_off, img_off, length, source)
    ('COMMAND',   454, 1408, 53, 'NPL line 044757: MSCOM=706, MICOM=2600, LOPCO=65'),
    ('SYS',       109, 1064,  3, 'MSSYS=155, MISYS=2050, LSSYS=3'),
]

print("="*72)
print("TEST: For each candidate CBLST, check known SAVE→IMAGE pairs")
print("="*72)
print()

for cblst in [1664, 1670, 1024, 2048]:
    print(f"--- Candidate CBLST = {cblst} ---")
    pair_results = []
    for name, sav, img, length, src in PAIRS:
        sav_lba = cblst + sav
        img_lba = cblst + img
        sav_total = sum(nz(sav_lba + i) for i in range(length) if nz(sav_lba+i) >= 0)
        img_total = sum(nz(img_lba + i) for i in range(length) if nz(img_lba+i) >= 0)

        # Boot invariant: SAVE+1..N matches IMAGE..N-1 if cold start was performed
        m, t = compare_runs(sav_lba + 1, img_lba, length - 1)
        pct = (100.0 * m / t) if t else 0
        pair_results.append((name, sav_lba, img_lba, sav_total, img_total, pct))
        print(f"  {name:8s}: SAV LBA {sav_lba:5d} ({sav_total:5d} nz)  "
              f"IMG LBA {img_lba:5d} ({img_total:5d} nz)  "
              f"SAV+1↔IMG: {pct:5.1f}% match  [{src}]")

    # Score this candidate: high score if all pairs have non-zero data and high match
    has_data = sum(1 for _, _, _, sav_t, img_t, _ in pair_results if sav_t > 0 and img_t > 0)
    avg_match = sum(p[5] for p in pair_results) / len(pair_results) if pair_results else 0
    print(f"  → pairs with data on both sides: {has_data}/{len(pair_results)}, avg match: {avg_match:.1f}%")
    print()

# Independent check: 5OPSEG IMAGE area should start at LBA = CBLST + MICOM = CBLST + 1408
# We see real data at LBA 3078 (511 nz). Check LBAs around it for boundaries.
print("="*72)
print("BOUNDARY CHECK: where does the 5OPSEG IMAGE region actually start/end?")
print("="*72)
print()
print("5OPSEG SECOLO=52 (verified via @LIST-SEGMENT 3 / segment table read).")
print("So the IMAGE region for 5OPSEG should be 52 contiguous sectors.")
print()
print("Checking LBA 3060–3140 for the region boundary:")
boundary_run = []
for lba in range(3060, 3140):
    n = nz(lba)
    boundary_run.append((lba, n))

# Find the start of the run (first non-zero) and end (last non-zero before a gap)
in_run = False
runs = []
run_start = None
prev_nz = False
for lba, n in boundary_run:
    if n > 0 and not prev_nz:
        run_start = lba
    elif n == 0 and prev_nz:
        runs.append((run_start, lba - 1))
    prev_nz = (n > 0)
if prev_nz:
    runs.append((run_start, boundary_run[-1][0]))

for s, e in runs:
    print(f"  Non-zero run: LBA {s} – {e} ({e-s+1} sectors)")

# If a run of exactly 52 sectors is found starting at LBA L, then CBLST = L - 1408.
print()
print("Looking for a 52-sector run starting somewhere in this range:")
for s, e in runs:
    if e - s + 1 >= 50:   # 52 ± boundary fuzz
        candidate_cblst = s - 1408
        print(f"  Run {s}–{e} ({e-s+1} sectors): if this is 5OPSEG IMAGE, CBLST = {candidate_cblst}")

# Triangulate via REECOMT: REECOMT_LBA = CBLST + MADR(5OPSEG=1408) + relPage_within_5OPSEG
print()
print("="*72)
print("TRIANGULATION: REECOMT_LBA - 5OPSEG.MADR = 3084 - 1408 = 1676")
print("="*72)
print()
print("So CBLST + REECOMT_relPage_within_5OPSEG = 1676.")
print("If REECOMT_relPage = 6: CBLST = 1670 (H1)")
print("If REECOMT_relPage = 12: CBLST = 1664 (H2)")
print()
print("Need INDEPENDENT measurement of REECOMT's relPage within 5OPSEG.")
print("This requires running the emulator and walking the BPAGL chain in CURRENT state.")
print("Cannot determine from disk image alone.")
print()

# Final summary
print("="*72)
print("CONCLUSION")
print("="*72)
print()
print("From disk image alone, cannot uniquely determine CBLST.")
print("Need either:")
print("  (a) Runtime BPAGL chain walk to identify REECOMT's relPage")
print("  (b) Another known SAVE/IMAGE pair to cross-validate")
print("  (c) NPL source reading the exact computation of SEGFSTART at boot")
print()
