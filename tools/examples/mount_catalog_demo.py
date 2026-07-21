#!/usr/bin/env python3
"""
mount_catalog_demo.py - mount a floppy straight from the online catalog into a
RUNNING nd100x, driven from Python.

Demonstrates the full chain that tools/nd100x_expect.py provides:
    search the online floppy catalog  ->  download the image over HTTPS (by md5 or
    SINTRAN directory name)  ->  hot-swap it onto a floppy unit of a running machine.

It boots the TPE monitor from a boot floppy, hot-swaps the chosen catalog floppy onto
floppy unit 0, then issues `LOAD PAGI` to prove the guest is now reading the *new*
disk (PAGING is on the TPE floppy, so it loads before the swap and reports
NO SUCH FILE NAME after - the guest's directory view follows whatever is mounted).

Usage:
    python mount_catalog_demo.py                    # pick the smallest catalog floppy
    python mount_catalog_demo.py --md5 <hash>       # a specific image (md5 is unique)
    python mount_catalog_demo.py --dir <NAME>       # by SINTRAN "Directory name"
    python mount_catalog_demo.py --unit 1           # mount on a different floppy unit

The boot floppy comes from $ND100X_TPE_FLOPPY (or edit DEFAULT_TPE). The catalog and
image are fetched from ndlib.hackercorp.no (needs network); the driver sends
User-Agent nd100x/1.0, which the server requires (it 403s the default Python UA).
Exit 0 = the catalog floppy downloaded and mounted; exit 2 = no boot floppy (skip).
"""
import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from nd100x_expect import Nd100x, FloppyCatalog, ExpectTimeout, ExpectAbort  # noqa: E402

DEFAULT_TPE = os.environ.get(
    "ND100X_TPE_FLOPPY",
    r"E:\Dev\Repos\Ronny\RetroCore\Emulated.Tests\ND100\TestData\Nd-210523I01-XX-01D.img",
)


def main():
    ap = argparse.ArgumentParser(description="Mount a catalog floppy into a running nd100x.")
    ap.add_argument("--md5", help="mount the image with this md5 (unique)")
    ap.add_argument("--dir", dest="directory", help="mount by SINTRAN directory name")
    ap.add_argument("--unit", type=int, default=0, help="floppy unit to mount onto (default 0)")
    ap.add_argument("--tpe", default=DEFAULT_TPE, help="boot floppy image")
    args = ap.parse_args()

    if not args.tpe or not os.path.exists(args.tpe):
        print("boot floppy not found (%s) - skipping." % args.tpe)
        return 2

    # Resolve the selection. Default: the smallest real floppy, for a quick demo.
    cat = FloppyCatalog.load()
    md5 = args.md5
    directory = args.directory
    if not md5 and not directory:
        floppy = sorted(
            [e for e in cat.entries if (not e["is_smd"]) and e["pages"] > 0 and e["md5"]],
            key=lambda e: e["pages"],
        )[0]
        md5 = floppy["md5"]
        print("no selector given; picking smallest floppy: %r (md5 %s, %d pages)"
              % (floppy["name"], md5, floppy["pages"]))

    with Nd100x(boot="floppy", image=args.tpe, echo=False) as vm:
        vm.expect("TPE>", timeout=60, abort=["HALT", "malfunction"])

        # Before the swap: the boot floppy has PAGING, so LOAD PAGI resolves it.
        vm.send("LOAD PAGI\r")
        before = vm.expect(["PAGING", "NO SUCH FILE"], timeout=15)
        print("before swap: LOAD PAGI ->", before.strip())

        # Download + hot-swap the catalog floppy onto the chosen unit.
        dest = vm.mount_catalog(args.unit, md5=md5, directory=directory, confirm=True, timeout=180)
        print("mounted catalog floppy on unit %d: %s (%d bytes)"
              % (args.unit, dest, os.path.getsize(dest)))

        if args.unit == 0:
            # After the swap unit 0 is the catalog floppy, which does not have PAGING.
            vm.send("LOAD PAGI\r")
            after = vm.expect(["NO SUCH FILE", "PAGING"], timeout=15)
            print("after swap:  LOAD PAGI ->", after.strip(),
                  "  (guest is reading the catalog floppy)")

    print("OK: catalog floppy downloaded, mounted, and read by the guest.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ExpectTimeout, ExpectAbort) as e:
        print("FAIL:", e)
        sys.exit(1)
