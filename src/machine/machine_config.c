/*
 * nd100x - ND100 Virtual Machine
 *
 * machine_config.c - controller registry, INI parser, and validator for the
 * machine configuration model. See machine_config.h and
 * docs/MACHINE-CONFIG-DESIGN.md.
 *
 * The INI parser is a small hand-written tokenizer (no external INI dependency).
 * Every rejection is reported with the file name, the 1-based line number, and a
 * short "what/why/how to fix" message.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>

#include "machine_config.h"
#include "../cpu/cpu_types.h"   /* CpuType (ND100, ND110, ...) */

/* ------------------------------------------------------------------ */
/* Controller registry                                                 */
/* ------------------------------------------------------------------ */
/* IOX base per thumbwheel. Index is the thumbwheel number. Wheels outside a
 * controller's [min_wheel,max_wheel] are 0 and never referenced. */
static const uint16_t iox_smd[]    = { 001540 };                               /* wheel 0 */
static const uint16_t iox_floppy[] = { 001560 };                               /* wheel 0 */
static const uint16_t iox_scsi[]   = { 0, 0, 0, 0 };                           /* filled below */
static const uint16_t iox_scsi_v[] = { 0144300, 0144400, 0144500, 0144600 };   /* wheel 0-3 */
static const uint16_t iox_hdlc[]   = { 0, 001640, 001660, 001700, 001720 };    /* wheel 1-4 */

/* The registry. is_disc controllers carry disk_slots; network controllers 0. */
static const ControllerDescriptor g_descriptors[] = {
    /* type        name      minW maxW  iox_base     span  slots is_disc bootable */
    { CTRL_FLOPPY, "floppy",  0,  0,  iox_floppy,     8,     3,  true,   true  },
    { CTRL_SMD,    "smd",     0,  0,  iox_smd,        8,     4,  true,   true  },
    { CTRL_SCSI,   "scsi",    0,  3,  iox_scsi_v,   0100,    7,  true,   true  },
    { CTRL_HDLC,   "hdlc",    1,  4,  iox_hdlc,     020,     0,  false,  false },
};
static const int g_descriptor_count =
    (int)(sizeof(g_descriptors) / sizeof(g_descriptors[0]));

/* Silence the unused-array warning for the reserved zero table; kept for
 * symmetry/readability with iox_scsi_v. */
static const uint16_t *const _mc_unused_iox = iox_scsi;

const ControllerDescriptor *MC_DescriptorForType(CtrlType type)
{
    for (int i = 0; i < g_descriptor_count; i++)
        if (g_descriptors[i].type == type)
            return &g_descriptors[i];
    return NULL;
}

const ControllerDescriptor *MC_DescriptorForName(const char *name)
{
    if (!name) return NULL;
    for (int i = 0; i < g_descriptor_count; i++)
        if (strcmp(g_descriptors[i].name, name) == 0)
            return &g_descriptors[i];
    return NULL;
}

CtrlType MC_CtrlTypeFromName(const char *name)
{
    const ControllerDescriptor *d = MC_DescriptorForName(name);
    return d ? d->type : CTRL_NONE;
}

const char *MC_CtrlTypeName(CtrlType type)
{
    const ControllerDescriptor *d = MC_DescriptorForType(type);
    return d ? d->name : "none";
}

/* ------------------------------------------------------------------ */
/* Small string helpers                                                */
/* ------------------------------------------------------------------ */
static void str_copy(char *dst, size_t dstlen, const char *src)
{
    if (!dstlen) return;
    strncpy(dst, src ? src : "", dstlen - 1);
    dst[dstlen - 1] = '\0';
}

/* Trim leading/trailing ASCII whitespace in place; returns the start. */
static char *str_trim(char *s)
{
    if (!s) return s;
    while (*s && isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return s;
}

static void str_lower(char *s)
{
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

static bool str_ieq(const char *a, const char *b)
{
    if (!a || !b) return false;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

/* Parse a yes/no/true/false/on/off/1/0 value. Returns -1 if unrecognized. */
static int parse_bool(const char *v)
{
    if (str_ieq(v, "yes") || str_ieq(v, "true") || str_ieq(v, "on") || str_ieq(v, "1"))
        return 1;
    if (str_ieq(v, "no") || str_ieq(v, "false") || str_ieq(v, "off") || str_ieq(v, "0"))
        return 0;
    return -1;
}

/* ------------------------------------------------------------------ */
/* Defaults                                                            */
/* ------------------------------------------------------------------ */
void MachineConfig_InitBaseline(MachineConfig *cfg)
{
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));

    cfg->cpu_type = 100;

    /* terminals 5-11 (console/0 is always present, not listed here) */
    int terms[] = { 5, 6, 7, 8, 9, 10, 11 };
    for (size_t i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
        cfg->terminals[cfg->terminalCount++] = terms[i];

    cfg->ptreader_enabled    = true;
    cfg->ptpunch_enabled     = true;
    cfg->lineprinter_enabled = true;

    cfg->boot.is_disc = true;
    cfg->boot.type    = CTRL_SMD;
    cfg->boot.wheel   = 0;
    cfg->boot.unit    = 0;

    cfg->runtime.telnet_port   = 0;
    cfg->runtime.throttle_mhz  = 0.0;
    str_copy(cfg->runtime.charset, sizeof(cfg->runtime.charset), "off");
    cfg->runtime.debugger_port = 0;
    cfg->runtime.trace         = false;

    (void)_mc_unused_iox;
}

void MachineConfig_SetDefaults(MachineConfig *cfg)
{
    if (!cfg) return;
    MachineConfig_InitBaseline(cfg);

    /* floppy.0 */
    MC_Controller *fl = &cfg->controllers[cfg->controllerCount++];
    fl->type = CTRL_FLOPPY; fl->wheel = 0; fl->enabled = true;
    fl->disks[0].present = true; fl->disks[0].media = SCSI_UNIT_HDD;
    str_copy(fl->disks[0].image, MC_PATH_LEN, "FLOPPY.IMG");

    /* smd.0 */
    MC_Controller *sm = &cfg->controllers[cfg->controllerCount++];
    sm->type = CTRL_SMD; sm->wheel = 0; sm->enabled = true;
    sm->disks[0].present = true; sm->disks[0].media = SCSI_UNIT_HDD;
    str_copy(sm->disks[0].image, MC_PATH_LEN, "SMD0.IMG");

    /* scsi.0 */
    MC_Controller *sc = &cfg->controllers[cfg->controllerCount++];
    sc->type = CTRL_SCSI; sc->wheel = 0; sc->enabled = true;
    sc->disks[0].present = true; sc->disks[0].media = SCSI_UNIT_HDD;
    str_copy(sc->disks[0].image, MC_PATH_LEN, "SCSI0.IMG");
}

/* ------------------------------------------------------------------ */
/* Autoload INI name from argv[0]                                      */
/* ------------------------------------------------------------------ */
void MachineConfig_DefaultIniName(const char *argv0, char *outbuf, size_t outlen)
{
    if (!outbuf || !outlen) return;
    const char *base = argv0 ? argv0 : "nd100x";

    /* basename: last '/' or '\\' */
    const char *slash = strrchr(base, '/');
    const char *bslash = strrchr(base, '\\');
    if (bslash && (!slash || bslash > slash)) slash = bslash;
    if (slash) base = slash + 1;

    char tmp[MC_PATH_LEN];
    str_copy(tmp, sizeof(tmp), base);

    /* strip a trailing ".exe" (case-insensitive) */
    size_t n = strlen(tmp);
    if (n > 4 && str_ieq(tmp + n - 4, ".exe")) tmp[n - 4] = '\0';

    if (tmp[0] == '\0') str_copy(tmp, sizeof(tmp), "nd100x");

    snprintf(outbuf, outlen, "%s.ini", tmp);
}

/* ------------------------------------------------------------------ */
/* INI parsing                                                         */
/* ------------------------------------------------------------------ */
/* Section kinds recognized while parsing. */
typedef enum {
    SEC_NONE = 0,
    SEC_MACHINE,
    SEC_CONTROLLER,
    SEC_TERMINALS,
    SEC_PERIPHERAL,
    SEC_BOOT,
    SEC_RUNTIME,
    SEC_UNKNOWN
} SectionKind;

static int mc_err(char *err, size_t errlen, const char *path, int line,
                  const char *fmt, ...)
{
    char msg[MC_ERR_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    if (err && errlen) {
        if (line > 0)
            snprintf(err, errlen, "%s:%d  %s", path ? path : "?", line, msg);
        else
            snprintf(err, errlen, "%s  %s", path ? path : "?", msg);
    }
    return 0; /* convenience: callers `return mc_err(...)` yields false */
}

/* Find (or create) the controller matching (type,wheel). Returns NULL if the
 * table is full. *created set true when a new entry was allocated. */
static MC_Controller *mc_get_controller(MachineConfig *cfg, CtrlType type,
                                        int wheel, bool *created)
{
    if (created) *created = false;
    for (int i = 0; i < cfg->controllerCount; i++)
        if (cfg->controllers[i].type == type && cfg->controllers[i].wheel == wheel)
            return &cfg->controllers[i];
    if (cfg->controllerCount >= MC_MAX_CONTROLLERS)
        return NULL;
    MC_Controller *c = &cfg->controllers[cfg->controllerCount++];
    memset(c, 0, sizeof(*c));
    c->type = type;
    c->wheel = wheel;
    c->enabled = true; /* a listed controller defaults to enabled */
    if (created) *created = true;
    return c;
}

/* Parse a "controller.<name>.<wheel>" section header. Returns false + message. */
static bool mc_parse_controller_header(const char *rest, CtrlType *type,
                                       int *wheel, char *err, size_t errlen,
                                       const char *path, int line)
{
    /* rest points past "controller." e.g. "scsi.0" */
    char buf[64];
    str_copy(buf, sizeof(buf), rest);
    char *dot = strrchr(buf, '.');
    if (!dot) {
        return mc_err(err, errlen, path, line,
            "[controller.%s]: missing thumbwheel. Use [controller.<type>.<wheel>], e.g. [controller.scsi.0].",
            rest);
    }
    *dot = '\0';
    const char *typeName = buf;
    const char *wheelStr = dot + 1;

    const ControllerDescriptor *d = MC_DescriptorForName(typeName);
    if (!d) {
        return mc_err(err, errlen, path, line,
            "unknown controller type '%s'. Known types: floppy, smd, scsi, hdlc.",
            typeName);
    }

    char *endp;
    long w = strtol(wheelStr, &endp, 10);
    if (*endp != '\0' || w < 0) {
        return mc_err(err, errlen, path, line,
            "[controller.%s.%s]: thumbwheel must be a number.", typeName, wheelStr);
    }
    if (w < d->min_wheel || w > d->max_wheel) {
        return mc_err(err, errlen, path, line,
            "[controller.%s.%ld]: thumbwheel %ld out of range for %s (%d-%d).",
            typeName, w, w, typeName, d->min_wheel, d->max_wheel);
    }

    *type = d->type;
    *wheel = (int)w;
    return true;
}

/* Parse a diskN key for a disc controller. keyrest is the text after "disk". */
static bool mc_parse_disk_key(MC_Controller *c, const ControllerDescriptor *d,
                              const char *keyrest, const char *value,
                              char *err, size_t errlen, const char *path, int line)
{
    char *endp;
    long slot = strtol(keyrest, &endp, 10);
    if (*endp != '\0' || slot < 0) {
        return mc_err(err, errlen, path, line,
            "[controller.%s.%d]: bad disk key 'disk%s'. Use disk0..disk%d.",
            d->name, c->wheel, keyrest, d->disk_slots - 1);
    }
    if (slot >= d->disk_slots) {
        if (d->type == CTRL_SCSI && slot == 7)
            return mc_err(err, errlen, path, line,
                "[controller.scsi.%d] disk7: SCSI ID 7 is the controller itself. Valid IDs are 0-6.",
                c->wheel);
        return mc_err(err, errlen, path, line,
            "[controller.%s.%d] disk%ld: slot out of range. Valid: disk0..disk%d.",
            d->name, c->wheel, slot, d->disk_slots - 1);
    }

    /* value = [media:]path. Media prefix only when the text before the first
     * ':' is a known media word (so a bare path, incl. "C:\..", stays a path). */
    SCSIUnitType media = SCSI_UNIT_HDD;
    const char *file = value;
    const char *colon = strchr(value, ':');
    const char *fslash = strchr(value, '/');
    if (colon && colon != value && (!fslash || colon < fslash)) {
        char prefix[16];
        size_t plen = (size_t)(colon - value);
        if (plen < sizeof(prefix)) {
            memcpy(prefix, value, plen);
            prefix[plen] = '\0';
            SCSIUnitType m = SCSI_ParseUnitType(prefix);
            if (m != SCSI_UNIT_NONE) {
                media = m;
                file = colon + 1;
            } else {
                /* text before ':' looks like a media word but isn't known */
                return mc_err(err, errlen, path, line,
                    "[controller.%s.%d] disk%ld: unknown media '%s'. Use hdd, cdrom, tape or floppy.",
                    d->name, c->wheel, slot, prefix);
            }
        }
    }
    if (*file == '\0') {
        return mc_err(err, errlen, path, line,
            "[controller.%s.%d] disk%ld: no image file given.",
            d->name, c->wheel, slot);
    }

    c->disks[slot].present = true;
    c->disks[slot].media = media;
    str_copy(c->disks[slot].image, MC_PATH_LEN, file);
    return true;
}

bool MachineConfig_LoadFile(MachineConfig *cfg, const char *path,
                            char *err, size_t errlen)
{
    if (!cfg || !path) return false;

    FILE *f = fopen(path, "r");
    if (!f) {
        return mc_err(err, errlen, path, 0,
            "cannot open config file (%s).", strerror(errno));
    }

    str_copy(cfg->source_path, sizeof(cfg->source_path), path);
    cfg->loaded_from_file = true;

    char line[512];
    int lineno = 0;

    SectionKind kind = SEC_NONE;
    CtrlType curType = CTRL_NONE;
    int curWheel = 0;
    MC_Controller *curCtrl = NULL;
    char periphName[64] = {0};

    while (fgets(line, sizeof(line), f)) {
        lineno++;

        /* strip comment (# or ;) and trim */
        char *p = line;
        for (char *c = line; *c; c++) {
            if (*c == '#' || *c == ';') { *c = '\0'; break; }
        }
        p = str_trim(line);
        if (*p == '\0') continue;

        /* section header */
        if (*p == '[') {
            char *close = strchr(p, ']');
            if (!close) {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "malformed section header (missing ']').");
            }
            *close = '\0';
            char *sec = str_trim(p + 1);
            str_lower(sec);

            curCtrl = NULL;
            if (str_ieq(sec, "machine")) {
                kind = SEC_MACHINE;
            } else if (str_ieq(sec, "terminals")) {
                kind = SEC_TERMINALS;
            } else if (str_ieq(sec, "boot")) {
                kind = SEC_BOOT;
            } else if (str_ieq(sec, "runtime")) {
                kind = SEC_RUNTIME;
            } else if (strncmp(sec, "controller.", 11) == 0) {
                kind = SEC_CONTROLLER;
                if (!mc_parse_controller_header(sec + 11, &curType, &curWheel,
                                                err, errlen, path, lineno)) {
                    fclose(f);
                    return false;
                }
                /* duplicate controller = duplicate (type,wheel) section */
                for (int i = 0; i < cfg->controllerCount; i++) {
                    if (cfg->controllers[i].type == curType &&
                        cfg->controllers[i].wheel == curWheel) {
                        fclose(f);
                        return mc_err(err, errlen, path, lineno,
                            "[controller.%s.%d]: duplicate controller. "
                            "A '%s' controller on thumbwheel %d is already defined. "
                            "Each controller+thumbwheel may appear only once.",
                            MC_CtrlTypeName(curType), curWheel,
                            MC_CtrlTypeName(curType), curWheel);
                    }
                }
                bool created = false;
                curCtrl = mc_get_controller(cfg, curType, curWheel, &created);
                if (!curCtrl) {
                    fclose(f);
                    return mc_err(err, errlen, path, lineno,
                        "too many controllers (max %d).", MC_MAX_CONTROLLERS);
                }
            } else if (strncmp(sec, "peripheral.", 11) == 0) {
                kind = SEC_PERIPHERAL;
                str_copy(periphName, sizeof(periphName), sec + 11);
            } else {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "unknown section [%s].", sec);
            }
            continue;
        }

        /* key = value */
        char *eq = strchr(p, '=');
        if (!eq) {
            fclose(f);
            return mc_err(err, errlen, path, lineno,
                "expected 'key = value' (no '=' found).");
        }
        *eq = '\0';
        char *key = str_trim(p);
        char *val = str_trim(eq + 1);
        char keyl[64];
        str_copy(keyl, sizeof(keyl), key);
        str_lower(keyl);

        switch (kind) {
        case SEC_MACHINE:
            if (str_ieq(keyl, "cpu")) {
                char *ep; long c = strtol(val, &ep, 10);
                if (*ep != '\0' || (c != 100 && c != 110 && c != 120)) {
                    fclose(f);
                    return mc_err(err, errlen, path, lineno,
                        "[machine] cpu = %s: must be 100, 110 or 120.", val);
                }
                cfg->cpu_type = (int)c;
            } else {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "[machine]: unknown key '%s'.", key);
            }
            break;

        case SEC_CONTROLLER: {
            if (!curCtrl) break;
            const ControllerDescriptor *d = MC_DescriptorForType(curType);
            if (str_ieq(keyl, "enabled")) {
                int b = parse_bool(val);
                if (b < 0) { fclose(f); return mc_err(err, errlen, path, lineno,
                    "[controller.%s.%d] enabled = %s: use yes or no.", d->name, curWheel, val); }
                curCtrl->enabled = (b == 1);
            } else if (strncmp(keyl, "disk", 4) == 0 && d->is_disc) {
                if (!mc_parse_disk_key(curCtrl, d, keyl + 4, val, err, errlen, path, lineno)) {
                    fclose(f); return false;
                }
            } else if (d->type == CTRL_HDLC && str_ieq(keyl, "mode")) {
                if (str_ieq(val, "server")) curCtrl->hdlc_is_server = true;
                else if (str_ieq(val, "client")) curCtrl->hdlc_is_server = false;
                else { fclose(f); return mc_err(err, errlen, path, lineno,
                    "[controller.hdlc.%d] mode = %s: use server or client.", curWheel, val); }
            } else if (d->type == CTRL_HDLC && str_ieq(keyl, "host")) {
                str_copy(curCtrl->hdlc_host, MC_PATH_LEN, val);
            } else if (d->type == CTRL_HDLC && str_ieq(keyl, "port")) {
                char *ep; long pt = strtol(val, &ep, 10);
                if (*ep != '\0' || pt <= 0 || pt > 65535) { fclose(f);
                    return mc_err(err, errlen, path, lineno,
                    "[controller.hdlc.%d] port = %s: must be 1-65535.", curWheel, val); }
                curCtrl->hdlc_port = (int)pt;
            } else {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "[controller.%s.%d]: unknown key '%s'.", d->name, curWheel, key);
            }
            break;
        }

        case SEC_TERMINALS:
            if (str_ieq(keyl, "enabled")) {
                /* comma/space separated list of terminal numbers */
                cfg->terminalCount = 0;
                char *tok = strtok(val, ", \t");
                while (tok) {
                    char *ep; long t = strtol(tok, &ep, 10);
                    if (*ep != '\0' || t < 0) { fclose(f);
                        return mc_err(err, errlen, path, lineno,
                        "[terminals] enabled: '%s' is not a terminal number.", tok); }
                    if (cfg->terminalCount >= MC_MAX_TERMINALS) { fclose(f);
                        return mc_err(err, errlen, path, lineno,
                        "[terminals]: too many terminals (max %d).", MC_MAX_TERMINALS); }
                    cfg->terminals[cfg->terminalCount++] = (int)t;
                    tok = strtok(NULL, ", \t");
                }
            } else {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "[terminals]: unknown key '%s' (expected 'enabled').", key);
            }
            break;

        case SEC_PERIPHERAL: {
            bool *target = NULL;
            if (str_ieq(periphName, "papertape-reader")) target = &cfg->ptreader_enabled;
            else if (str_ieq(periphName, "papertape-punch")) target = &cfg->ptpunch_enabled;
            else if (str_ieq(periphName, "lineprinter")) target = &cfg->lineprinter_enabled;
            else { fclose(f); return mc_err(err, errlen, path, lineno,
                "unknown peripheral '%s'. Known: papertape-reader, papertape-punch, lineprinter.",
                periphName); }
            if (str_ieq(keyl, "enabled")) {
                int b = parse_bool(val);
                if (b < 0) { fclose(f); return mc_err(err, errlen, path, lineno,
                    "[peripheral.%s] enabled = %s: use yes or no.", periphName, val); }
                *target = (b == 1);
            }
            /* other peripheral keys (type/format) accepted but not yet applied */
            break;
        }

        case SEC_BOOT:
            if (str_ieq(keyl, "device")) {
                /* forms: <type>.<wheel>.<unit> | bpun:FILE | aout:FILE */
                if (strncmp(val, "bpun:", 5) == 0) {
                    cfg->boot.is_disc = false;
                    cfg->boot.file_boot_type = BOOT_BPUN;
                    str_copy(cfg->boot.file, MC_PATH_LEN, val + 5);
                } else if (strncmp(val, "aout:", 5) == 0) {
                    cfg->boot.is_disc = false;
                    cfg->boot.file_boot_type = BOOT_AOUT;
                    str_copy(cfg->boot.file, MC_PATH_LEN, val + 5);
                } else {
                    char b[64]; str_copy(b, sizeof(b), val);
                    char *d1 = strchr(b, '.');
                    char *d2 = d1 ? strchr(d1 + 1, '.') : NULL;
                    if (!d1 || !d2) { fclose(f); return mc_err(err, errlen, path, lineno,
                        "[boot] device = %s: use <type>.<wheel>.<unit>, e.g. smd.0.0.", val); }
                    *d1 = '\0'; *d2 = '\0';
                    CtrlType bt = MC_CtrlTypeFromName(b);
                    if (bt == CTRL_NONE) { fclose(f); return mc_err(err, errlen, path, lineno,
                        "[boot] device: unknown controller type '%s'.", b); }
                    cfg->boot.is_disc = true;
                    cfg->boot.type = bt;
                    cfg->boot.wheel = (int)strtol(d1 + 1, NULL, 10);
                    cfg->boot.unit = (int)strtol(d2 + 1, NULL, 10);
                }
            } else {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "[boot]: unknown key '%s' (expected 'device').", key);
            }
            break;

        case SEC_RUNTIME:
            if (str_ieq(keyl, "telnet")) {
                cfg->runtime.telnet_port = (int)strtol(val, NULL, 10);
            } else if (str_ieq(keyl, "throttle")) {
                cfg->runtime.throttle_mhz = atof(val);
            } else if (str_ieq(keyl, "charset")) {
                str_copy(cfg->runtime.charset, sizeof(cfg->runtime.charset), val);
            } else if (str_ieq(keyl, "printdir")) {
                str_copy(cfg->runtime.printdir, MC_PATH_LEN, val);
            } else if (str_ieq(keyl, "tapedir")) {
                str_copy(cfg->runtime.tapedir, MC_PATH_LEN, val);
            } else if (str_ieq(keyl, "debugger")) {
                cfg->runtime.debugger_port = (int)strtol(val, NULL, 10);
            } else if (str_ieq(keyl, "trace")) {
                int b = parse_bool(val);
                cfg->runtime.trace = (b == 1);
            } else {
                fclose(f);
                return mc_err(err, errlen, path, lineno,
                    "[runtime]: unknown key '%s'.", key);
            }
            break;

        case SEC_NONE:
            fclose(f);
            return mc_err(err, errlen, path, lineno,
                "key '%s' appears before any [section].", key);

        default:
            break;
        }
    }

    fclose(f);
    return true;
}

/* ------------------------------------------------------------------ */
/* Validation                                                          */
/* ------------------------------------------------------------------ */
/* Reserved IOX ranges of the always-present core devices, for overlap checks. */
typedef struct { uint16_t base; int span; const char *name; } IoxRange;
static const IoxRange g_core_ranges[] = {
    { 001570, 8, "rtc" },
    { 000300, 8, "console" },
    { 000400, 4, "papertape-reader" },
    { 000410, 4, "papertape-punch" },
    { 000430, 4, "lineprinter" },
};

static bool ranges_overlap(uint16_t a, int aspan, uint16_t b, int bspan)
{
    uint32_t a0 = a, a1 = a + (uint32_t)aspan;
    uint32_t b0 = b, b1 = b + (uint32_t)bspan;
    return a0 < b1 && b0 < a1;
}

bool MachineConfig_Validate(const MachineConfig *cfg, char *err, size_t errlen)
{
    if (!cfg) return false;
    const char *path = cfg->loaded_from_file ? cfg->source_path : "(defaults)";

    /* Gather IOX ranges of every enabled controller, checking overlap as we go
     * (against core devices and each other). */
    IoxRange used[MC_MAX_CONTROLLERS + 8];
    int usedCount = 0;
    for (size_t i = 0; i < sizeof(g_core_ranges)/sizeof(g_core_ranges[0]); i++)
        used[usedCount++] = g_core_ranges[i];

    for (int i = 0; i < cfg->controllerCount; i++) {
        const MC_Controller *c = &cfg->controllers[i];
        if (!c->enabled) continue;
        const ControllerDescriptor *d = MC_DescriptorForType(c->type);
        if (!d) continue;
        if (c->wheel < d->min_wheel || c->wheel > d->max_wheel)
            return mc_err(err, errlen, path, 0,
                "%s controller on thumbwheel %d out of range (%d-%d).",
                d->name, c->wheel, d->min_wheel, d->max_wheel);

        uint16_t base = d->iox_base[c->wheel];
        for (int j = 0; j < usedCount; j++) {
            if (ranges_overlap(base, d->iox_span, used[j].base, used[j].span))
                return mc_err(err, errlen, path, 0,
                    "IOX address clash: '%s' on thumbwheel %d (0%o-0%o) overlaps '%s'. "
                    "Move one controller to another thumbwheel.",
                    d->name, c->wheel, base, base + d->iox_span - 1, used[j].name);
        }
        used[usedCount].base = base; used[usedCount].span = d->iox_span;
        used[usedCount].name = d->name; usedCount++;

        /* SCSI: booting/using needs implemented media (hdd only today) */
        for (int s = 0; s < d->disk_slots; s++) {
            if (c->disks[s].present && c->type == CTRL_SCSI &&
                c->disks[s].media != SCSI_UNIT_HDD) {
                /* non-hdd media is accepted but flagged as not-yet-usable at
                 * boot time; not a hard error here. */
            }
        }
    }

    /* Boot device sanity. */
    if (cfg->boot.is_disc) {
        const ControllerDescriptor *d = MC_DescriptorForType(cfg->boot.type);
        if (!d || !d->bootable)
            return mc_err(err, errlen, path, 0,
                "[boot] device: '%s' is not a bootable controller.",
                MC_CtrlTypeName(cfg->boot.type));
        /* find the enabled controller instance */
        const MC_Controller *bc = NULL;
        for (int i = 0; i < cfg->controllerCount; i++)
            if (cfg->controllers[i].type == cfg->boot.type &&
                cfg->controllers[i].wheel == cfg->boot.wheel) { bc = &cfg->controllers[i]; break; }
        if (!bc || !bc->enabled)
            return mc_err(err, errlen, path, 0,
                "[boot] device = %s.%d.%d: no enabled %s controller on thumbwheel %d.",
                d->name, cfg->boot.wheel, cfg->boot.unit, d->name, cfg->boot.wheel);
        if (cfg->boot.unit < 0 || cfg->boot.unit >= d->disk_slots)
            return mc_err(err, errlen, path, 0,
                "[boot] device = %s.%d.%d: unit %d out of range (0-%d).",
                d->name, cfg->boot.wheel, cfg->boot.unit, cfg->boot.unit, d->disk_slots - 1);
        if (!bc->disks[cfg->boot.unit].present)
            return mc_err(err, errlen, path, 0,
                "[boot] device = %s.%d.%d: that unit has no image. "
                "Add 'disk%d = FILE' to [controller.%s.%d], or boot a different device.",
                d->name, cfg->boot.wheel, cfg->boot.unit, cfg->boot.unit, d->name, cfg->boot.wheel);
        if (cfg->boot.type == CTRL_SCSI && bc->disks[cfg->boot.unit].media != SCSI_UNIT_HDD)
            return mc_err(err, errlen, path, 0,
                "[boot] device = scsi.%d.%d: media is '%s' but only 'hdd' can boot.",
                cfg->boot.wheel, cfg->boot.unit,
                SCSI_UnitTypeName(bc->disks[cfg->boot.unit].media));
    }

    return true;
}

/* ------------------------------------------------------------------ */
/* Pretty printer (--show-config)                                      */
/* ------------------------------------------------------------------ */
void MachineConfig_Print(const MachineConfig *cfg, FILE *out)
{
    if (!cfg || !out) return;
    fprintf(out, "Machine configuration (%s):\n",
            cfg->loaded_from_file ? cfg->source_path : "built-in defaults");
    fprintf(out, "  CPU: ND-%d\n", cfg->cpu_type);

    fprintf(out, "  Controllers:\n");
    for (int i = 0; i < cfg->controllerCount; i++) {
        const MC_Controller *c = &cfg->controllers[i];
        const ControllerDescriptor *d = MC_DescriptorForType(c->type);
        if (!d) continue;
        uint16_t base = d->iox_base[c->wheel];
        fprintf(out, "    %-7s wheel %d  IOX 0%o-0%o  %s\n",
                d->name, c->wheel, base, base + d->iox_span - 1,
                c->enabled ? "enabled" : "disabled");
        if (d->is_disc) {
            for (int s = 0; s < d->disk_slots; s++) {
                if (c->disks[s].present) {
                    if (c->type == CTRL_SCSI)
                        fprintf(out, "        disk%d = %s:%s\n", s,
                                SCSI_UnitTypeName(c->disks[s].media), c->disks[s].image);
                    else
                        fprintf(out, "        disk%d = %s\n", s, c->disks[s].image);
                }
            }
        } else if (c->type == CTRL_HDLC) {
            if (c->hdlc_is_server)
                fprintf(out, "        server port %d\n", c->hdlc_port);
            else
                fprintf(out, "        client %s:%d\n", c->hdlc_host, c->hdlc_port);
        }
    }

    fprintf(out, "  Terminals: console(0)");
    for (int i = 0; i < cfg->terminalCount; i++)
        fprintf(out, ", %d", cfg->terminals[i]);
    fprintf(out, "\n");

    fprintf(out, "  Peripherals: papertape-reader=%s papertape-punch=%s lineprinter=%s\n",
            cfg->ptreader_enabled ? "on" : "off",
            cfg->ptpunch_enabled ? "on" : "off",
            cfg->lineprinter_enabled ? "on" : "off");

    if (cfg->boot.is_disc)
        fprintf(out, "  Boot: %s.%d.%d\n",
                MC_CtrlTypeName(cfg->boot.type), cfg->boot.wheel, cfg->boot.unit);
    else
        fprintf(out, "  Boot: %s:%s\n",
                cfg->boot.file_boot_type == BOOT_BPUN ? "bpun" : "aout", cfg->boot.file);
}

/* ------------------------------------------------------------------ */
/* INI serializer (--write-config / web Download-.ini)                 */
/* ------------------------------------------------------------------ */
bool MachineConfig_WriteFile(const MachineConfig *cfg, const char *path,
                             char *err, size_t errlen)
{
    if (!cfg || !path) return false;
    FILE *f = fopen(path, "w");
    if (!f)
        return mc_err(err, errlen, path, 0, "cannot write config file (%s).", strerror(errno));

    fprintf(f, "# nd100x machine configuration (generated)\n");
    fprintf(f, "# Toggle a device with 'enabled = yes|no'. Sections are [type.thumbwheel].\n\n");

    fprintf(f, "[machine]\n");
    fprintf(f, "cpu = %d\n\n", cfg->cpu_type);

    for (int i = 0; i < cfg->controllerCount; i++) {
        const MC_Controller *c = &cfg->controllers[i];
        const ControllerDescriptor *d = MC_DescriptorForType(c->type);
        if (!d) continue;
        fprintf(f, "[controller.%s.%d]\n", d->name, c->wheel);
        fprintf(f, "enabled = %s\n", c->enabled ? "yes" : "no");
        if (d->is_disc) {
            for (int s = 0; s < d->disk_slots; s++) {
                if (!c->disks[s].present) continue;
                if (c->type == CTRL_SCSI)
                    fprintf(f, "disk%d = %s:%s\n", s,
                            SCSI_UnitTypeName(c->disks[s].media), c->disks[s].image);
                else
                    fprintf(f, "disk%d = %s\n", s, c->disks[s].image);
            }
        } else if (c->type == CTRL_HDLC) {
            fprintf(f, "mode = %s\n", c->hdlc_is_server ? "server" : "client");
            if (!c->hdlc_is_server && c->hdlc_host[0])
                fprintf(f, "host = %s\n", c->hdlc_host);
            fprintf(f, "port = %d\n", c->hdlc_port);
        }
        fprintf(f, "\n");
    }

    fprintf(f, "[terminals]\n");
    fprintf(f, "enabled = ");
    for (int i = 0; i < cfg->terminalCount; i++)
        fprintf(f, "%s%d", i ? ", " : "", cfg->terminals[i]);
    fprintf(f, "\n\n");

    fprintf(f, "[peripheral.papertape-reader]\nenabled = %s\n\n",
            cfg->ptreader_enabled ? "yes" : "no");
    fprintf(f, "[peripheral.papertape-punch]\nenabled = %s\n\n",
            cfg->ptpunch_enabled ? "yes" : "no");
    fprintf(f, "[peripheral.lineprinter]\nenabled = %s\n\n",
            cfg->lineprinter_enabled ? "yes" : "no");

    fprintf(f, "[boot]\n");
    if (cfg->boot.is_disc)
        fprintf(f, "device = %s.%d.%d\n\n",
                MC_CtrlTypeName(cfg->boot.type), cfg->boot.wheel, cfg->boot.unit);
    else
        fprintf(f, "device = %s:%s\n\n",
                cfg->boot.file_boot_type == BOOT_BPUN ? "bpun" : "aout", cfg->boot.file);

    fprintf(f, "[runtime]\n");
    if (cfg->runtime.telnet_port)   fprintf(f, "telnet = %d\n", cfg->runtime.telnet_port);
    if (cfg->runtime.throttle_mhz)  fprintf(f, "throttle = %g\n", cfg->runtime.throttle_mhz);
    if (cfg->runtime.charset[0])    fprintf(f, "charset = %s\n", cfg->runtime.charset);
    if (cfg->runtime.printdir[0])   fprintf(f, "printdir = %s\n", cfg->runtime.printdir);
    if (cfg->runtime.tapedir[0])    fprintf(f, "tapedir = %s\n", cfg->runtime.tapedir);
    if (cfg->runtime.debugger_port) fprintf(f, "debugger = %d\n", cfg->runtime.debugger_port);
    if (cfg->runtime.trace)         fprintf(f, "trace = on\n");

    fclose(f);
    return true;
}

/* ------------------------------------------------------------------ */
/* CPU number -> CpuType                                               */
/* ------------------------------------------------------------------ */
bool MachineConfig_CpuTypeForNumber(int cpuNumber, int *outType)
{
    switch (cpuNumber) {
    case 100: if (outType) *outType = ND100;   return true;
    case 110: if (outType) *outType = ND110;   return true;
    case 120: if (outType) *outType = ND120CX; return true;
    default:  return false;
    }
}
