#!/usr/bin/env python3
"""
tpe_autorun.py - unattended runner for the ND TPE diagnostic floppy.

Lives in tools/ at the repo root; derives the repo location from its own path.

Boots build/bin/nd100x from a TPE test floppy, then for each selected test
program: loads it, optionally sends a SET-PARAMETERS line, sends RUN, captures
the console output for a fixed time-box, scans for error markers, and moves on.

One fresh emulator boot PER test so a hung/looping test cannot poison the next.

Verified behaviour (23-JUL-2026, nd100x @ 763a4f4):
  - Typing a test file name at "TPE>" loads it and returns to the "TPE>" prompt
    with that program's commands registered (abbreviations work, e.g. "instr").
  - "run" is the common convention to execute a loaded program's tests.
    INSTRUCTION-C03 and MEMORY-D04 both start testing on "run".
  - INSTRUCTION-C03 needs a SET-PARAMETERS line first to select the full suite,
    e.g.  set-para,N,N,Y,N,Y   then  run   -> tests levels 1..9, "=== End of run ==="
  - Some programs REFUSE without real hardware/switches (e.g. CACHE prints
    "*** The CACHE MANUAL DISABLE switch is ON ***"); device tests need the
    emulated device present and usually interactive parameters.

Usage (from the repo root):
  python3 tools/tpe_autorun.py \
      --image FLOPPY.IMG \
      --cputype ND110CX --outdir /tmp/tpe_logs

  # run only specific programs:
  python3 tools/tpe_autorun.py --only INSTRUCTION-C03,MEMORY-D04,PAGING-C02
"""

import argparse, os, sys, time, pty, select, re

# Repo root = parent of the tools/ directory this script lives in.
REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN  = os.path.join(REPO, "build/bin/nd100x")

# Per-program recipe: name -> (setparams_or_None, run_cmd, timebox_seconds, post_replies)
#   setparams   : full command line typed at TPE> BEFORE run (None = skip)
#   run_cmd     : command that starts the tests (usually "run")
#   timebox     : seconds to let the tests run before moving on
#   post_replies: replies sent, in order, to any non-"TPE>" parameter prompt that
#                 ends in ":" AFTER run_cmd (e.g. PAGING asks "Test number(s):")
# These are the CPU/memory/MMS-class tests that run without real peripherals.
RECIPES = {
    "INSTRUCTION-C03": ("set-para,N,N,Y,N,Y", "run", 60, []),
    "MEMORY-D04":      (None,                 "run", 60, []),
    "PAGING-C02":      (None,                 "run", 90, ["1-11"]),
    "CONFIGURATIO-D05":(None,                 "run", 20, []),
}

# Substrings that mean "this program refused / cannot run unattended" (skip, not fail).
REFUSE = [
    "MANUAL DISABLE switch is ON",
    "No such command",
    "NO SUCH FILE NAME",
]
# Substrings that mean a genuine test failure.
FAIL = ["*** ERROR", "No interrupt generated", "FATAL", "FAILED", "MISMATCH"]


def drive(image, cputype, program, setparams, run_cmd, timebox, post_replies):
    """Boot, load `program`, optional setparams, run_cmd, answer post prompts, capture."""
    pid, fd = pty.fork()
    if pid == 0:
        os.environ["TERM"] = "vt100"
        os.execvp(BIN, [BIN, "--boot=floppy", f"--image={image}",
                        "--pipe", f"--cputype={cputype}"])
        os._exit(1)

    out = bytearray()
    start = time.time()
    kicked = False
    steps = [s for s in (program, setparams, run_cmd) if s]
    si = 0
    last_send = 0.0
    run_started_at = None
    replies = list(post_replies)
    ri = 0
    hard_deadline = 25 + timebox   # boot budget + per-test time-box

    while time.time() - start < hard_deadline:
        r, _, _ = select.select([fd], [], [], 0.3)
        if r:
            try:
                c = os.read(fd, 4096)
            except OSError:
                break
            if not c:
                break
            out += c
        now = time.time()
        text = out.decode("latin1", "replace")
        tail = text.rstrip()

        if not kicked and now - start > 3:
            os.write(fd, b"\r"); kicked = True; last_send = now; continue

        # Feed the next TPE> step when the console shows an idle TPE> prompt.
        if kicked and si < len(steps):
            if tail.endswith("TPE>") and now - last_send > 1.5:
                os.write(fd, steps[si].encode() + b"\r")
                if steps[si] == run_cmd:
                    run_started_at = now
                si += 1; last_send = now
                continue

        # After run_cmd, answer parameter prompts (lines ending in ":") in order.
        if run_started_at and ri < len(replies):
            if tail.endswith(":") and now - last_send > 1.0:
                os.write(fd, replies[ri].encode() + b"\r")
                ri += 1; last_send = now
                continue

        # Stop early on a natural finish, or a fresh prompt once all replies are sent.
        if run_started_at and now - run_started_at > 3 and ri >= len(replies):
            if "End of run" in text or "END OF RUN" in text:
                break
            if tail.endswith("TPE>"):
                break

    try:
        os.kill(pid, 15)
    except Exception:
        pass
    return out.decode("latin1", "replace")


def classify(text):
    for m in REFUSE:
        if m in text:
            return "SKIP", m
    for m in FAIL:
        if m in text:
            return "FAIL", m
    if "End of run" in text or "END OF RUN" in text or "End of test" in text or "END OF TEST" in text:
        return "PASS", ""
    return "INCONCLUSIVE", ""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", default=os.path.join(REPO, "FLOPPY.IMG"))
    ap.add_argument("--cputype", default="ND110CX")
    ap.add_argument("--outdir", default="/tmp/tpe_logs")
    ap.add_argument("--only", default="", help="comma list of program names")
    args = ap.parse_args()

    os.chdir(REPO)
    os.makedirs(args.outdir, exist_ok=True)

    names = list(RECIPES.keys())
    if args.only:
        names = [n.strip() for n in args.only.split(",") if n.strip()]

    print(f"# TPE auto-run  image={args.image}  cpu={args.cputype}")
    summary = []
    for name in names:
        setp, runc, tb, post = RECIPES.get(name, (None, "run", 40, []))
        print(f"\n=== {name} (set-para={setp!r}) ===", flush=True)
        text = drive(args.image, args.cputype, name, setp, runc, tb, post)
        verdict, why = classify(text)
        logf = os.path.join(args.outdir, f"{name}.log")
        with open(logf, "w") as fh:
            fh.write(text)
        print(f"  -> {verdict}  {why}   log: {logf}", flush=True)
        summary.append((name, verdict, why))

    print("\n===== SUMMARY =====")
    for name, verdict, why in summary:
        print(f"  {verdict:13} {name}  {why}")
    fails = [s for s in summary if s[1] == "FAIL"]
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
