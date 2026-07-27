#!/usr/bin/env python3
"""
Instruction-level test for the optional 32-bit FPP in nd100x (handoff item 6.8).

Generates a minimal BPUN program and runs the real emulator with --trace:

    0400  171077  SAT 077        ; T = 000077 sentinel
    0401  170403  SAA 3          ; A = 3
    0402  151420  NLZ 20         ; A,D = float(3)   (T untouched in 32-bit mode)
    0403  100005  FAD * + 5      ; add the float at 0410
    0404  124000  JMP *          ; spin until -n stops the CPU
    0405  124000
    0406  000000
    0407  000000
    0410  040100  ; memory operand word A (1.0, manual/fixed-oracle encoding)
    0411  000000  ; memory operand word D
    0412  POISON  ; must NOT be read by the 2-word 32-bit operand fetch

Assertions:
  32-bit mode (--fpp=32):
    - T still 000077 after NLZ and after FAD (detection rule),
    - FAD result A:D identical for two different POISON values
      (proves the operand fetch reads exactly TWO words),
    - result is float(3) + float(1) = float(4) = 040300:000000.
  48-bit mode (default, no --fpp):
    - T changes after NLZ (equals 040002, the 48-bit float(3) exponent word),
    - FAD result DIFFERS between the two POISON values
      (positive control: the 48-bit fetch reads THREE words).

Exit codes: 0 = pass, 1 = fail, 2 = emulator binary not found (-> ctest SKIP).
"""

import re
import subprocess
import sys
import tempfile
import os

SENTINEL_T = 0o000077


def build_bpun(path, poison):
    words = [
        0o171077,   # SAT 077
        0o170403,   # SAA 3
        0o151420,   # NLZ 20
        0o100005,   # FAD P+5 -> 0410
        0o124000,   # JMP *
        0o124000,   # JMP *
        0o000000,
        0o000000,
        0o040100,   # mem float word A (1.0)
        0o000000,   # mem float word D
        poison,     # poison word at EA+2
    ]
    data = bytearray()
    data += b"400/!"                       # preamble: start/boot = 0400
    data += bytes([0x01, 0x00])            # load address 0400 (octal) = 0x0100
    data += len(words).to_bytes(2, "big")  # word count
    checksum = 0
    for w in words:
        data += w.to_bytes(2, "big")
        checksum = (checksum + w) & 0xFFFF
    data += checksum.to_bytes(2, "big")
    data += bytes([0x00, 0x01])            # action != 0
    with open(path, "wb") as f:
        f.write(data)


TRACE_RE = re.compile(
    r"^(?P<pc>[0-7]{6}) [0-7]{6} .*A=(?P<a>[0-7]{6}) D=(?P<d>[0-7]{6}) "
    r"T=(?P<t>[0-7]{6}) ")


def run_and_trace(nd100x, bpun, fpp32):
    cmd = [nd100x, "--boot=bpun", "--image=" + bpun, "--trace", "-n", "25"]
    if fpp32:
        cmd.append("--fpp=32")
    proc = subprocess.run(cmd, stdin=subprocess.DEVNULL,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          timeout=60, text=True)
    regs = {}   # pc -> (A, D, T) at FETCH time of the instruction at pc
    for line in proc.stderr.splitlines():
        m = TRACE_RE.match(line)
        if m:
            pc = int(m.group("pc"), 8)
            if pc not in regs:   # keep the FIRST fetch at each pc
                regs[pc] = (int(m.group("a"), 8),
                            int(m.group("d"), 8),
                            int(m.group("t"), 8))
    return regs


failures = []


def check(name, expected, got):
    if expected != got:
        failures.append("%s: expected %06o, got %06o" % (name, expected, got))


def main():
    if len(sys.argv) < 2:
        print("usage: test_fpp32_instr.py <path-to-nd100x>")
        return 2
    nd100x = sys.argv[1]
    if not os.path.isfile(nd100x):
        print("nd100x binary not found: %s -> SKIP" % nd100x)
        return 2

    with tempfile.TemporaryDirectory() as tmp:
        results32 = []
        for poison in (0o123456, 0o054321):
            bpun = os.path.join(tmp, "fpp32_%06o.bpun" % poison)
            build_bpun(bpun, poison)

            regs = run_and_trace(nd100x, bpun, fpp32=True)
            if 0o404 not in regs or 0o403 not in regs:
                failures.append("32-bit run (poison %06o): trace lines for "
                                "0403/0404 not found - program did not run"
                                % poison)
                continue

            a, d, t = regs[0o403]          # state after NLZ 20
            check("32-bit: T after NLZ", SENTINEL_T, t)
            a, d, t = regs[0o404]          # state after FAD
            check("32-bit: T after FAD", SENTINEL_T, t)
            check("32-bit: A after FAD", 0o040300, a)
            check("32-bit: D after FAD", 0o000000, d)
            results32.append((a, d))

        if len(results32) == 2 and results32[0] != results32[1]:
            failures.append("32-bit: FAD result depends on the word at EA+2 "
                            "(three words were fetched, expected two): "
                            "%r vs %r" % (results32[0], results32[1]))

        results48 = []
        for poison in (0o123456, 0o054321):
            bpun = os.path.join(tmp, "fpp48_%06o.bpun" % poison)
            build_bpun(bpun, poison)

            regs = run_and_trace(nd100x, bpun, fpp32=False)
            if 0o404 not in regs or 0o403 not in regs:
                failures.append("48-bit run (poison %06o): trace lines for "
                                "0403/0404 not found - program did not run"
                                % poison)
                continue

            a, d, t = regs[0o403]
            check("48-bit: T after NLZ (float(3) exponent)", 0o040002, t)
            a, d, t = regs[0o404]
            results48.append((a, d, t))

        if len(results48) == 2 and results48[0] == results48[1]:
            failures.append("48-bit: FAD result ignored the word at EA+2 "
                            "(two words were fetched, expected three)")

    if failures:
        print("FAIL (%d):" % len(failures))
        for f in failures:
            print("  " + f)
        return 1
    print("fpp32 instruction-level tests: all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
