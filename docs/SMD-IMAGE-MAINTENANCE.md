# SMD Disk Image Server Maintenance

How to add, update, and remove SMD disk images from the server catalog so users can copy them into their local browser library.

---

## Two Serving Modes

The emulator has two ways of serving disk images to the browser:

| Mode | Server | Catalog Source | How to start |
|------|--------|---------------|--------------|
| **Static** | Python / any HTTP server | `template-glass/smd-catalog.json` (static file, copied to build) | `make wasm-glass-run` |
| **Gateway** | Node.js gateway server | Generated at request time from `tools/nd100-gateway/gateway.conf.json` | `make wasm-glass-gateway` |

Both modes serve the same browser UI. The difference is where the catalog JSON comes from and how image files are streamed to the browser.

---

## Gateway Mode (Recommended for Development)

### Config File

**Path:** `tools/nd100-gateway/gateway.conf.json`

The gateway reads this file on startup. The `smd.images` array defines the catalog:

```json
{
  "smd": {
    "images": [
      {
        "path": "../../SMD0.IMG",
        "name": "SINTRAN K",
        "description": "SINTRAN III/VSE version K, standard ND-100 system disk"
      },
      {
        "path": "../../SMD1.IMG",
        "name": "SMD Disk 1",
        "description": "Secondary SMD disk (unit 1)"
      }
    ]
  }
}
```

### Field Reference

| Field | Required | Description |
|-------|----------|-------------|
| `path` | Yes | Path to the `.IMG` file. **Relative to `tools/nd100-gateway/`**, so the project root is `../../`. |
| `name` | Yes | Display name shown in the browser UI catalog. Completely independent of the filename. |
| `description` | No | Longer text shown below the name in the catalog card. |

### How the Gateway Serves Images

The gateway exposes two HTTP endpoints (no static files needed):

- `GET /smd-catalog.json` -- Returns a JSON array generated from `gateway.conf.json`. Each entry includes an `available` flag (checks if the file exists on disk at request time).
- `GET /smd-images/{index}` -- Streams the image file by its array index (0-based position in `smd.images`).

The browser fetches the catalog, displays the entries, and when the user clicks "Copy to Library", it streams the image from `/smd-images/{index}` into the browser's local OPFS storage.

### Adding a New Image

1. Place the `.IMG` file somewhere the gateway process can read. The project root (alongside `SMD0.IMG`) is the simplest location:

   ```
   cp /path/to/SINTRAN-L.IMG /home/ronny/repos/nd100x/SINTRAN-L.IMG
   ```

2. Add an entry to `tools/nd100-gateway/gateway.conf.json`:

   ```json
   {
     "path": "../../SINTRAN-L.IMG",
     "name": "SINTRAN L",
     "description": "SINTRAN III/VSE version L with all patches"
   }
   ```

3. Restart the gateway:

   ```bash
   # Ctrl-C the running gateway, then:
   make wasm-glass-gateway
   ```

4. Verify:

   ```bash
   curl -s http://localhost:8765/smd-catalog.json | python3 -m json.tool
   ```

   Your entry should appear with `"available": true`. If the path is wrong, it shows `"available": false` and the browser displays a greyed-out card.

### Removing an Image

Remove its entry from the `smd.images` array in `gateway.conf.json` and restart the gateway.

**Note:** Array indices shift when you remove an entry. Do not remove entries while users may have in-progress downloads (the browser uses the index to stream).

### Renaming or Re-describing an Image

Change `name` and/or `description` in `gateway.conf.json` and restart the gateway. This has no effect on the physical `.IMG` file and no effect on images already copied to users' local browser libraries.

### Where Image Files Live

Images do NOT need to be inside `staticDir`. They are streamed directly from whatever `path` points to. You can put them anywhere:

```
../../SMD0.IMG                    # project root
../../images/SINTRAN-L.IMG        # project images/ subdirectory
/srv/nd100/disk-images/PROD.IMG   # absolute path (use full path in conf)
```

For absolute paths, use the full path:
```json
{ "path": "/srv/nd100/disk-images/PROD.IMG", "name": "Production", "description": "..." }
```

### The `staticDir` Setting

`staticDir` controls where HTML/JS/WASM files are served from. It is completely separate from the disk image catalog. When using `make wasm-glass-gateway`, the Makefile passes `--static build_wasm_glass/bin` automatically.

### Block I/O vs. Image Streaming

The catalog/streaming endpoints (`/smd-catalog.json`, `/smd-images/{n}`) are for the **browser's local library** -- the user copies an image into their browser's persistent storage (OPFS).

This is separate from the **gateway block I/O** (WebSocket binary frames 0x20-0x23), which is used for live disk access during emulation. Block I/O uses the `smd.images` array indices as unit numbers for the emulator's direct disk reads/writes.

---

## Static Mode (No Gateway)

### Catalog File

**Path:** `template-glass/smd-catalog.json` (source) -> `build_wasm_glass/bin/smd-catalog.json` (build output)

This is a hand-maintained JSON file. The build copies it to the output directory.

```json
[
    {
        "name": "SINTRAN K",
        "description": "SINTRAN III/VSE version K, standard ND-100 system disk",
        "size": 78643200,
        "url": "SMD0.IMG"
    },
    {
        "name": "Clean 75 MB Disk",
        "description": "Empty 75 MB SMD disk image, for installing an OS from floppy",
        "size": 78643200,
        "url": null,
        "generate": true
    }
]
```

### Field Reference

| Field | Required | Description |
|-------|----------|-------------|
| `name` | Yes | Display name in the browser catalog. |
| `description` | No | Longer description text. |
| `size` | Yes | Expected byte size. Used for progress display only, not validated. |
| `url` | Yes* | Relative URL to the `.IMG` file, served from the same directory as `index.html`. Set to `null` if using `generate`. |
| `generate` | No | If `true`, the browser creates a zero-filled image locally instead of downloading. No server file needed. |

*`url` must be present in `build_wasm_glass/bin/` for the download to work.

### Adding a New Image (Static Mode)

1. Place the `.IMG` file where the build output can reach it. The simplest approach is to copy it into the build output:

   ```bash
   cp /path/to/SINTRAN-L.IMG build_wasm_glass/bin/SINTRAN-L.IMG
   ```

   For a permanent setup, add a copy step to the Makefile (around line 117):

   ```makefile
   @cp SINTRAN-L.IMG $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
   ```

2. Add an entry to `template-glass/smd-catalog.json`:

   ```json
   {
       "name": "SINTRAN L",
       "description": "SINTRAN III/VSE version L",
       "size": 78643200,
       "url": "SINTRAN-L.IMG"
   }
   ```

3. Rebuild: `make wasm-glass` (or `make wasm-glass-run` to build and serve).

### Removing an Image (Static Mode)

Remove its entry from `template-glass/smd-catalog.json` and remove the `.IMG` file from the build output. Rebuild.

---

## Image File Format

All SMD disk images are raw binary files:

- **Standard size:** 78,643,200 bytes (75 MB) -- this is the size of a 75 MB SMD disk (Calcomp/CDC compatible)
- **Block size:** 1,024 words = 2,048 bytes per block (though block I/O uses 4,096-byte reads)
- **No header:** The file starts at byte 0 = disk sector 0. Byte offsets map directly to disk addresses.
- **File extension:** `.IMG` (uppercase by convention)
- **Endianness:** Big-endian (ND-100 native byte order)

To create a blank image from the command line:

```bash
dd if=/dev/zero of=BLANK.IMG bs=1M count=75
```

This produces a 78,643,200-byte file (75 * 1024 * 1024) that the emulator treats as an empty, unformatted SMD disk.

---

## What Happens in the Browser

When the user clicks "Copy to Library" in the SMD Disk Manager:

1. The browser opens a dialog where the user can edit the name and description.
2. On confirm, the browser downloads the image via XHR (from `url` or `/smd-images/{n}`).
3. The image is stored in the browser's Origin Private File System (OPFS) under a UUID filename.
4. Metadata (name, description, source) is stored in `localStorage`.
5. The user can then assign the image to a drive unit (0-3) from the Local Library section.

Multiple copies from the same source are allowed -- each gets its own UUID and can be independently named and assigned.

---

## Quick Reference

### Gateway Mode Checklist

```
1. Put .IMG file on the server
2. Edit tools/nd100-gateway/gateway.conf.json  (add to smd.images)
3. Restart gateway (make wasm-glass-gateway)
4. Verify: curl http://localhost:8765/smd-catalog.json
```

### Static Mode Checklist

```
1. Put .IMG file in project root or images/
2. Edit template-glass/smd-catalog.json  (add entry with url)
3. Add Makefile copy step (or manually cp to build output)
4. Rebuild: make wasm-glass
```
