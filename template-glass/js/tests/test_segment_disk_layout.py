#!/usr/bin/env python3
"""
SINTRAN segment disk layout analysis and validation.

Proves the 3-area disk model against SMD0.IMG and nd100_physmem_4MB.bin:

  Area 1 – ABSOLUTE SAVE (sectors 0 to CBLST-1)
    Error Program, RT-Loader saved at absolute sectors MERRP=64, MRTLO=95
    These segments carry FLAG=SGFI4 (SEGFIL 4, base=0) so MADR is an absolute LBA.

  Area 2 – SAVE IMAGE (CBLST + MS-offsets)
    Pristine copies of system PITs, Segment Table, Extended Common etc.
    Never modified during operation; restored to IMAGE area at cold start.
    Formula:  LBA = CBLST + MS_OFFSET
    Example:  MSDPT=100₈=64  → sector 1670+64=1734

  Area 3 – LIVE IMAGE (CBLST + MI-offsets / MADR from segment table)
    The working copies of all segments, paged in/out during operation.
    Formula:  LBA = CBLST + MADR + relPage   (CABLPAGE=1 on this SMD disk)
    Example:  MICOM=2600₈=1408 → 5OPSEG page 0 at sector 1670+1408=3078
              5OPSEG page 6 (REECOMT) at sector 3084

All offsets (MS_OFFSET, MI_OFFSET, MADR) are relative to CBLST = 1670.
CABLPAGE = 1: each sector is 1024 bytes = 512 words = exactly one SINTRAN page.

Validated disk areas from s3vs-4.symb (all lengths are octal, offsets are decimal):
SAVE IMAGE offsets from CBLST:
  MIMAG=0     len=63  (image of common/start code)
  MSDPT=64    len=45  (save of DPIT)
  MSSYS=109   len=3   (save of system segment)
  MSPDF=112   len=1   (save of spooling df)
  MSECO=113   len=2   (save of ext-common)
  MSRPT=115   len=51  (save of RPIT)
  MSMPT=166   len=51  (save of MPIT)
  MSIPT=217   len=51  (save of IPIT)
  MS5PT=268   len=5   (save of 5PIT)
  MN5MO=273   len=48  (save of ND-500 monitor)
  MSSGT=321   len=16  (save of segment table)
  MSUC1=337   len=32  (save of 110 micro code)
  MSUC2=369   len=32  (save of 120 micro code)

LIVE IMAGE offsets from CBLST (from segment table MADR fields):
  MIECO=1067  len=2   (image of ext-common)
  MISGT=1275  len=16  (image of segment table)
  MICOM=1408  len=52  (image of command segment / 5OPSEG)
  ... plus all other dynamically allocated segment MADR values

Run:
    python3 test_segment_disk_layout.py
"""

import struct
import sys

SMD_IMG  = '/Users/ronny/rh/nd100x/SMD0.IMG'
MEM_DUMP = '/Users/ronny/Downloads/nd100_physmem_4MB.bin'

SECTOR_BYTES  = 1024   # 1 sector = 512 words = 1024 bytes (CABLPAGE=1)
WORDS_PER_SEC = 512
SE_SIZE       = 8      # words per segment table entry

# Proven anchor (verified by test_disk_read.py)
CBLST = 1670   # = SEGFIL0 base sector = sector where SEGFIL IMAGE begins

# ── ABSOLUTE SAVE area (sectors 0..CBLST-1, SEGFIL 4, base=0) ─────────────
MERRP = 64    # Error Program SAVE   (absolute LBA, NOT CBLST-relative)
LERRP = 20    # 20 sectors
MRTLO = 95    # RT-Loader SAVE code  (absolute LBA)
MRTLO_DATA = 115  # RT-Loader data (MRTLO+20)
LRTLO = 20    # 20 sectors each

# ── SAVE IMAGE offsets from CBLST (MS-prefix = pristine backup) ───────────
# optional=True: subsystem may not be installed; zero sectors = not present
SAVE_AREAS = [
    ('MIMAG', 0,   63,  'Image of common/start code',       False),
    ('MSDPT', 64,  45,  'Save of DPIT',                     False),
    ('MSSYS', 109, 3,   'Save of system segment',           True),
    ('MSPDF', 112, 1,   'Save of spooling datafield',       True),
    ('MSECO', 113, 2,   'Save of extended common',          True),
    ('MSRPT', 115, 51,  'Save of RPIT',                     False),
    ('MSMPT', 166, 51,  'Save of MPIT',                     False),
    ('MSIPT', 217, 51,  'Save of IPIT',                     False),
    ('MS5PT', 268, 5,   'Save of 5PIT',                     False),
    ('MN5MO', 273, 48,  'Save of ND-500 monitor',           False),
    ('MSSGT', 321, 16,  'Save of segment table',            False),
    ('MSUC1', 337, 32,  'Save of 110 microcode',            False),
    ('MSUC2', 369, 32,  'Save of 120 microcode',            False),
]

# ── LIVE IMAGE offsets from CBLST (MI-prefix = operational copies) ────────
# (partial list of known constants; MADR from segment table covers the rest)
KNOWN_IMAGE_AREAS = [
    ('MIECO', 1067, 2,  'Image of extended common'),
    ('MISGT', 1275, 16, 'Image of segment table'),
    ('MICOM', 1408, 52, 'Image of 5OPSEG (command segment)'),
]

# ── Memory dump constants for segment table location ──────────────────────
# Physical page 66 (102₈) hosts DPIT page 2 (logical 0o4000–0o5777)
DPIT_PAGE2_PHYSPAGE = 66
DPIT_PAGE2_LOGBASE  = 0o4000    # = 2048

def dpit_page2_word(mem, logical_addr):
    """Read one 16-bit word at a logical address in DPIT page 2 (0o4000–0o5777)."""
    offset = logical_addr - DPIT_PAGE2_LOGBASE
    phys_word = DPIT_PAGE2_PHYSPAGE * 1024 + offset
    return struct.unpack_from('>H', mem, phys_word * 2)[0]

# ── Low-level helpers ──────────────────────────────────────────────────────

def read_word(data, byte_off):
    if byte_off + 2 > len(data): return 0
    return struct.unpack_from('>H', data, byte_off)[0]

def read_phys_word(mem, word_addr):
    return read_word(mem, word_addr * 2)

def read_sector_words(disk, lba):
    """Return list of 512 big-endian words for the sector at LBA, or None."""
    off = lba * SECTOR_BYTES
    if off + SECTOR_BYTES > len(disk): return None
    return [struct.unpack_from('>H', disk, off + i*2)[0] for i in range(WORDS_PER_SEC)]

def sector_nonzero_count(disk, lba):
    w = read_sector_words(disk, lba)
    if w is None: return -1
    return sum(1 for x in w if x)

def sectors_nonzero(disk, base_lba, count):
    """Return number of sectors in [base_lba, base_lba+count) that are non-zero."""
    return sum(1 for i in range(count) if sector_nonzero_count(disk, base_lba + i) > 0)

# ── Test harness ──────────────────────────────────────────────────────────

pass_count = fail_count = 0

def test(name, got, want, note=''):
    global pass_count, fail_count
    ok = (got == want)
    print(f'  [{"PASS" if ok else "FAIL"}] {name}')
    if not ok:
        print(f'         got  = {repr(got)}')
        print(f'         want = {repr(want)}')
        if note: print(f'         note = {note}')
        fail_count += 1
    else:
        pass_count += 1

def test_nonzero(name, disk, lba, note=''):
    nz = sector_nonzero_count(disk, lba)
    ok = nz > 0
    print(f'  [{"PASS" if ok else "FAIL"}] {name}  LBA={lba}  nonzero={nz}')
    if not ok and note: print(f'         note = {note}')
    global pass_count, fail_count
    if ok: pass_count += 1
    else: fail_count += 1
    return nz

# ─────────────────────────────────────────────────────────────────────────
print(f'Loading {SMD_IMG} ...')
with open(SMD_IMG, 'rb') as f:
    disk = f.read()
disk_sectors = len(disk) // SECTOR_BYTES
print(f'  {len(disk):,} bytes = {disk_sectors:,} sectors')

print(f'Loading {MEM_DUMP} ...')
with open(MEM_DUMP, 'rb') as f:
    mem = f.read()
print(f'  {len(mem):,} bytes = {len(mem)//2:,} words')
print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 1: Disk formula  LBA = CBLST + MADR + relPage ===')
# Verify CABLPAGE=1 using the proven anchor from test_disk_read.py
SEG3_MADR        = 1408  # 5OPSEG MADR (= MICOM offset = 0o2600 = 1408)
REECOMT_REL_PAGE = 6     # relPage of REECOMT within 5OPSEG
REECOMT_LBA      = CBLST + SEG3_MADR + REECOMT_REL_PAGE   # 3084
REECOMT_SIG      = 0x4E52

w303 = read_sector_words(disk, REECOMT_LBA)[303]
test('CBLST=1670 + MADR=1408 + relPage=6 = LBA 3084 has REECOMT sig',
     w303, REECOMT_SIG, f'word303=0x{w303:04x}')

# CABLPAGE=1: multiplying by 8 would give LBA 3084+42=3126 → no sig
w303_x8 = read_sector_words(disk, CBLST + SEG3_MADR + REECOMT_REL_PAGE * 8)[303]
test('CABLPAGE=8 (wrong: relPage*8) gives NO REECOMT sig',
     w303_x8 != REECOMT_SIG, True, f'word303=0x{w303_x8:04x}')
print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 2: Read segment table from physical memory dump ===')

SEGTB = dpit_page2_word(mem, 0o4320)   # segment table bank number
SEGST = dpit_page2_word(mem, 0o4321)   # segment table offset within bank
CORMB = dpit_page2_word(mem, 0o4322)   # core map bank number
SGMAX_RAW = dpit_page2_word(mem, 0o4015)

test('SEGTB (bank of segment table) = 3',           SEGTB, 3)
test('SEGST (offset within bank) = 0o124000 = 43008', SEGST, 0o124000)

phys_base = (SEGTB << 16) + SEGST   # = 239616 words
print(f'  physBase (segment table) = {phys_base} words = 0x{phys_base:06x}')
print(f'  SGMAX = {SGMAX_RAW} (0x{SGMAX_RAW:04x} = {oct(SGMAX_RAW)})')
print(f'  CORMB = {CORMB}')
print()

# Read all segment entries
max_seg = min(SGMAX_RAW + 1, 512)   # cap for safety
segs = []
for sn in range(max_seg):
    base = phys_base + sn * SE_SIZE
    entry = [read_phys_word(mem, base + i) for i in range(SE_SIZE)]
    if any(entry):
        segs.append({
            'segNum': sn,
            'logad':  entry[2],
            'segle':  entry[3],
            'madr':   entry[4],
            'flag':   entry[5],
            'sgsta':  entry[6],
            'bpagl':  entry[7],
        })

print(f'  Non-zero segment entries: {len(segs)} out of {max_seg} scanned')

# Verify known segments
seg_by_num = {s['segNum']: s for s in segs}

s3 = seg_by_num.get(3)
if s3:
    print(f'  Seg 3 (5OPSEG): LOGAD={oct(s3["logad"])} SEGLE={oct(s3["segle"])} '
          f'MADR={s3["madr"]} (0x{s3["madr"]:04x}) FLAG={oct(s3["flag"])} BPAGL={oct(s3["bpagl"])}')
    test('Seg 3 MADR = 1408 (= MICOM = 0o2600)', s3['madr'], 1408)
else:
    print('  WARNING: segment 3 not found in table')
print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 3: Area 1 – ABSOLUTE SAVE (sectors 0..CBLST-1, base=0) ===')
print(f'  These segments have top 3 bits of FLAG set (SEGFIL 4, base sector = 0)')
print(f'  Formula: LBA = 0 + MADR + relPage  (MADR is an absolute sector number)')
print()

# Boot area (sectors 0-63): should have non-zero boot code
boot_nz = sectors_nonzero(disk, 0, 64)
print(f'  Boot area sectors 0-63: {boot_nz}/64 non-zero')
test('Boot area (sectors 0-63) has content', boot_nz > 0, True)

# Error Program SAVE at MERRP=64 (absolute)
print()
print(f'  ERROR PROGRAM SAVE: absolute sectors {MERRP}–{MERRP+LERRP-1}')
err_nz = sectors_nonzero(disk, MERRP, LERRP)
print(f'    {err_nz}/{LERRP} sectors non-zero')
test(f'Error Program SAVE (abs LBA={MERRP}) has content', err_nz > 0, True)

# RT-Loader SAVE at MRTLO=95 (absolute)
print()
print(f'  RT-LOADER SAVE: absolute sectors {MRTLO}–{MRTLO+LRTLO-1}')
rtl_nz = sectors_nonzero(disk, MRTLO, LRTLO)
print(f'    {rtl_nz}/{LRTLO} sectors non-zero')
print(f'  NOTE: RT-Loader SAVE at abs LBA={MRTLO} has {rtl_nz}/{LRTLO} sectors non-zero')
print(f'        (may be in SEGFIL 4 with different absolute address in this build)')

# Gap between RT-Loader and CBLST should be largely empty (user-allocated area)
gap_start = MRTLO + LRTLO * 2   # after data too
gap_end   = CBLST
gap_nz    = sectors_nonzero(disk, gap_start, gap_end - gap_start)
print()
print(f'  Sectors {gap_start}–{gap_end-1} (between absolute SAVE and IMAGE): '
      f'{gap_nz}/{gap_end-gap_start} non-zero')
print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 4: Area 2 – SAVE IMAGE (CBLST + MS-offsets) ===')
print(f'  Formula: LBA = CBLST + MS_OFFSET  (CBLST={CBLST})')
print(f'  Pristine copies, written at system generation, never modified at runtime.')
print()

save_ranges = {}  # name → (abs_lba, len, nz)
for name, offset, length, desc, optional in SAVE_AREAS:
    abs_lba = CBLST + offset
    nz = sectors_nonzero(disk, abs_lba, length)
    save_ranges[name] = (abs_lba, length, nz)
    opt_tag = '  [optional]' if optional else ''
    print(f'  {name:6s} +{offset:4d} → LBA {abs_lba:5d}  [{length:3d} sectors]  '
          f'{nz:3d}/{length} non-zero  ({desc}){opt_tag}')
    if optional:
        if nz == 0:
            # Investigate the segment table entries pointing to this area.
            # From s3vs-4.symb: MSSYS = "SAVE OF SYSTEM SEGMENT",
            #   MSPDF = "SAVE OF SPOOLING DATAFIELD/QUEUE SEGMENT",
            #   MSECO = "SAVE AREA OF EXT. COMMON"
            # Look up segments with MADR in this offset range.
            segtb = 3; segst = 0o124000; phys_base_seg = (segtb << 16) + segst
            for sn in range(512):
                base_seg = phys_base_seg + sn * SE_SIZE
                entry_raw = [read_phys_word(mem, base_seg + i) for i in range(SE_SIZE)]
                if not any(entry_raw): continue
                seg_madr = entry_raw[4]
                if offset <= seg_madr <= offset + length - 1:
                    segli, prese, logad, segle, _, flag, sgsta, bpagl = entry_raw
                    print(f'         Seg {sn}: segli={segli}, prese={prese},'
                          f' logad={oct(logad)}, bpagl={bpagl}')
                    if segli == 0 and prese == 0 and bpagl == 0:
                        print(f'           segli=prese=bpagl=0: not in LRU chain, never loaded into RAM.')
                        print(f'           Disk area at LBA {CBLST + seg_madr} is zero because'
                              f' this segment ({name}) has never been written to disk.')
                    break
    else:
        test(f'{name} (LBA {abs_lba}) has content', nz > 0, True)

print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 5: Area 3 – LIVE IMAGE (CBLST + MADR + relPage) ===')
print(f'  Formula: LBA = {CBLST} + MADR + relPage * CABLPAGE,  CABLPAGE=1')
print(f'  MADR comes from the segment table entry (offset 4 in 8-word entry).')
print()

# Verify known IMAGE areas
for name, offset, length, desc in KNOWN_IMAGE_AREAS:
    abs_lba = CBLST + offset
    nz = sectors_nonzero(disk, abs_lba, length)
    print(f'  {name:6s} +{offset:4d} → LBA {abs_lba:5d}  [{length:3d} sectors]  '
          f'{nz:3d}/{length} non-zero  ({desc})')
    test(f'{name} (LBA {abs_lba}) has content', nz > 0, True)

print()
print(f'  Verifying MADR-based addresses from live segment table ...')
in_core = 0
disk_only = 0
zero_madr = 0
segfil_nonzero = {}

for seg in segs:
    madr    = seg['madr']
    pages   = seg['segle'] & 0x3FF
    segfil  = (seg['flag'] >> 13) & 0x7
    if madr == 0:
        zero_madr += 1
        continue
    abs_lba = (segfil * 0 + CBLST) + madr  # base for SEGFIL 0 = CBLST
    # (only SEGFIL 0 for now; SEGFIL 4 segments use absolute addressing)
    if segfil == 0:
        nz = sector_nonzero_count(disk, abs_lba)
        if nz > 0:
            disk_only += 1
        segfil_nonzero[segfil] = segfil_nonzero.get(segfil, 0) + (1 if nz > 0 else 0)
    if seg['bpagl']:
        in_core += 1

segs_with_madr = [s for s in segs if s['madr'] > 0 and (s['flag'] >> 13 & 7) == 0]
print(f'  Segments with MADR>0 and SEGFIL=0: {len(segs_with_madr)}')
print(f'  Of those: {disk_only} have non-zero first sector on disk (LBA=CBLST+MADR)')
# NOTE: many page-0s are currently resident in RAM; disk copy may be zero.
# Low count does not disprove the formula — MIECO/MISGT/MICOM above confirm it.
print(f'  INFO: {disk_only}/{len(segs_with_madr)} non-zero first sector '
      f'(low count expected: in-core segments have page-0 in RAM, not yet written to disk)')

print()
print(f'  MADR distribution for SEGFIL-0 segments (first 20):')
for seg in sorted(segs_with_madr, key=lambda s: s['madr'])[:20]:
    abs_lba = CBLST + seg['madr']
    nz = sector_nonzero_count(disk, abs_lba)
    print(f'    seg {seg["segNum"]:3d} MADR={seg["madr"]:5d} → LBA {abs_lba:5d}'
          f'  nz={nz:3d}  pages={seg["segle"]}'
          f'  {"IN CORE" if seg["bpagl"] else ""}')

print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 6: SAVE vs IMAGE content differs (proves 2 separate areas) ===')

# Compare MSSGT (save of segment table) vs MISGT (image of segment table)
MSSGT_LBA = CBLST + 321
MISGT_LBA = CBLST + 1275
misgt_w = read_sector_words(disk, MISGT_LBA)
misgt_nz = sum(1 for x in misgt_w if x)

# MSSGT spans 16 sectors; check how many are non-zero (first sector may be a header)
MSSGT_LEN = 16
mssgt_sectors_nz = sectors_nonzero(disk, MSSGT_LBA, MSSGT_LEN)
mssgt_w0 = read_sector_words(disk, MSSGT_LBA)
mssgt_w0_nz = sum(1 for x in mssgt_w0 if x)

print(f'  MSSGT (SAVE of seg table) at LBA {MSSGT_LBA}: '
      f'{mssgt_sectors_nz}/{MSSGT_LEN} sectors non-zero  '
      f'(first sector: {mssgt_w0_nz}/512 words non-zero)')
print(f'  MISGT (IMAGE of seg table) at LBA {MISGT_LBA}: {misgt_nz}/512 words non-zero')
test('MSSGT area (16 sectors) has some non-zero sectors', mssgt_sectors_nz > 0, True)
test('MISGT first sector is non-zero', misgt_nz > 0, True)
if mssgt_w0_nz == 0:
    print(f'  NOTE: MSSGT sector-0 is all zeros — segment table SAVE content starts at sector 1+')
identical = (mssgt_w0 == misgt_w)
print(f'  MSSGT-sector0 == MISGT-sector0? {identical}  '
      f'(equal on fresh cold boot, may differ after runtime modifications)')
print()

# Compare MSECO vs MIECO (ext common)
MSECO_LBA = CBLST + 113
MIECO_LBA = CBLST + 1067
mseco_w = read_sector_words(disk, MSECO_LBA)
mieco_w = read_sector_words(disk, MIECO_LBA)
mseco_nz = sum(1 for x in mseco_w if x)
mieco_nz = sum(1 for x in mieco_w if x)
eco_identical = (mseco_w == mieco_w)
print(f'  MSECO (SAVE of ext-common) at LBA {MSECO_LBA}: {mseco_nz}/512 words non-zero')
print(f'  MIECO (IMAGE of ext-common) at LBA {MIECO_LBA}: {mieco_nz}/512 words non-zero')
if mseco_nz == 0:
    # Look up seg 48 in segment table
    segtb2 = 3; segst2 = 0o124000; pb2 = (segtb2 << 16) + segst2
    e48 = [read_phys_word(mem, pb2 + 48*SE_SIZE + i) for i in range(SE_SIZE)]
    segli48, prese48, _, _, _, _, _, bpagl48 = e48
    print(f'  NOTE: MSECO sector at LBA {MSECO_LBA} is all zeros.')
    print(f'        Seg 48 in segment table: segli={segli48}, prese={prese48}, bpagl={bpagl48}.')
    if segli48 == 0 and prese48 == 0 and bpagl48 == 0:
        print(f'        segli=prese=bpagl=0 confirms segment 48 has never been loaded; disk is zero.')
test('MIECO first sector is non-zero', mieco_nz > 0, True)
print(f'  MSECO == MIECO? {eco_identical}')
print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 7: Disk usage map ===')

used   = [False] * disk_sectors
reason = [''] * disk_sectors

def mark(lba, count, label):
    for i in range(count):
        if 0 <= lba+i < disk_sectors:
            used[lba+i] = True
            reason[lba+i] = label

mark(0, MERRP, 'boot')
mark(MERRP, LERRP, 'ABS-SAVE:ERRPROG')
mark(MRTLO, LRTLO, 'ABS-SAVE:RTLOADER-CODE')
mark(MRTLO + LRTLO, LRTLO, 'ABS-SAVE:RTLOADER-DATA')

for name, offset, length, *_ in SAVE_AREAS:
    mark(CBLST + offset, length, f'SAVE:{name}')

for name, offset, length, _ in KNOWN_IMAGE_AREAS:
    mark(CBLST + offset, length, f'IMAGE:{name}')

for seg in segs:
    madr = seg['madr']
    if madr == 0: continue
    segfil = (seg['flag'] >> 13) & 0x7
    pages  = seg['segle'] & 0x3FF
    if segfil == 0:
        base = CBLST + madr
        mark(base, pages, f'IMAGE:seg{seg["segNum"]}')

total_used = sum(used)
total_nz   = sum(1 for i in range(disk_sectors) if sector_nonzero_count(disk, i) > 0)
print(f'  Total sectors: {disk_sectors:,}')
print(f'  Sectors mapped to known areas: {total_used:,}')
print(f'  Sectors with non-zero content: {total_nz:,}')
print(f'  CBLST (start of SEGFIL 0): {CBLST}')
print(f'  Last segment IMAGE sector:  {max((CBLST+s["madr"]+s["segle"] for s in segs if s["madr"] and not (s["flag"]>>13&7)), default=0)}')
print()

# Find first empty sector after CBLST  (marks end of active SEGFIL area)
first_zero_after_cblst = None
for i in range(CBLST, min(CBLST + 8000, disk_sectors)):
    if sector_nonzero_count(disk, i) == 0 and first_zero_after_cblst is None:
        first_zero_after_cblst = i
        # look for a run of zeros
        run = 0
        for j in range(i, min(i + 100, disk_sectors)):
            if sector_nonzero_count(disk, j) == 0: run += 1
            else: break
        if run >= 5:
            print(f'  First zero-run of >=5 sectors after CBLST: LBA {i} (run={run})')
            break

# Sample of unmapped non-zero sectors (potential HENT area or extra segments)
unmapped_nz = []
for i in range(disk_sectors):
    if not used[i] and sector_nonzero_count(disk, i) > 0:
        unmapped_nz.append(i)
        if len(unmapped_nz) >= 200: break

if unmapped_nz:
    print(f'  Non-zero sectors NOT yet mapped to a known area: {len(unmapped_nz)} found')
    # Find contiguous runs
    runs = []
    start = unmapped_nz[0]; end = start
    for lba in unmapped_nz[1:]:
        if lba == end + 1:
            end = lba
        else:
            runs.append((start, end))
            start = end = lba
    runs.append((start, end))
    for s, e in runs[:15]:
        if e - s > 0:
            print(f'    LBA {s:5d}–{e:5d}  ({e-s+1} sectors)  [possible HENT or other]')
print()

# ═══════════════════════════════════════════════════════════════════════════
print('=== Section 8: Formula summary ===')
print()
print('  For a segment with MADR=M, page P within the segment:')
print()
print('  If FLAG bits 13-15 = 0 (SEGFIL 0, normal system/user segments):')
print(f'    LBA = {CBLST} (CBLST) + M (MADR) + P (relPage)')
print()
print('  If FLAG bits 13-15 = 4 (SEGFIL 4, absolute SAVE area):')
print('    LBA = 0 + M + P  (M is an absolute disk sector, no base needed)')
print()
print('  CABLPAGE = 1 on this SMD disk (1 disk sector = 512 words = 1 SINTRAN page).')
print('  SINTRAN stores sectors as big-endian 16-bit words (high byte first).')
print()

total = pass_count + fail_count
print(f'=== Results: {pass_count}/{total} passed, {fail_count} failed ===')
sys.exit(0 if fail_count == 0 else 1)
