#!/usr/bin/env python3
"""
Run the real TPE "INSTRUCTION" diagnostic on nd100x as an ND-120/CX and check the
identity header it reports. Demonstrates the nd100x_expect driver end to end.

Usage:
    python tpe_instruction_nd120.py [path-to-TPE-floppy.img]

If no path is given it uses the RetroCore test floppy, if present. Boots the TPE
floppy under --cputype=ND120CX, types INSTRUCTION then run, and asserts TPE prints
"ND-120/CX" / "3202" / "100014B". Exit code 0 = pass.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from nd100x_expect import Nd100x, ExpectTimeout, ExpectAbort   # noqa: E402

# The image ships in this repository, so the fallback is a path derived from
# this file's own location rather than a drive letter on one machine: the
# default now works on every checkout instead of silently skipping the test.
# $ND100X_TPE_FLOPPY still wins when the image lives somewhere else.
_REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_REPO_TPE = os.path.join(_REPO_ROOT, "images", "Nd-210523I01-XX-01D.img")
DEFAULT_TPE = _REPO_TPE


def main():
    # Floppy path: argv[1] (if non-empty), else $ND100X_TPE_FLOPPY, else the copy in images/.
    # (CMake may pass an empty argv[1] when -DNDX_TPE_FLOPPY is unset - treat that as "not given".)
    tpe = (sys.argv[1] if (len(sys.argv) > 1 and sys.argv[1])
           else os.environ.get("ND100X_TPE_FLOPPY", DEFAULT_TPE))
    if not tpe or not os.path.exists(tpe):
        # Exit 2 = "skip" (CMake add_test SKIP_RETURN_CODE 2): no image, not a failure.
        print("TPE floppy image not found (%s) - skipping." % tpe)
        return 2

    with Nd100x(boot="floppy", image=tpe, cputype="ND120CX", echo=True) as vm:
        # 1) wait for the TPE monitor prompt
        vm.expect("TPE>", timeout=60, abort=["HALT", "malfunction"])
        # 2) load + run the INSTRUCTION test
        vm.send("INSTRUCTION\r")
        vm.expect("Version", timeout=20)
        vm.send("run\r")
        # 3) the identity header appears near the top of the run; assert each field.
        #    Patterns are LITERAL substrings (see nd100x_expect.Regex for regex).
        #    Abort fast if TPE flags any instruction error.
        vm.expect("ND-120/CX", timeout=60, abort="*** ERROR ***")
        vm.expect("3202", timeout=10)
        vm.expect("100014B", timeout=10)

    print("\n\nPASS: nd100x reported ND-120/CX, print 3202, microprogram 100014B")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ExpectTimeout, ExpectAbort) as e:
        print("\n\nFAIL: %s" % e)
        sys.exit(1)
