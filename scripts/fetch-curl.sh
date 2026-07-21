#!/bin/sh
# fetch-curl.sh — download a prebuilt MinGW libcurl into external/curl/
#
# Mirrors scripts/fetch-sdl2.sh in the tiki100 project: vendor a prebuilt MinGW
# devel release so a LOCAL Windows build (w64devkit) links real libcurl without
# needing MSYS2's pacman. POSIX sh only (runs under w64devkit's busybox ash).
#
# Only needed for LOCAL Windows builds. On Linux/macOS the build finds a system
# libcurl (pkg-config / system framework), so this script is not used there.
#
# The package is Viktor Szakats' "curl for Windows" (https://curl.se/windows/):
# a self-contained mingw build with bin/libcurl-x64.dll, lib/libcurl.dll.a,
# include/curl/*.h and a curl-ca-bundle.crt (its TLS backend verifies against
# that CA file — download.c points CURLOPT_CAINFO at it on Windows).
#
# Run from the repo root:
#   make fetch-curl                          # pinned version below
#   CURL_WIN_VERSION=8.21.0_4 make fetch-curl # a specific dl-<ver> build

set -eu

# Pinned to a known-good curl-for-win build (dl-<version> / build suffix _N).
# Bump this to move forward; the extracted folder is curl-<version>-win64-mingw.
VERSION="${CURL_WIN_VERSION:-8.21.0_4}"
URL="https://curl.se/windows/dl-${VERSION}/curl-${VERSION}-win64-mingw.zip"

# Verify we are at the repo root.
if [ ! -f CMakeLists.txt ] || [ ! -d src ]; then
    echo "Error: run this via 'make fetch-curl' from the repository root." >&2
    exit 1
fi

EXTERNAL="external"
TARGET="$EXTERNAL/curl"
ZIP="$EXTERNAL/curl-${VERSION}-win64-mingw.zip"

mkdir -p "$EXTERNAL"

if [ -d "$TARGET" ]; then
    echo "external/curl already exists."
    echo "Delete it first if you want to re-fetch:  rm -rf external/curl"
    exit 0
fi

echo "Downloading libcurl ${VERSION} (win64 mingw) from:"
echo "  $URL"

if command -v curl >/dev/null 2>&1; then
    curl -L --fail -o "$ZIP" "$URL"
elif command -v wget >/dev/null 2>&1; then
    wget -O "$ZIP" "$URL"
else
    echo "Error: neither curl nor wget is available to download the package." >&2
    exit 1
fi

echo "Extracting..."
(cd "$EXTERNAL" && unzip -q "curl-${VERSION}-win64-mingw.zip")
mv "$EXTERNAL/curl-${VERSION}-win64-mingw" "$TARGET"
rm "$ZIP"

echo
echo "libcurl ${VERSION} ready at: $TARGET"
ls "$TARGET" 2>/dev/null || true
