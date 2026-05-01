//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// segment-disassembler.js — Segment Disassembler Window
//
// I/D bank model:
//   No split  → one "Disassembly" tab, no D tab.
//   Split      → "Disassembly" (I bank) + "D Bank" (hex dump of DATA segment).
//   The D bank is a SEPARATE segment loaded explicitly via "Load D Bank" button.
//
// Save button:
//   No split  → NAME.PROG (single bank) + NAME-disasm.md (I section only)
//   Split      → NAME.PROG (dual bank)   + NAME-disasm.md (I + D sections)

(function() {
  'use strict';

  // SINTRAN III VSX-L disk layout constants (verified from NPL source).
  // PH-P2-OPPSTART.NPL:769,1377: SEGFSTART = CBLST = 200₈ = 128 (in MMU pages).
  // PH-P2-OPPSTART.NPL:743-745: CABLPAGE = 2000₈/SECWO = 2 (sectors per page, universal).
  // IP-P2-SEGADM.NPL:1576-1585: disk_LBA = BLSTX[segfile] + (MADR + relPage) × CABLPAGE.
  // BLSTX[0] (sectors) = CBLST × CABLPAGE = 128 × 2 = 256.
  var SECTOR_BYTES   = 1024;   // 1 disk sector = 1024 bytes = 512 words
  var CABLPAGE       = 2;      // sectors per page (universal SINTRAN constant)
  var WORDS_PER_PAGE = 1024;   // 1 page = 2 sectors = 1024 words
  var BLSTX_0        = 256;    // sectors — start of SEGFIL 0 (= CBLST × CABLPAGE)
  var BLSTX_4        = 0;      // sectors — SEGFIL 4 (absolute) base ≈ 0

  // ---- State ------------------------------------------------------------------
  var iSeg     = null;   // I bank segment entry (always set when window is open)
  var iWords   = null;   // I bank Uint16Array
  var iName    = '';
  var iStart   = 0;
  var iRestart = 0;

  var dSeg     = null;   // D bank segment entry — null means no split
  var dWords   = null;   // D bank Uint16Array

  var iSource  = 'IMAGE';   // current source for I bank: 'IMAGE' | 'SAVE' | 'HENT'

  var segfilBases = null;  // [base0, base1, base2, base3] for SEGFIL 0-3; null = not read

  // Read up to 256 disk sectors via Dbg_ReadSMDSectors. Works for both file-backed
  // and in-memory drives. Returns a Uint8Array view (count*1024 bytes) into the
  // C-side static buffer. Throws Error with specific reason on failure.
  // IMPORTANT: the view is invalidated by the next readSectors() call (static buffer reuse).
  function readSectors(lba, count) {
    var emuRef = (typeof emu !== 'undefined') ? emu : null;
    if (!emuRef) {
      throw new Error('readSectors: emu proxy not loaded');
    }
    if (!emuRef.readSMDSectors) {
      throw new Error('readSectors: emu.readSMDSectors missing (WASM Dbg_ReadSMDSectors not exported — rebuild?)');
    }
    if (!emuRef.getHEAPU8) {
      throw new Error('readSectors: emu.getHEAPU8 missing');
    }
    if (count <= 0 || count > 256) {
      throw new Error('readSectors: count=' + count + ' out of range [1,256]');
    }
    var ptr = emuRef.readSMDSectors(0, lba, count);
    if (!ptr) {
      throw new Error('readSectors: Dbg_ReadSMDSectors returned 0 ' +
        '(unit=0 lba=' + lba + ' count=' + count + ' — drive not mounted? read past end?)');
    }
    var heapu8 = emuRef.getHEAPU8();
    if (!heapu8) {
      throw new Error('readSectors: HEAPU8 not accessible after Dbg_ReadSMDSectors');
    }
    return new Uint8Array(heapu8.buffer, ptr, count * 1024);
  }

  // Scan the SMD disk for the REECOMT signature (w303 = 0x4E52).
  // Returns the first matching LBA. Throws Error if not found or scan failed.
  function scanDiskForREECOMT() {
    var emuRef = (typeof emu !== 'undefined') ? emu : null;
    if (!emuRef || !emuRef.getSMDBufferSize) {
      throw new Error('scanDiskForREECOMT: emu.getSMDBufferSize missing');
    }
    var totalBytes = emuRef.getSMDBufferSize(0);
    if (!totalBytes) {
      throw new Error('scanDiskForREECOMT: SMD unit 0 not mounted (getSMDBufferSize returned 0)');
    }
    var totalSectors = (totalBytes / 1024) | 0;   // do NOT cap — disk may be 75MB+
    console.log('[disasm] scanning ' + totalSectors + ' sectors for REECOMT signature');
    var BATCH = 256;
    for (var base = 0; base < totalSectors; base += BATCH) {
      var n = Math.min(BATCH, totalSectors - base);
      var view = readSectors(base, n);   // throws on failure
      for (var i = 0; i < n; i++) {
        var off = i * 1024;
        // Strict REECOMT validation: word 303 = 0x4E52 ('NR')
        // AND first entry has plausible segment number (1..255) and name_ptr 0..512
        var w303 = (view[off + 606] << 8) | view[off + 607];
        if (w303 !== 0x4E52) continue;
        var w0_segno   = (view[off + 6] << 8) | view[off + 7];   // first entry word 3
        var w0_namePtr = (view[off + 4] << 8) | view[off + 5];   // first entry word 2
        if (w0_segno < 1 || w0_segno > 255) continue;            // implausible segno
        if (w0_namePtr > 512) continue;                          // namePtr out of page
        return base + i;
      }
    }
    throw new Error('scanDiskForREECOMT: REECOMT signature not found in ' +
      totalSectors + ' sectors (looked for word 303=0x4E52 + valid first entry)');
  }

  // Discover SEGFIL0 base sector.
  //
  // Step 1: scan disk for REECOMT signature (word 303 = 0x4E52) → reecomtLBA.
  //         This is always deterministic — there is exactly one matching LBA.
  //
  // Step 2: walk 5OPSEG's (seg 3) core-map chain to collect all in-RAM pages.
  //
  // Step 3a (fast path): if ANY in-RAM page of 5OPSEG has word 303 = 0x4E52, that
  //         page IS REECOMT. Its relPage gives base = reecomtLBA - madr3 - relPage.
  //
  // Step 3b (fallback): if REECOMT is not in RAM, use disk/RAM comparison.
  //         For each in-RAM page (as anchor), read its first 8 words from RAM and from
  //         disk at each of the ~50 candidate LBAs (reecomtLBA - madr3 - r for r in
  //         0..pages3-1).  When ≥6/8 words match, the matching candidate r gives base.
  //         This works because at least some in-RAM pages of 5OPSEG are code and do
  //         not change between disk-write and now.
  //
  // Returns [0,0,0,0] only if all anchors fail (e.g. all in-RAM pages are data pages
  // that have changed since last disk write, or 5OPSEG is fully evicted).
  function getSegfilBasesAsync() {
    if (segfilBases) return Promise.resolve(segfilBases);

    var sym = window.sintranSymbols;
    if (!sym || !sym.readWord || !sym.readBlockPhysical) {
      return Promise.resolve([0, 0, 0, 0]);
    }

    var preDiscovered = (typeof window.getDiscoveredSegfilBase0 === 'function')
      ? window.getDiscoveredSegfilBase0() : 0;
    if (preDiscovered > 0) {
      segfilBases = [preDiscovered, 0, 0, 0];
      console.log('[disasm] SEGFIL0 base=' + preDiscovered + ' (from reentrant module)');
      return Promise.resolve(segfilBases);
    }

    var reecomtLBA;
    try {
      reecomtLBA = scanDiskForREECOMT();
    } catch (e) {
      console.warn('[disasm] disk scan failed:', e.message);
      return Promise.reject(e);  // surface to caller (UI can display)
    }
    console.log('[disasm] REECOMT at disk LBA=' + reecomtLBA);

    return Promise.all([
      sym.readWord(sym.FIXED.SEGTB),
      sym.readWord(sym.FIXED.SEGST),
      sym.readWord(sym.FIXED.CORMB)
    ]).then(function(vals) {
      var segtb = vals[0], segst = vals[1], cormb = vals[2];
      var physBase = (segtb << 16) + segst;
      if (!physBase) throw new Error('physBase=0 (SINTRAN not running?)');
      var cormBase = (cormb & 0xFF) << 16;

      return sym.readBlockPhysical(physBase + 3 * 8, 8).then(function(s3) {
        if (!s3 || s3.length < 8) throw new Error('seg3 read failed');
        var madr3  = s3[4];
        var logad3 = s3[2];
        var pages3 = s3[3] & 0x3FF;
        var bpagl3 = s3[7];
        if (!bpagl3) throw new Error('5OPSEG not in RAM');

        // Walk 5OPSEG chain — collect ALL in-RAM relPage → physPage entries
        var physForRelPage3 = {};
        var chainLimit = 256;
        function walkSeg3(entryAddr) {
          if (entryAddr === 0 || chainLimit-- <= 0) return Promise.resolve();
          return sym.readBlockPhysical(cormBase + entryAddr, 4).then(function(cme) {
            if (!cme || cme.length < 4) return Promise.resolve();
            var physPage = (entryAddr / 4) | 0;
            var relPage  = cme[3] - logad3;
            if (relPage >= 0 && relPage < pages3) {
              physForRelPage3[relPage] = physPage;
            }
            return walkSeg3(cme[0]);
          });
        }

        return walkSeg3(bpagl3).then(function() {
          var inRamRelPages = Object.keys(physForRelPage3).map(Number);
          console.log('[disasm] 5OPSEG in-RAM relPages: [' + inRamRelPages.sort(function(a,b){return a-b;}).join(',') + ']');

          // Fast path: check each in-RAM page for the REECOMT signature.
          // Strict validation: word 303 = 0x4E52 ('NR'), word 304 high byte = 'L',
          // first 2 entries have plausible distinct segnos, namePtrs are non-zero,
          // and apostrophe separator (0x27) exists in the name table.
          function checkSig(list, idx) {
            if (idx >= list.length) return Promise.resolve(-1);
            var rp = list[idx];
            var pp = physForRelPage3[rp];
            return sym.readBlockPhysical(pp * 1024, 512).then(function(w) {
              if (!w || w.length < 512) return checkSig(list, idx + 1);
              if (w[303] !== 0x4E52) return checkSig(list, idx + 1);
              if ((w[304] >>> 8) !== 0x4C) {
                console.log('[disasm] checkSig: rp=' + rp + ' has w303=0x4E52 but w304 != "L..." — skipping');
                return checkSig(list, idx + 1);
              }
              var seg0 = w[3], seg1 = w[7];
              if (seg0 < 1 || seg0 > 255 || seg1 < 1 || seg1 > 255 || seg0 === seg1) {
                console.log('[disasm] checkSig: rp=' + rp + ' bad segnos (entry0=' + seg0 +
                  ' entry1=' + seg1 + ') — skipping');
                return checkSig(list, idx + 1);
              }
              if (w[2] === 0 || w[2] === 0xFFFF || w[6] === 0 || w[6] === 0xFFFF) {
                console.log('[disasm] checkSig: rp=' + rp + ' bad namePtrs — skipping');
                return checkSig(list, idx + 1);
              }
              return rp;
            });
          }

          return checkSig(inRamRelPages, 0).then(function(reecomtRelPage) {
            if (reecomtRelPage >= 0) {
              var base = reecomtLBA - madr3 - reecomtRelPage;
              console.log('[disasm] SEGFIL0 base=' + base +
                ' (REECOMT in RAM relPage=' + reecomtRelPage + ')');
              segfilBases = [Math.max(0, base), 0, 0, 0];
              return segfilBases;
            }

            // Fallback: disk/RAM comparison using any in-RAM page as anchor
            console.log('[disasm] REECOMT not in RAM — using disk/RAM comparison');

            function diskFirst8(lba) {
              var view = readSectors(lba, 1);
              if (!view) return null;
              var w = [];
              for (var i = 0; i < 8; i++) {
                w.push(((view[i*2] << 8) | view[i*2+1]) & 0xFFFF);
              }
              return w;
            }

            var sortedAnchors = inRamRelPages.slice().sort(function(a, b) { return a - b; });

            function tryAnchor(aIdx) {
              if (aIdx >= sortedAnchors.length) return Promise.resolve(null);
              var rAnchor = sortedAnchors[aIdx];
              var physPage = physForRelPage3[rAnchor];
              return sym.readBlockPhysical(physPage * 1024, 8).then(function(ramW) {
                if (!ramW || ramW.length < 8) return tryAnchor(aIdx + 1);
                var nzCount = 0;
                for (var i = 0; i < 8; i++) { if (ramW[i]) nzCount++; }
                if (nzCount < 3) return tryAnchor(aIdx + 1);  // skip near-zero pages
                for (var rReecomt = 0; rReecomt < pages3; rReecomt++) {
                  var candidateBase = reecomtLBA - madr3 - rReecomt;
                  if (candidateBase <= 0) continue;
                  var diskW = diskFirst8(candidateBase + madr3 + rAnchor);
                  if (!diskW) continue;
                  var matches = 0;
                  for (var wi = 0; wi < 8; wi++) { if (ramW[wi] === diskW[wi]) matches++; }
                  if (matches >= 6) {
                    console.log('[disasm] SEGFIL0 base=' + candidateBase +
                      ' (disk/RAM: anchor relPage=' + rAnchor + ' rReecomt=' + rReecomt +
                      ' matches=' + matches + '/8)');
                    return candidateBase;
                  }
                }
                return tryAnchor(aIdx + 1);
              });
            }

            return tryAnchor(0).then(function(base) {
              if (base === null) throw new Error('disk/RAM comparison failed for all in-RAM 5OPSEG pages');
              segfilBases = [Math.max(0, base), 0, 0, 0];
              return segfilBases;
            });
          });
        });
      });
    }).catch(function(e) {
      console.warn('[disasm] getSegfilBasesAsync failed:', e.message || e);
      segfilBases = null;  // allow retry next open
      throw e;             // propagate so caller can show error in UI
    });
  }

  // =========================================================
  // BLSTX[segfil] — disk base in sectors, per SEGFIL number.
  // SEGFIL 0: BLSTX[0] = CBLST × CABLPAGE = 256 (verified from NPL).
  // SEGFIL 4: absolute disk addressing, BLSTX[4] = 0.
  // SEGFIL 1..3: not yet determined. Returns null for unsupported segfils.
  // =========================================================
  function blstxForSegfil(n) {
    if (n === 0) return BLSTX_0;
    if (n === 4) return BLSTX_4;
    return null;
  }

  // =========================================================
  // Source-only reader (for SAVE / HENT). Reads SECOLO pages from disk
  // at the appropriate base, NO RAM fallback.
  // =========================================================
  function readFromDiskOnly(segEntry, source, pages, result, pageSource) {
    var segfilNum = (segEntry.flag >> 13) & 0x7;
    var madr = segEntry.madr;

    var base, label, sourceName = source;
    if (source === 'SAVE') {
      // For system segments only — at MS_xxx offset which we don't have here without
      // a per-segment lookup table. NOT IMPLEMENTED for arbitrary segments.
      return Promise.reject(new Error(
        'SAVE source not yet implemented for arbitrary segments. ' +
        'System segments have explicit MS_xxx offsets; user reentrant segments may not have a separate SAVE area.'));
    } else if (source === 'HENT') {
      // HENT = warm-restart memory snapshot. Per NPL research:
      //   MIMAG=0 on SEGFIL 4, length LIMAG=77₈=63 sectors. Stored as segment SG52
      //   flagged SGFI4 (absolute addressing on SEGFIL 4).
      // Whether this contains the FULL memory or just COMMON is unclear.
      // For now: try reading at LBA 0 + (MADR + relPage) × CABLPAGE assuming the
      // segment's data is at its MADR within the HENT area. This is a HYPOTHESIS,
      // not verified.
      base = BLSTX_4;
      label = 'HENT (hypothesis: SEGFIL 4 absolute, MADR + relPage × CABLPAGE)';
    } else {
      return Promise.reject(new Error('Unknown source: ' + source));
    }

    console.log('[disasm] disk-only read (' + sourceName + '): ' + label +
      ' base=' + base + ' madr=' + oct6(madr) + ' CABLPAGE=' + CABLPAGE);

    var firstReadFailure = null;
    for (var pg = 0; pg < pages; pg++) {
      var lba = base + (madr + pg) * CABLPAGE;
      try {
        var view = readSectors(lba, CABLPAGE);
        var resBase = pg * WORDS_PER_PAGE;
        var nonZero = false;
        for (var w = 0; w < WORDS_PER_PAGE; w++) {
          var word = ((view[w*2] << 8) | view[w*2 + 1]) & 0xFFFF;
          result[resBase + w] = word;
          if (word) nonZero = true;
        }
        pageSource[pg] = nonZero ? (sourceName.toLowerCase() + ':lba' + lba)
                                 : (sourceName.toLowerCase() + '-zero:lba' + lba);
      } catch (rerr) {
        pageSource[pg] = 'err:' + rerr.message;
        if (!firstReadFailure) firstReadFailure = rerr;
      }
    }
    if (firstReadFailure && pageSource.every(function(s) { return s && s.slice(0,3) === 'err'; })) {
      return Promise.reject(new Error('readFromDiskOnly: all ' + pages +
        ' reads failed — first error: ' + firstReadFailure.message));
    }
    // Run the same per-page diagnostic as the main path
    var totalNZ = 0;
    for (var i = 0; i < result.length; i++) if (result[i]) totalNZ++;
    console.log('[disasm] ' + sourceName + ' result: total ' + totalNZ + '/' +
      (pages * WORDS_PER_PAGE) + ' words non-zero | sources: ' + pageSource.join(' '));
    return Promise.resolve(result);
  }

  // =========================================================
  // Segment data reader — physical memory primary, disk fallback
  // =========================================================

  // Source selector: 'IMAGE' | 'SAVE' | 'HENT'
  // - IMAGE: live working copy. LBA = BLSTX[segfile] + (MADR + relPage) × CABLPAGE
  // - SAVE:  pristine cold-start backup. Only system segments have explicit MS_xxx offsets.
  //          For user reentrant segments: no separate SAVE area exists per NPL research.
  // - HENT:  warm-restart memory snapshot. Address calculation TBD.
  function readSegmentDataAsync(segEntry, source) {
    source = source || 'IMAGE';
    var pages = segEntry.segle & 0x3FF;
    if (pages === 0 || pages > 256) {
      console.warn('[disasm] unreasonable page count:', pages, 'for seg', oct6(segEntry.segNum),
        'bpagl=' + segEntry.bpagl + ' logad=' + segEntry.logad + ' segle=' + segEntry.segle);
      return Promise.resolve(null);
    }

    var result    = new Uint16Array(pages * WORDS_PER_PAGE);
    var pageSource = [];
    var sym       = window.sintranSymbols;

    console.log('[disasm] seg ' + oct6(segEntry.segNum) + ' source=' + source +
      ' pages=' + pages + ' logad=' + segEntry.logad +
      ' bpagl=' + segEntry.bpagl + ' madr=' + oct6(segEntry.madr));

    // For SAVE/HENT, skip RAM read (we want disk content of that area, not current RAM).
    // For IMAGE, RAM has the most current data (in-RAM pages override disk).
    if (source !== 'IMAGE') {
      return readFromDiskOnly(segEntry, source, pages, result, pageSource);
    }

    return sym.readWord(sym.FIXED.CORMB).then(function(cormb) {
      var cormBase = (cormb & 0xFF) << 16;
      var physForRelPage = {};
      var limit = 512;
      var chainVisited = [];

      console.log('[disasm] CORMB=' + cormb + ' cormBase=0x' + cormBase.toString(16));

      function walkChain(entryAddr) {
        if (entryAddr === 0 || limit-- <= 0) return Promise.resolve();
        return sym.readBlockPhysical(cormBase + entryAddr, 4).then(function(cme) {
          if (!cme || cme.length < 4) return Promise.resolve();
          var physPage = (entryAddr / 4) | 0;
          var logpa    = cme[3];
          var relPage  = logpa - segEntry.logad;
          chainVisited.push('pp=' + physPage + '/lp=' + logpa + '(rel=' + relPage + ')');
          if (relPage >= 0 && relPage < pages) {
            physForRelPage[relPage] = physPage;
          }
          return walkChain(cme[0]);
        });
      }

      if (!segEntry.bpagl) {
        console.log('[disasm] bpagl=0 — segment not in RAM, going to disk');
        return Promise.resolve(physForRelPage);
      }
      return walkChain(segEntry.bpagl).then(function() {
        console.log('[disasm] chain (' + chainVisited.length + ' entries): ' + chainVisited.slice(0, 8).join(' ') +
          (chainVisited.length > 8 ? '...' : ''));
        console.log('[disasm] pages in RAM: ' + Object.keys(physForRelPage).length + '/' + pages);
        return physForRelPage;
      });

    }).then(function(physForRelPage) {
      var memReadPromises = [];
      for (var p = 0; p < pages; p++) {
        if (physForRelPage[p] !== undefined) {
          (function(page, physPage) {
            var pr = sym.readBlockPhysical(physPage * 1024, WORDS_PER_PAGE).then(function(data) {
              result.set(data, page * WORDS_PER_PAGE);
              pageSource[page] = 'mem:' + physPage;
            });
            memReadPromises.push(pr);
          })(p, physForRelPage[p]);
        }
      }
      return Promise.all(memReadPromises).then(function() { return physForRelPage; });

    }).then(function(physForRelPage) {
      var missing = [];
      for (var p2 = 0; p2 < pages; p2++) {
        if (physForRelPage[p2] === undefined) missing.push(p2);
      }
      if (missing.length === 0) return physForRelPage;

      var segfilNum  = (segEntry.flag >> 13) & 0x7;
      var blstx      = blstxForSegfil(segfilNum);
      if (blstx === null) {
        throw new Error('readSegmentDataAsync: SEGFIL ' + segfilNum +
          ' base unknown (only SEGFIL 0 and 4 supported, got ' + segfilNum + ')');
      }
      var madr = segEntry.madr;
      console.log('[disasm] disk fallback (IMAGE): ' + missing.length + ' missing pages,' +
        ' segfil=' + segfilNum + ' BLSTX[' + segfilNum + ']=' + blstx +
        ' madr=' + oct6(madr) + ' CABLPAGE=' + CABLPAGE);

      var firstReadFailure = null;
      for (var i = 0; i < missing.length; i++) {
        var pg = missing[i];
        // Verified formula (IP-P2-SEGADM.NPL:1576-1585):
        // LBA = BLSTX[segfile] + (MADR + relPage) × CABLPAGE
        var lba = blstx + (madr + pg) * CABLPAGE;
        try {
          // 1 page = CABLPAGE sectors (= 2 sectors = 1024 words on standard SINTRAN)
          var view = readSectors(lba, CABLPAGE);
          var resBase = pg * WORDS_PER_PAGE;
          var nonZero = false;
          for (var w = 0; w < WORDS_PER_PAGE; w++) {
            var word = ((view[w*2] << 8) | view[w*2 + 1]) & 0xFFFF;
            result[resBase + w] = word;
            if (word) nonZero = true;
          }
          pageSource[pg] = nonZero ? ('disk:lba' + lba) : ('disk-zero:lba' + lba);
        } catch (rerr) {
          pageSource[pg] = 'err:' + rerr.message;
          if (!firstReadFailure) firstReadFailure = rerr;
        }
      }
      if (firstReadFailure && missing.every(function(pg) { return pageSource[pg].slice(0,3) === 'err'; })) {
        throw new Error('readSegmentDataAsync: all ' + missing.length +
          ' disk reads failed — first error: ' + firstReadFailure.message);
      }
      return physForRelPage;
    }).then(function() {
      var memCnt  = pageSource.filter(function(s) { return s && s.slice(0,3) === 'mem'; }).length;
      var diskCnt = pageSource.filter(function(s) { return s && s.slice(0,4) === 'disk'; }).length;
      console.log('[disasm] result: ' + memCnt + ' pages from RAM, ' + diskCnt + ' from disk, sources: ' + pageSource.join(' '));

      // Data quality check — per-page non-zero count
      var totalNZ = 0;
      var perPageStats = [];
      var firstNZWord = -1;
      for (var pi = 0; pi < pages; pi++) {
        var pgNZ = 0;
        for (var wi = 0; wi < WORDS_PER_PAGE; wi++) {
          var word = result[pi * WORDS_PER_PAGE + wi];
          if (word) {
            pgNZ++;
            totalNZ++;
            if (firstNZWord < 0) firstNZWord = pi * WORDS_PER_PAGE + wi;
          }
        }
        perPageStats.push('p' + pi + '(' + (pageSource[pi] || '?') + '):' + pgNZ);
      }
      console.log('[disasm] per-page nz: ' + perPageStats.join(' '));
      console.log('[disasm] total: ' + totalNZ + '/' + (pages * WORDS_PER_PAGE) +
        ' words non-zero' +
        (firstNZWord >= 0 ? ' | first nz at word ' + firstNZWord +
          ' (page ' + ((firstNZWord / WORDS_PER_PAGE)|0) + ')' : ' ← ALL ZERO across all pages'));

      return result;
    });
  }

  // =========================================================
  // Segment classification from SGSTA
  // WPM=bit15, RPM=bit14, FPM=bit13
  // =========================================================

  // Standard SINTRAN segments have WPM+RPM+FPM (0o162000) — this is normal for all code segments.
  // Only FPM-clear segments are pure data (no execute permission).
  function classifySegment(sgsta) {
    var wpm  = (sgsta >> 15) & 1;
    var fpm  = (sgsta >> 13) & 1;
    var ring = (sgsta >> 9) & 3;   // bits 9-10: 0=RING0, 1=RING1, 2=RING2
    var noclear = sgsta & 1;       // bit 0: NOCLEAR

    var ringLabel = 'R' + ring + (noclear ? ' NC' : '');
    if (fpm && wpm)  return { label: ringLabel,           color: '#4CAF50' };  // normal: exec+write (most segments)
    if (fpm && !wpm) return { label: ringLabel + ' RO',   color: '#2196F3' };  // read-only code
    if (!fpm && wpm) return { label: ringLabel + ' DATA', color: '#FFA726' };  // data, no execute
    return                  { label: 'UNKNOWN',           color: 'rgba(160,175,210,0.5)' };
  }

  // =========================================================
  // Tab visibility helpers
  // =========================================================

  function showDTab(label) {
    var btn = document.getElementById('seg-disasm-tab-d');
    if (btn) {
      btn.style.display = '';
      btn.textContent = label || 'D Bank';
    }
  }

  function hideDTab() {
    var btn = document.getElementById('seg-disasm-tab-d');
    if (btn) btn.style.display = 'none';
    // Switch back to I tab if D was active
    var dP = document.getElementById('seg-disasm-d-panel');
    var iP = document.getElementById('seg-disasm-i-panel');
    var iBtn = document.getElementById('seg-disasm-tab-i');
    var dBtn = document.getElementById('seg-disasm-tab-d');
    if (dP) dP.style.display = 'none';
    if (iP) iP.style.display = '';
    if (iBtn) iBtn.classList.add('active');
    if (dBtn) dBtn.classList.remove('active');
  }

  // =========================================================
  // Core open / load functions
  // =========================================================

  function openDisasmWindow(seg, name, stadr, restart) {
    iSeg     = seg;
    iName    = name;
    iStart   = stadr   || 0;
    iRestart = restart || 0;
    iWords   = null;
    dSeg     = null;
    dWords   = null;

    var win = document.getElementById('seg-disasm-window');
    if (!win) return;

    // Title + classification badge
    var cls     = classifySegment(seg.sgsta || 0);
    var titleEl = document.getElementById('seg-disasm-title');
    if (titleEl) {
      titleEl.innerHTML =
        escHtml('Disassembler: ' + name + ' [' + seg.segNum.toString(8) + ']') +
        '&nbsp;<span style="font-size:10px;padding:1px 5px;border-radius:3px;' +
        'background:rgba(0,0,0,0.3);color:' + cls.color + ';border:1px solid ' + cls.color + '60;">' +
        cls.label + '</span>';
    }

    // Reset tab bar: I tab active, D tab hidden
    var iBtn = document.getElementById('seg-disasm-tab-i');
    if (iBtn) { iBtn.classList.add('active'); iBtn.textContent = 'Disassembly'; }
    hideDTab();
    // Show the "Load D Bank" button so user can attach a companion data segment
    var loadDBtn = document.getElementById('seg-disasm-load-d');
    if (loadDBtn) loadDBtn.style.display = '';
    // Sync source dropdown with current state
    var srcSelect = document.getElementById('seg-disasm-source');
    if (srcSelect) srcSelect.value = iSource;

    var iPanel = document.getElementById('seg-disasm-i-panel');
    var dPanel = document.getElementById('seg-disasm-d-panel');
    if (iPanel) { iPanel.style.display = ''; iPanel.innerHTML = '<div style="padding:12px;color:rgba(160,175,210,0.5);">Reading segment…</div>'; }
    if (dPanel) { dPanel.style.display = 'none'; dPanel.innerHTML = ''; }

    win.style.display = 'flex';
    if (typeof windowManager !== 'undefined') windowManager.focus('seg-disasm-window');

    readSegmentDataAsync(seg, iSource).then(function(words) {
      if (!words) {
        if (iPanel) iPanel.innerHTML = '<div style="padding:12px;color:rgba(255,80,80,0.8);">' +
          'Failed to read segment data.<br><span style="font-size:10px;color:rgba(160,175,210,0.6);">' +
          'See browser console for details (filter: <b>[disasm]</b>)</span></div>';
        return;
      }
      iWords = words;
      // Empty-result detection — report observed facts only, no speculation
      var totalNZ = 0;
      for (var i = 0; i < words.length; i++) if (words[i]) { totalNZ++; if (totalNZ > 8) break; }
      if (totalNZ === 0) {
        var pages = (seg.segle & 0x3FF);
        var sharedBit = (seg.flag & (1 << 4)) !== 0;
        var lines = [];
        lines.push('All ' + pages + ' pages of segment ' + oct6(seg.segNum) +
          ' (' + escHtml(iName) + ') read back as zero.');
        lines.push('BPAGL = ' + seg.bpagl + ' (' + (seg.bpagl ? 'in RAM' : 'not in LRU chain') + ')');
        lines.push('FLAG bit 4 (5SREE / SHARED-REENTRANT): ' + (sharedBit ? 'SET' : 'not set'));
        lines.push('Disk fallback used base + MADR + relPage. See console <b>[disasm]</b> log for the LBAs read.');
        lines.push('We have no confirmed information about where this segment\'s code actually lives. ' +
          'If you have just run this program in SINTRAN and BPAGL is still 0, that means the chain ' +
          'walk did not find any pages — which is itself unexpected and worth investigating.');
        if (iPanel) iPanel.innerHTML =
          '<div style="padding:12px;color:rgba(255,200,80,0.9);">' +
          '<b>Segment ' + oct6(seg.segNum) + ': all pages zero</b>' +
          '<ul style="margin-top:8px;line-height:1.6;font-size:12px;color:rgba(220,225,235,0.85);">' +
          lines.map(function(r) { return '<li>' + r + '</li>'; }).join('') +
          '</ul></div>';
        return;
      }
      renderITab();
    }).catch(function(err) {
      console.error('[disasm] read failed for seg ' + oct6(seg.segNum) + ':', err);
      if (iPanel) iPanel.innerHTML =
        '<div style="padding:12px;color:rgba(255,80,80,0.9);">' +
        '<b>Failed to load segment ' + oct6(seg.segNum) + '</b><br>' +
        '<div style="margin-top:8px;font-family:monospace;font-size:11px;background:rgba(0,0,0,0.3);padding:8px;border-left:3px solid rgba(255,80,80,0.6);white-space:pre-wrap;word-break:break-word;">' +
        escHtml(err.message || String(err)) +
        '</div>' +
        '<div style="margin-top:8px;font-size:10px;color:rgba(160,175,210,0.6);">' +
        'Filter browser console by <b>[disasm]</b> for full diagnostic trace.</div>' +
        '</div>';
    });
  }

  function loadDBank(segEntry) {
    var dPanel = document.getElementById('seg-disasm-d-panel');
    var dBtn   = document.getElementById('seg-disasm-tab-d');
    var name   = (typeof window.resolveSegmentName === 'function')
      ? window.resolveSegmentName(segEntry.segNum) : ('S' + segEntry.segNum.toString(8));

    showDTab('D Bank: ' + name);
    if (dPanel) { dPanel.style.display = ''; dPanel.innerHTML = '<div style="padding:12px;color:rgba(160,175,210,0.5);">Reading D bank…</div>'; }

    // Switch to D tab
    switchTab('d');

    readSegmentDataAsync(segEntry).then(function(words) {
      if (!words) {
        if (dPanel) dPanel.innerHTML = '<div style="padding:12px;color:rgba(255,80,80,0.8);">Failed to read D bank data.</div>';
        return;
      }
      dSeg   = segEntry;
      dWords = words;
      renderDTab();
    });
  }

  // =========================================================
  // I tab — disassembly (or hex dump when WASM unavailable)
  // =========================================================

  function renderITab() {
    var iPanel = document.getElementById('seg-disasm-i-panel');
    if (!iPanel || !iWords || !iSeg) return;

    var logicalBase = iSeg.logad << 10;

    // Find first non-zero word (for auto-scroll to actual code)
    var firstNZ = -1;
    for (var nz = 0; nz < iWords.length; nz++) {
      if (iWords[nz]) { firstNZ = nz; break; }
    }

    // Banner reporting observed zero gaps without interpretation
    var banner = '';
    if (firstNZ > 0) {
      var skipPages = (firstNZ / WORDS_PER_PAGE) | 0;
      banner = '<div style="padding:6px 10px;background:rgba(255,200,80,0.08);' +
        'border-bottom:1px solid rgba(255,200,80,0.2);font-size:11px;color:rgba(255,200,80,0.9);">' +
        'Pages 0–' + (skipPages - 1) + ' are all zeros. First non-zero word at offset ' +
        firstNZ + ' (relPage ' + skipPages + '). View scrolled there.' +
        '</div>';
    }

    if (emu.loadInspectBuffer && emu.disassembleFromBuffer &&
        typeof Module !== 'undefined' && Module._Dbg_LoadInspectBuffer) {
      var byteLen = iWords.length * 2;
      var ptr = Module._malloc(byteLen);
      if (ptr) {
        Module.HEAPU8.set(new Uint8Array(iWords.buffer), ptr);
        emu.loadInspectBuffer(ptr, iWords.length, logicalBase);
        Module._free(ptr);
        var result = emu.disassembleFromBuffer(0, iWords.length);
        iPanel.innerHTML = banner + parseDisasmText(result);
        scrollToFirstNonzero(iPanel, firstNZ);
        return;
      }
    }

    // Fallback: hex dump with ASCII column (works without WASM rebuild)
    iPanel.innerHTML = banner +
      '<div style="padding:4px 8px 6px;color:rgba(255,200,80,0.7);font-size:10px;border-bottom:1px solid rgba(255,255,255,0.06);">' +
      'WASM disassembler not available — showing hex dump with ASCII.</div>' +
      buildHexHtml(iWords, logicalBase);
    scrollToFirstNonzero(iPanel, firstNZ);
  }

  // Scroll the panel so the line containing word index `wordIdx` is visible at top.
  function scrollToFirstNonzero(panel, wordIdx) {
    if (wordIdx <= 0) return;
    // Both render paths produce one line per word (disasm) or per 16 words (hex).
    // Find the line whose first column contains the word's octal address and scroll to it.
    var logicalBase = iSeg.logad << 10;
    var targetAddr = logicalBase + wordIdx;
    setTimeout(function() {
      // For hex dump: lines are .disasm-hex-line. For disasm: .disasm-line.
      var lines = panel.querySelectorAll('.disasm-line, .disasm-hex-line');
      if (!lines.length) return;
      // Each line's first text segment is the address. For hex: 16 words per line;
      // for disasm: 1 word per line.
      var lineWordsPerLine = panel.querySelector('.disasm-hex-line') ? 16 : 1;
      var targetLine = (wordIdx / lineWordsPerLine) | 0;
      if (targetLine < lines.length) {
        lines[targetLine].scrollIntoView({ block: 'start', behavior: 'auto' });
      }
    }, 0);
  }

  function parseDisasmText(text) {
    var lines = text.split('\n');
    var html  = '';
    for (var i = 0; i < lines.length; i++) {
      var line  = lines[i];
      if (!line) continue;
      var parts = line.split(/\s+/);
      if (parts.length < 2) continue;
      html += '<div class="disasm-line">' +
        '<span class="disasm-addr">'     + parts[0] + '</span>' +
        ' <span class="disasm-word">'    + parts[1] + '</span>' +
        '  <span class="disasm-mnemonic">' + escHtml(parts.slice(2).join(' ')) + '</span>' +
        '</div>';
    }
    return html || '<div style="padding:8px;color:rgba(160,175,210,0.4);">No instructions</div>';
  }

  // =========================================================
  // D tab — hex dump of D bank segment
  // =========================================================

  function renderDTab() {
    var dPanel = document.getElementById('seg-disasm-d-panel');
    if (!dPanel || !dWords || !dSeg) return;
    dPanel.innerHTML = buildHexHtml(dWords, dSeg.logad << 10);
  }

  // =========================================================
  // Hex dump builder (used by both I fallback and D tab)
  // =========================================================

  function buildHexHtml(words, logicalBase) {
    var WORDS_PER_LINE = 16;
    var html = '';
    for (var i = 0; i < words.length; i += WORDS_PER_LINE) {
      var addr  = logicalBase + i;
      var line  = oct6(addr) + ': ';
      var ascii = '';
      for (var w = 0; w < WORDS_PER_LINE; w++) {
        var word = (i + w < words.length) ? words[i + w] : 0;
        line += oct6(word) + ' ';
        var hi = (word >> 8) & 0x7F;
        var lo = word & 0x7F;
        ascii += (hi >= 0x20 && hi < 0x7F) ? String.fromCharCode(hi) : '.';
        ascii += (lo >= 0x20 && lo < 0x7F) ? String.fromCharCode(lo) : '.';
      }
      html += '<div class="disasm-hex-line">' + escHtml(line) + '| ' + escHtml(ascii) + '</div>';
    }
    return html || '<div style="padding:8px;color:rgba(160,175,210,0.4);">No data</div>';
  }

  // =========================================================
  // Tab switching
  // =========================================================

  function switchTab(tab) {
    var iBtn = document.getElementById('seg-disasm-tab-i');
    var dBtn = document.getElementById('seg-disasm-tab-d');
    var iP   = document.getElementById('seg-disasm-i-panel');
    var dP   = document.getElementById('seg-disasm-d-panel');

    if (tab === 'i') {
      if (iBtn) iBtn.classList.add('active');
      if (dBtn) dBtn.classList.remove('active');
      if (iP)  iP.style.display = '';
      if (dP)  dP.style.display = 'none';
    } else {
      if (dBtn && dSeg) {
        if (iBtn) iBtn.classList.remove('active');
        dBtn.classList.add('active');
        if (iP)  iP.style.display = 'none';
        if (dP)  dP.style.display = '';
      }
    }
  }

  function initTabs() {
    var tabBar = document.querySelector('#seg-disasm-window .disasm-tab-bar');
    if (!tabBar) return;
    tabBar.addEventListener('click', function(e) {
      var btn = e.target.closest('.disasm-tab');
      if (!btn) return;
      switchTab(btn.getAttribute('data-tab'));
    });
  }

  // =========================================================
  // Keyboard navigation: arrow keys, page up/down, home/end, G to go to address
  // =========================================================
  function activePanel() {
    var iP = document.getElementById('seg-disasm-i-panel');
    var dP = document.getElementById('seg-disasm-d-panel');
    if (dP && dP.style.display !== 'none') return dP;
    return iP;
  }

  function lineHeight(panel) {
    var line = panel.querySelector('.disasm-line, .disasm-hex-line');
    return line ? line.getBoundingClientRect().height : 16;
  }

  function gotoAddress() {
    if (!iWords || !iSeg) { return; }
    var input = prompt(
      'Go to address (octal):\n' +
      'Segment range: 0o' + (iSeg.logad << 10).toString(8) +
      ' to 0o' + (((iSeg.logad << 10) + iWords.length - 1) | 0).toString(8)
    );
    if (!input) return;
    var s = input.trim().replace(/^0o/i, '');
    var addr = parseInt(s, 8);
    if (isNaN(addr)) { alert('Not a valid octal number: ' + input); return; }

    var logicalBase = iSeg.logad << 10;
    var offset = addr - logicalBase;
    if (offset < 0 || offset >= iWords.length) {
      alert('Address 0o' + addr.toString(8) + ' is outside segment range ' +
            '(0o' + logicalBase.toString(8) + '..0o' +
            (logicalBase + iWords.length - 1).toString(8) + ')');
      return;
    }
    var panel = activePanel();
    if (!panel) return;
    var lines = panel.querySelectorAll('.disasm-line, .disasm-hex-line');
    if (!lines.length) return;
    var lineWordsPerLine = panel.querySelector('.disasm-hex-line') ? 16 : 1;
    var targetLine = (offset / lineWordsPerLine) | 0;
    if (targetLine < lines.length) {
      lines[targetLine].scrollIntoView({ block: 'start', behavior: 'smooth' });
      // Briefly highlight the target line
      var el = lines[targetLine];
      var oldBg = el.style.background;
      el.style.background = 'rgba(255, 220, 80, 0.25)';
      setTimeout(function() { el.style.background = oldBg; }, 1500);
    }
  }

  function initKeyboard() {
    var win = document.getElementById('seg-disasm-window');
    if (!win) return;
    // Make the window focusable so it receives keyboard events
    if (!win.hasAttribute('tabindex')) win.setAttribute('tabindex', '0');

    win.addEventListener('keydown', function(e) {
      if (win.style.display === 'none') return;
      // Don't intercept while typing in an input/select
      var tag = (e.target && e.target.tagName) || '';
      if (tag === 'INPUT' || tag === 'SELECT' || tag === 'TEXTAREA') return;

      var panel = activePanel();
      if (!panel) return;
      var lh = lineHeight(panel);
      var ph = panel.clientHeight;

      switch (e.key) {
        case 'ArrowDown': panel.scrollTop += lh;            e.preventDefault(); break;
        case 'ArrowUp':   panel.scrollTop -= lh;            e.preventDefault(); break;
        case 'PageDown':  panel.scrollTop += ph - lh;       e.preventDefault(); break;
        case 'PageUp':    panel.scrollTop -= ph - lh;       e.preventDefault(); break;
        case 'Home':      panel.scrollTop = 0;              e.preventDefault(); break;
        case 'End':       panel.scrollTop = panel.scrollHeight; e.preventDefault(); break;
        case 'g':
        case 'G':         gotoAddress();                    e.preventDefault(); break;
      }
    });

    // Auto-focus the window when it becomes visible (so keys work without clicking first)
    var observer = new MutationObserver(function() {
      if (win.style.display !== 'none') win.focus();
    });
    observer.observe(win, { attributes: true, attributeFilter: ['style'] });
  }

  // =========================================================
  // Save PROG + MD
  // =========================================================

  function saveFiles() {
    if (!iWords || !iSeg) { alert('No segment loaded'); return; }

    var name     = iName.replace(/[^A-Za-z0-9_\-]/g, '_');
    var iBase    = iSeg.logad << 10;
    var iCount   = iWords.length;
    var hasSplit = (dSeg !== null && dWords !== null);

    // ---- Build PROG file ----
    // Single-bank header: 256 words
    //   [0] startAddr  [1] restartAddr  [2] firstAddr  [3] lastAddr
    //   [4] 0xFFFF (no Bank2)  [5] 0x0000  [6..255] = 0
    // Followed by data blocks (big-endian uint16)

    var headerWords  = 256;
    var iDataBlocks  = Math.ceil(iCount / 256);
    var dBase        = hasSplit ? (dSeg.logad << 10) : 0;
    var dCount       = hasSplit ? dWords.length : 0;
    var dDataBlocks  = hasSplit ? Math.ceil(dCount / 256) : 0;

    // Dual-bank layout: 0x0000 = Bank1 header, 0x20000 = Bank2 header (pad to 128KW offset)
    var totalWords;
    if (hasSplit) {
      totalWords = 0x10000 + headerWords + dDataBlocks * 256;  // Bank2 at word offset 0x10000
    } else {
      totalWords = headerWords + iDataBlocks * 256;
    }

    var progBuf  = new ArrayBuffer(totalWords * 2);
    var progView = new DataView(progBuf);

    // Bank1 header at byte 0
    progView.setUint16(0,  iStart    & 0xFFFF, false);
    progView.setUint16(2,  iRestart  & 0xFFFF, false);
    progView.setUint16(4,  iBase     & 0xFFFF, false);
    progView.setUint16(6,  (iBase + iCount - 1) & 0xFFFF, false);
    if (hasSplit) {
      progView.setUint16(8,  dBase & 0xFFFF, false);        // Bank2 first addr
      progView.setUint16(10, (dBase + dCount - 1) & 0xFFFF, false); // Bank2 last addr
    } else {
      progView.setUint16(8,  0xFFFF, false);  // Bank2 absent
      progView.setUint16(10, 0x0000, false);
    }

    // Bank1 data
    for (var i = 0; i < iCount; i++) {
      progView.setUint16((headerWords + i) * 2, iWords[i], false);
    }

    // Bank2 header + data (at word offset 0x10000 = byte offset 0x20000)
    if (hasSplit) {
      var b2Start = 0x10000 * 2;
      progView.setUint16(b2Start + 0, iStart   & 0xFFFF, false);
      progView.setUint16(b2Start + 2, iRestart & 0xFFFF, false);
      progView.setUint16(b2Start + 4, dBase    & 0xFFFF, false);
      progView.setUint16(b2Start + 6, (dBase + dCount - 1) & 0xFFFF, false);
      progView.setUint16(b2Start + 8, 0xFFFF, false);
      for (var di = 0; di < dCount; di++) {
        progView.setUint16(b2Start + (headerWords + di) * 2, dWords[di], false);
      }
    }

    triggerDownload(new Blob([progBuf], { type: 'application/octet-stream' }), name + '.PROG');

    // ---- Build BPUN file (single-bank only) ----
    if (!hasSplit) {
      var bpunBytes = buildBPUNBytes(iWords, iBase, iStart, iStart);
      triggerDownload(new Blob([bpunBytes], { type: 'application/octet-stream' }), name + '.bpun');
    }

    // ---- Build MD file ----
    var timestamp = new Date().toISOString();
    var segfilN   = (iSeg.flag >> 13) & 0x7;
    var cls       = classifySegment(iSeg.sgsta || 0);

    var md = '# Segment ' + iName + '\n\n';
    md += '**Segment:** ' + iSeg.segNum.toString(8) + ' (octal)';
    md += ' | **Type:** ' + cls.label;
    md += ' | **SEGFIL:** ' + segfilN;
    md += ' | **MADR:** ' + oct6(iSeg.madr);
    md += ' | **Pages:** ' + (iSeg.segle & 0x3FF);
    if (hasSplit) {
      md += '\n**D Bank:** ' + dSeg.segNum.toString(8) + ' (octal)';
      md += ' | **MADR:** ' + oct6(dSeg.madr);
      md += ' | **Pages:** ' + (dSeg.segle & 0x3FF);
    }
    md += '\n**Generated:** ' + timestamp + '\n\n';

    md += buildMarkdownBody(hasSplit);

    triggerDownload(new Blob([md], { type: 'text/markdown; charset=utf-8' }), name + '-disasm.md');
  }

  function disasmLineText(el) {
    var a = el.querySelector('.disasm-addr');
    var w = el.querySelector('.disasm-word');
    var m = el.querySelector('.disasm-mnemonic');
    if (a && w && m) {
      return a.textContent + '  ' + w.textContent + '  ' + m.textContent;
    }
    return el.textContent;
  }

  function buildMarkdownBody(hasSplit) {
    var text = '## Disassembly\n\n```\n';
    var iPanel = document.getElementById('seg-disasm-i-panel');
    if (iPanel) {
      var iLines = iPanel.querySelectorAll('.disasm-line');
      for (var li = 0; li < iLines.length; li++) { text += disasmLineText(iLines[li]) + '\n'; }
      var hLines = iPanel.querySelectorAll('.disasm-hex-line');
      for (var hi = 0; hi < hLines.length; hi++) { text += hLines[hi].textContent + '\n'; }
    }
    text += '```\n';

    if (hasSplit) {
      text += '\n## D Bank\n\n```\n';
      var dPanel = document.getElementById('seg-disasm-d-panel');
      if (dPanel) {
        var dLines = dPanel.querySelectorAll('.disasm-hex-line');
        for (var dl = 0; dl < dLines.length; dl++) { text += dLines[dl].textContent + '\n'; }
      }
      text += '```\n';
    }
    return text;
  }

  function copyToClipboard() {
    if (!iWords || !iSeg) { return; }
    var hasSplit  = (dSeg !== null && dWords !== null);
    var segfilN   = (iSeg.flag >> 13) & 0x7;
    var cls       = classifySegment(iSeg.sgsta || 0);
    var timestamp = new Date().toISOString();

    var md = '# Segment ' + iName + '\n\n';
    md += '**Segment:** ' + iSeg.segNum.toString(8) + ' (octal)';
    md += ' | **Type:** ' + cls.label;
    md += ' | **SEGFIL:** ' + segfilN;
    md += ' | **MADR:** ' + oct6(iSeg.madr);
    md += ' | **Pages:** ' + (iSeg.segle & 0x3FF);
    if (hasSplit) {
      md += '\n**D Bank:** ' + dSeg.segNum.toString(8) + ' (octal)';
      md += ' | **MADR:** ' + oct6(dSeg.madr);
      md += ' | **Pages:** ' + (dSeg.segle & 0x3FF);
    }
    md += '\n**Generated:** ' + timestamp + '\n\n';
    md += buildMarkdownBody(hasSplit);

    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(md).then(function() {
        var btn = document.getElementById('seg-disasm-copy');
        if (btn) { var orig = btn.title; btn.title = 'Copied!'; setTimeout(function(){ btn.title = orig; }, 1500); }
      });
    } else {
      var ta = document.createElement('textarea');
      ta.value = md;
      ta.style.position = 'fixed'; ta.style.left = '-9999px';
      document.body.appendChild(ta); ta.select(); document.execCommand('copy');
      document.body.removeChild(ta);
    }
  }

  // =========================================================
  // Public entry points
  // =========================================================

  // Called from segment table detail pane
  window.segDisasmOpenWindow = function(segNum) {
    var segs = (typeof window.getLastSegments === 'function') ? window.getLastSegments() : [];
    var seg  = null;
    for (var i = 0; i < segs.length; i++) {
      if (segs[i].segNum === segNum) { seg = segs[i]; break; }
    }
    if (!seg && typeof window.getReentrantCachedSegment === 'function') {
      seg = window.getReentrantCachedSegment(segNum);
    }
    if (!seg) { alert('Segment ' + segNum.toString(8) + ' not found'); return; }

    var segName = (typeof window.resolveSegmentName === 'function')
      ? window.resolveSegmentName(segNum) : '';
    openDisasmWindow(seg, segName || ('S' + segNum.toString(8)), 0, 0);
  };

  // Called from reentrant programs detail pane
  window.segDisasmOpenWindowForReentrant = function(name, segno, stadr, restart) {
    var segs = (typeof window.getLastSegments === 'function') ? window.getLastSegments() : [];
    var seg  = null;
    for (var i = 0; i < segs.length; i++) {
      if (segs[i].segNum === segno) { seg = segs[i]; break; }
    }
    if (!seg && typeof window.getReentrantCachedSegment === 'function') {
      seg = window.getReentrantCachedSegment(segno);
    }
    if (!seg) {
      // Build minimal stub from what we know
      seg = { segNum: segno, segno: segno, segli: 0, prese: 0, logad: 0, segle: 0,
              madr: 0, flag: 0, sgsta: 0, bpagl: 0, usedBy: [] };
    }
    openDisasmWindow(seg, name, stadr || 0, restart || 0);
  };

  // =========================================================
  // Helpers
  // =========================================================

  function oct6(n) { return (n >>> 0).toString(8).padStart(6, '0'); }

  function escHtml(s) {
    return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

  // Build a BPUN binary tape image from a Uint16Array.
  // Format: ASCII preamble "{startOctal}/{bootOctal}!" followed by binary
  // [loadAddr 2B][wordCount 2B][words N*2B big-endian][checksum 2B][action 2B]
  function buildBPUNBytes(words, loadAddr, startAddr, bootAddr) {
    var preamble = (startAddr >>> 0).toString(8) + '/' + (bootAddr >>> 0).toString(8) + '!';
    var preambleLen = preamble.length;
    var wordCount = words.length;
    var checksum = 0;
    for (var i = 0; i < wordCount; i++) checksum = (checksum + words[i]) & 0xFFFF;

    var buf = new Uint8Array(preambleLen + 2 + 2 + wordCount * 2 + 2 + 2);
    var pos = 0;
    for (var c = 0; c < preambleLen; c++) buf[pos++] = preamble.charCodeAt(c) & 0x7F;
    buf[pos++] = (loadAddr >> 8) & 0xFF;
    buf[pos++] = loadAddr & 0xFF;
    buf[pos++] = (wordCount >> 8) & 0xFF;
    buf[pos++] = wordCount & 0xFF;
    for (var w = 0; w < wordCount; w++) {
      buf[pos++] = (words[w] >> 8) & 0xFF;
      buf[pos++] = words[w] & 0xFF;
    }
    buf[pos++] = (checksum >> 8) & 0xFF;
    buf[pos++] = checksum & 0xFF;
    buf[pos++] = 0; buf[pos++] = 0;  // action = 0
    return buf;
  }

  function triggerDownload(blob, filename) {
    var url = URL.createObjectURL(blob);
    var a   = document.createElement('a');
    a.href = url; a.download = filename;
    document.body.appendChild(a); a.click(); document.body.removeChild(a);
    setTimeout(function() { URL.revokeObjectURL(url); }, 1000);
  }

  // =========================================================
  // Event handlers + init
  // =========================================================

  var closeBtn = document.getElementById('seg-disasm-close');
  if (closeBtn) closeBtn.addEventListener('click', function() {
    var win = document.getElementById('seg-disasm-window');
    if (win) win.style.display = 'none';
  });

  var saveBtn = document.getElementById('seg-disasm-save');
  if (saveBtn) saveBtn.addEventListener('click', saveFiles);

  var copyBtn = document.getElementById('seg-disasm-copy');
  if (copyBtn) copyBtn.addEventListener('click', copyToClipboard);

  // Source selector — reload current segment when source changes
  var srcSelect = document.getElementById('seg-disasm-source');
  if (srcSelect) {
    srcSelect.addEventListener('change', function() {
      iSource = srcSelect.value;
      if (iSeg) {
        // Re-open with same segment but new source
        openDisasmWindow(iSeg, iName, iStart, iRestart);
      }
    });
  }

  var loadDBtn = document.getElementById('seg-disasm-load-d');
  if (loadDBtn) loadDBtn.addEventListener('click', function() {
    var input = prompt(
      'Load D Bank — enter companion data segment number (octal):\n' +
      '(The code segment is the I bank; the data segment is the D bank.)\n' +
      'Example: if NPL is seg 134 and its data is seg 135, enter 135'
    );
    if (!input) return;
    var segNum = parseInt(input.trim(), 8);
    if (isNaN(segNum) || segNum < 0) { alert('Invalid segment number'); return; }
    var segs = (typeof window.getLastSegments === 'function') ? window.getLastSegments() : [];
    var seg  = null;
    for (var i = 0; i < segs.length; i++) {
      if (segs[i].segNum === segNum) { seg = segs[i]; break; }
    }
    if (!seg && typeof window.getReentrantCachedSegment === 'function') {
      seg = window.getReentrantCachedSegment(segNum);
    }
    if (!seg) { alert('Segment ' + segNum.toString(8) + ' not found — open Segment Table first'); return; }
    loadDBank(seg);
  });

  initTabs();
  initKeyboard();

})();
