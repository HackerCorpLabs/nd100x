/*
 * nd500_wasm.c - the ND-500 inside nd100x's WebAssembly module.
 *
 * WHY THE ND-500 IS LINKED IN HERE AND NOT LOADED AS A SECOND MODULE
 * -----------------------------------------------------------------
 * An ND-100 + ND-500 machine has SHARED MEMORY (MPM5): the ND-500 reads and
 * writes the ND-100's memory directly. In a browser that means a single
 * WebAssembly.Memory, and a Module owns its memory, so both CPUs have to live
 * in ONE module. Two script tags cannot express the hardware. The whole shape
 * of this file follows from that one fact.
 *
 * WHAT THIS FILE IS, AND IS NOT
 * -----------------------------
 * It is the thin seam between the browser and libnd500: create the machine,
 * hand it a kernel and a disk, boot it, step it, move console bytes. All the
 * hard parts - the MMU setup, the PSEG/DSEG placement, THA/CTE1/CTE2/CAD, the
 * u-area - live in nd500x's own nd500_ndix_boot.c and are called, not copied.
 * That library function exists precisely so a caller with no debugger and no
 * main() can boot NDIX, which is exactly what a browser is.
 *
 * It is NOT the ND-100 <-> ND-500 pairing. Nothing here connects the two
 * machines: the ND-500 runs on its own, answering its own fecalls the way
 * nd500x does natively (front_end = synthetic). Giving SINTRAN a real ND-500
 * over the 3022 bus interface, and the shared-memory window itself, are later
 * milestones (M5).
 *
 * BUILD. Everything below is inside ND100X_WITH_ND500, which the root
 * CMakeLists sets only when an nd500x checkout was found. The stub half at the
 * bottom keeps every exported name present either way - EXPORTED_FUNCTIONS
 * names them and emscripten fails the link on a name it cannot find, so
 * "no ND-500" has to be a value returned at runtime, not a missing symbol.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EMSCRIPTEN_EXPORT
#endif

#ifdef ND100X_WITH_ND500

/* nd500x's own headers, addressed BY SUBDIRECTORY on purpose.
 *
 * nd100x and nd500x both have a src/machine/machine_types.h and a
 * src/cpu/cpu_protos.h, and nd100wasm already has ${CMAKE_SOURCE_DIR}/src/machine
 * and .../src/cpu on its include path - so a bare "machine_types.h" here picks
 * up the ND-100's. It did, and the errors were about Nd500Machine being an
 * incomplete type. A header whose contents merely DIFFERED would have compiled
 * and been silently wrong, so this is worth the two-part paths: only nd500x's
 * src/ is on the path as a directory, nd100x's is not.
 *
 * cpu_protos.h is where struct Nd500Cpu is actually DEFINED (line 62); the
 * other cpu headers only forward-declare it, and the machine and CPU below are
 * plain objects with static storage, not pointers. */
#include "machine/machine_types.h"
#include "machine/machine_protos.h"
#include "machine/nd500_ndix_boot.h"
#include "cpu/nd500_host.h"
#include "cpu/nd500_fecall.h"
#include "cpu/cpu_protos.h"

/* ------------------------------------------------------------------ state */

/* One ND-500 per module, matching nd500x's own "one ND-500 per process" rule
 * (see the note in nd500_host.h about why the host ops are module-level). Even
 * in the eventual ND-100 + ND-500 pairing there is one ND-500 per front end. */
static Nd500Machine  g_m;
static Nd500Cpu      g_cpu;
static int           g_created = 0;
static int           g_booted  = 0;

/* ------------------------------------------------------- disks (host ops)
 *
 * The ND-500's disks are served out of the wasm heap: JS allocates a buffer
 * with _malloc, fills it from OPFS or from a download, and hands the pointer
 * over. That is the same deal MountSMDFromBuffer already makes for the ND-100
 * side, and it is deliberately the SIMPLEST of the three storage paths nd100x
 * has.
 *
 * NOT DONE YET, and worth being explicit about: the OPFS-worker path and the
 * gateway path are not wired to the ND-500. A disk mounted here is a snapshot
 * in memory - writes land in the buffer and JS has to read it back out to keep
 * them. For a root filesystem of any size that is a lot of heap, and streaming
 * it through the existing SharedArrayBuffer disk worker is the real answer.
 * The Nd500HostOps interface is already the right shape for it (byte offsets,
 * per unit), so that is a change behind this seam, not to it.
 */
typedef struct {
    uint8_t* data;
    uint64_t size;
    int      writable;
} Nd500WasmDisk;

static Nd500WasmDisk g_disks[ND500_HOST_MAX_DISKS];

static int64_t wasm_disk_size(void* ctx, int unit) {
    (void)ctx;
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return -1;
    if (!g_disks[unit].data) return -1;          /* -1 IS "not mounted" */
    return (int64_t)g_disks[unit].size;
}

static int64_t wasm_disk_read(void* ctx, int unit, uint64_t off,
                              void* dst, uint32_t len) {
    (void)ctx;
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return -1;
    Nd500WasmDisk* d = &g_disks[unit];
    if (!d->data) return -1;
    if (off >= d->size) return 0;                /* past the end: 0 bytes, not an error */
    /* A short count is a real short count - the interface says so, and the
     * fecall layer copes with it. Clamping here is how a read that straddles
     * the end of the image behaves like a real disc rather than failing. */
    if (off + len > d->size) len = (uint32_t)(d->size - off);
    memcpy(dst, d->data + off, len);
    return (int64_t)len;
}

static int64_t wasm_disk_write(void* ctx, int unit, uint64_t off,
                               const void* src, uint32_t len) {
    (void)ctx;
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return -1;
    Nd500WasmDisk* d = &g_disks[unit];
    if (!d->data || !d->writable) return -1;
    if (off >= d->size) return 0;
    if (off + len > d->size) len = (uint32_t)(d->size - off);
    memcpy(d->data + off, src, len);
    return (int64_t)len;
}

static int wasm_disk_writable(void* ctx, int unit) {
    (void)ctx;
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return 0;
    return g_disks[unit].data ? g_disks[unit].writable : 0;
}

static const Nd500HostOps g_host_ops = {
    wasm_disk_size, wasm_disk_read, wasm_disk_write, wasm_disk_writable
};

/* ------------------------------------------------------------- console ---
 *
 * Same shape as the ND-100 terminal ring buffer above in nd100wasm.c: the
 * guest writes whenever it likes, JS drains after each Step. Packing the unit
 * into the high byte keeps one queue for all four tty units, so the order
 * bytes were produced in survives - which matters, because the boot log and a
 * getty banner interleave.
 */
#define ND500_CON_BUF 8192
static struct {
    uint16_t entries[ND500_CON_BUF];
    volatile int writePos;
    volatile int readPos;
} g_con = { {0}, 0, 0 };

static void con_push(int unit, uint8_t c) {
    int next = (g_con.writePos + 1) % ND500_CON_BUF;
    if (next == g_con.readPos) return;           /* full: drop, like a real line */
    g_con.entries[g_con.writePos] = (uint16_t)(((unit & 0xFF) << 8) | c);
    g_con.writePos = next;
}

static void nd500_tty_sink(int unit, const unsigned char* buf, int len, void* ctx) {
    (void)ctx;
    for (int i = 0; i < len; i++) con_push(unit, buf[i]);
}

/* ------------------------------------------------------- boot-log capture
 *
 * nd500_ndix_boot() reports each step through a log hook. Natively that goes to
 * stderr; here it goes into the same queue as guest output on a unit of its
 * own (0xFF), so the page can show "mmusetup", "map-kdata", "ndix-uarea" as
 * they happen. When a boot dies, the last line printed is how you know which
 * step it died in - losing that in the browser would make every failure look
 * identical.
 */
static void nd500_log_line(void* ctx, const char* line) {
    (void)ctx;
    if (!line) return;
    for (const char* p = line; *p; p++) con_push(0xFF, (uint8_t)*p);
    con_push(0xFF, '\n');
}

/* --------------------------------------------------------- MEMFS staging
 *
 * nd500_ndix_boot() takes FILE PATHS, and reads the a.out header with stat()
 * and fopen(). Rather than fork the library for the browser, the buffers JS
 * hands over are written into emscripten's in-memory filesystem and the paths
 * of those files are what gets passed in. The boot path stays the ONE that is
 * exercised natively - which is the whole point of having lifted it into the
 * library in the first place.
 */
#define ND500_KERNEL_PATH "/nd500_kernel"
#define ND500_PSEG_PATH   "/nd500_kernel.pseg"
#define ND500_DSEG_PATH   "/nd500_kernel.dseg"

static int stage_file(const char* path, const uint8_t* data, int len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t n = fwrite(data, 1, (size_t)len, f);
    fclose(f);
    return (n == (size_t)len) ? 0 : -1;
}

/* ================================================================ exports */

EMSCRIPTEN_EXPORT int Nd500_Available(void) { return 1; }

/* Create the machine. <mem_bytes> 0 takes the 16 MB nd500x uses natively.
 * Returns 0 on success. */
EMSCRIPTEN_EXPORT int Nd500_Create(int mem_bytes) {
    if (g_created) return 0;                     /* idempotent, like the ND-100 Init */
    if (mem_bytes <= 0) mem_bytes = 16 * 1024 * 1024;

    memset(&g_m, 0, sizeof g_m);
    memset(&g_cpu, 0, sizeof g_cpu);
    nd500_machine_init(&g_m, (uint32_t)mem_bytes);
    nd500_cpu_init(&g_cpu, &g_m);
    nd500_cpu_reset(&g_cpu);

    /* Disks come from the browser, not from stdio. This must happen before the
     * machine runs - nd500_host.h is explicit that changing hosts mid-boot is
     * not supported. */
    nd500_host_set(&g_host_ops, NULL);

    /* Both boot-log channels into the console queue. */
    nd500_ndix_set_notice_log(nd500_log_line, NULL);
    nd500_ndix_set_verbose_log(nd500_log_line, NULL);

    /* Every tty unit the fecall layer knows about. Without a sink, output for
     * a unit nobody is attached to is dropped - correct natively, wrong here,
     * where the page IS every terminal. */
    for (int u = 0; u < ND500_TTY_MAX_UNITS; u++)
        nd500_fecall_set_tty_output(u, nd500_tty_sink, NULL);

    g_created = 1;
    g_booted = 0;
    return 0;
}

EMSCRIPTEN_EXPORT int Nd500_IsCreated(void) { return g_created; }
EMSCRIPTEN_EXPORT int Nd500_IsBooted(void)  { return g_booted; }

/* Stage the kernel a.out. Call before Nd500_Boot. */
EMSCRIPTEN_EXPORT int Nd500_LoadKernel(uint8_t* data, int len) {
    if (!data || len <= 0) return -1;
    return stage_file(ND500_KERNEL_PATH, data, len);
}

/* Stage the pre-split segment files, when they exist. BOTH or NEITHER: the
 * library treats a half-present pair as absent and derives the sizes from the
 * a.out header instead, which is the path a kernel extracted from a disk image
 * takes - there are no segment files inside an image. */
EMSCRIPTEN_EXPORT int Nd500_LoadSegments(uint8_t* pseg, int pseg_len,
                                         uint8_t* dseg, int dseg_len) {
    if (!pseg || !dseg || pseg_len <= 0 || dseg_len <= 0) return -1;
    if (stage_file(ND500_PSEG_PATH, pseg, pseg_len) != 0) return -1;
    return stage_file(ND500_DSEG_PATH, dseg, dseg_len);
}

/* Mount a disk image already in the wasm heap. The buffer is NOT copied and
 * NOT freed here: JS owns it, and that is what lets JS read written blocks
 * back out afterwards. */
EMSCRIPTEN_EXPORT int Nd500_MountDisk(int unit, uint8_t* data, int len, int writable) {
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return -1;
    if (!data || len <= 0) return -1;
    g_disks[unit].data     = data;
    g_disks[unit].size     = (uint64_t)len;
    g_disks[unit].writable = writable ? 1 : 0;
    return 0;
}

EMSCRIPTEN_EXPORT int Nd500_UnmountDisk(int unit) {
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return -1;
    g_disks[unit].data = NULL;
    g_disks[unit].size = 0;
    g_disks[unit].writable = 0;
    return 0;
}

/* Where a unit's bytes live, so JS can read back what the guest wrote. */
EMSCRIPTEN_EXPORT uint8_t* Nd500_GetDiskBuffer(int unit) {
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return NULL;
    return g_disks[unit].data;
}
EMSCRIPTEN_EXPORT int Nd500_GetDiskSize(int unit) {
    if (unit < 0 || unit >= ND500_HOST_MAX_DISKS) return 0;
    return (int)g_disks[unit].size;
}

/* Run the whole NDIX boot sequence. Everything it does belongs to SINTRAN and
 * the ND-100 on real hardware; nd500x stands in for them until the 3022 bus
 * interface is real (M5). */
EMSCRIPTEN_EXPORT int Nd500_Boot(void) {
    if (!g_created) return -1;
    Nd500NdixBoot cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.kernel_path = ND500_KERNEL_PATH;

    /* Offer the segment files only when both were staged. fopen is the test:
     * MEMFS has no stat cost worth avoiding and this needs no extra header. */
    FILE* fp = fopen(ND500_PSEG_PATH, "rb");
    FILE* fd = fopen(ND500_DSEG_PATH, "rb");
    if (fp && fd) { cfg.pseg_path = ND500_PSEG_PATH; cfg.dseg_path = ND500_DSEG_PATH; }
    if (fp) fclose(fp);
    if (fd) fclose(fd);

    /* The u-area step IS done here. Natively it is the frontend's job because
     * two different boot routes need it; here there is only one route. */
    cfg.with_uarea = 1;

    int rc = nd500_ndix_boot(&g_m, &cfg);
    if (rc == 0) g_booted = 1;
    return rc;
}

/* Advance the ND-500 by <count> instructions. Stepping rather than running:
 * a browser has one thread for the page, and nd500_dbg_run() does not come
 * back until the guest stops. The caller decides the slice. */
EMSCRIPTEN_EXPORT int Nd500_Step(int count) {
    if (!g_created || !g_booted) return -1;
    if (count <= 0) count = 1;
    nd500_dbg_step(&g_m, (uint32_t)count);
    return (int)g_m.stop_reason;
}

EMSCRIPTEN_EXPORT int Nd500_GetStopReason(void) {
    return g_created ? (int)g_m.stop_reason : 0;
}

/* Why it stopped, in words. nd500x already has the table; there is no reason
 * for JS to carry a second copy of the enum. */
EMSCRIPTEN_EXPORT const char* Nd500_GetStopReasonText(void) {
    if (!g_created) return "no machine";
    return nd500_stop_reason_str(g_m.stop_reason);
}

EMSCRIPTEN_EXPORT uint32_t Nd500_GetPC(void) {
    return g_created ? g_cpu.PC : 0u;
}

/* Drain one console byte: (unit << 8) | byte, or -1 when empty.
 * Unit 0xFF is this file's own boot log, not guest output. */
EMSCRIPTEN_EXPORT int Nd500_PollConsole(void) {
    if (g_con.readPos == g_con.writePos) return -1;
    uint16_t e = g_con.entries[g_con.readPos];
    g_con.readPos = (g_con.readPos + 1) % ND500_CON_BUF;
    return (int)e;
}

/* Type at a guest terminal. */
EMSCRIPTEN_EXPORT void Nd500_SendInput(int unit, const char* text, int len) {
    if (!g_created || !text || len <= 0) return;
    nd500_fecall_tty_input(unit, text, len);
}

#else  /* ---------------------------------------------- no nd500x checkout */

/*
 * The same names, doing nothing. EXPORTED_FUNCTIONS lists them unconditionally
 * and emscripten fails the link on a name it cannot find, so the absence of an
 * ND-500 has to be something the page ASKS about (Nd500_Available returns 0)
 * rather than a module that will not load.
 */
EMSCRIPTEN_EXPORT int  Nd500_Available(void) { return 0; }
EMSCRIPTEN_EXPORT int  Nd500_Create(int mem_bytes) { (void)mem_bytes; return -1; }
EMSCRIPTEN_EXPORT int  Nd500_IsCreated(void) { return 0; }
EMSCRIPTEN_EXPORT int  Nd500_IsBooted(void) { return 0; }
EMSCRIPTEN_EXPORT int  Nd500_LoadKernel(uint8_t* d, int n) { (void)d; (void)n; return -1; }
EMSCRIPTEN_EXPORT int  Nd500_LoadSegments(uint8_t* p, int pn, uint8_t* d, int dn) {
    (void)p; (void)pn; (void)d; (void)dn; return -1;
}
EMSCRIPTEN_EXPORT int  Nd500_MountDisk(int u, uint8_t* d, int n, int w) {
    (void)u; (void)d; (void)n; (void)w; return -1;
}
EMSCRIPTEN_EXPORT int  Nd500_UnmountDisk(int u) { (void)u; return -1; }
EMSCRIPTEN_EXPORT uint8_t* Nd500_GetDiskBuffer(int u) { (void)u; return 0; }
EMSCRIPTEN_EXPORT int  Nd500_GetDiskSize(int u) { (void)u; return 0; }
EMSCRIPTEN_EXPORT int  Nd500_Boot(void) { return -1; }
EMSCRIPTEN_EXPORT int  Nd500_Step(int count) { (void)count; return -1; }
EMSCRIPTEN_EXPORT int  Nd500_GetStopReason(void) { return 0; }
EMSCRIPTEN_EXPORT const char* Nd500_GetStopReasonText(void) { return "no ND-500 in this build"; }
EMSCRIPTEN_EXPORT uint32_t Nd500_GetPC(void) { return 0u; }
EMSCRIPTEN_EXPORT int  Nd500_PollConsole(void) { return -1; }
EMSCRIPTEN_EXPORT void Nd500_SendInput(int u, const char* t, int n) {
    (void)u; (void)t; (void)n;
}

#endif /* ND100X_WITH_ND500 */
