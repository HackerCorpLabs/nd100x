/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2026 Ronny Hansen
 *
 * Unit tests for the [machine] fpp INI key in src/machine/machine_config.c:
 * defaults, write/read round trip, and rejection of invalid values.
 *
 * machine_config.c is linked in DIRECTLY; it has no machine/CPU dependencies
 * beyond headers, so no devices or disk images are involved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "machine_config.h"

/* machine_config.c calls these two SCSI helpers for the disk media names;
 * they are replicated here (1:1 with deviceSCSI.c) so the whole SCSI device
 * does not have to be linked in. */
SCSIUnitType SCSI_ParseUnitType(const char *name)
{
    if (!name)
        return SCSI_UNIT_NONE;
    if (strcmp(name, "hdd") == 0)
        return SCSI_UNIT_HDD;
    if (strcmp(name, "tape") == 0)
        return SCSI_UNIT_TAPE;
    if (strcmp(name, "cdrom") == 0)
        return SCSI_UNIT_CDROM;
    if (strcmp(name, "floppy") == 0)
        return SCSI_UNIT_FLOPPY;
    return SCSI_UNIT_NONE;
}

const char *SCSI_UnitTypeName(SCSIUnitType type)
{
    switch (type)
    {
    case SCSI_UNIT_HDD:    return "hdd";
    case SCSI_UNIT_TAPE:   return "tape";
    case SCSI_UNIT_CDROM:  return "cdrom";
    case SCSI_UNIT_FLOPPY: return "floppy";
    default:               return "none";
    }
}

static int mc_total;
static int mc_failed;

static void mc_check(const char *name, long exp, long got)
{
    mc_total++;
    if (exp != got) {
        printf("  FAIL  %-40s expected %ld, got %ld\n", name, exp, got);
        mc_failed++;
    }
}

static void mc_check_bool(const char *name, int cond)
{
    mc_total++;
    if (!cond) {
        printf("  FAIL  %s\n", name);
        mc_failed++;
    }
}

int main(void)
{
    MachineConfig cfg;
    char err[MC_ERR_LEN];
    char dir_tmpl[] = "/tmp/nd100x_mc_testXXXXXX";
    char *dir = mkdtemp(dir_tmpl);
    char path[300];

    if (!dir) {
        printf("mkdtemp failed\n");
        return 1;
    }

    /* Default is the standard 48-bit FPP. */
    MachineConfig_SetDefaults(&cfg);
    mc_check("default fpp_bits", 48, cfg.fpp_bits);

    /* Write a config with fpp = 32 and read it back. */
    cfg.fpp_bits = 32;
    snprintf(path, sizeof(path), "%s/fpp32.ini", dir);
    mc_check_bool("WriteFile(fpp=32)",
                  MachineConfig_WriteFile(&cfg, path, err, sizeof(err)));

    MachineConfig_InitBaseline(&cfg);
    mc_check_bool("LoadFile(fpp=32)",
                  MachineConfig_LoadFile(&cfg, path, err, sizeof(err)));
    mc_check("round-tripped fpp_bits", 32, cfg.fpp_bits);

    /* A config without an fpp key keeps the 48-bit default. */
    snprintf(path, sizeof(path), "%s/nofpp.ini", dir);
    {
        FILE *f = fopen(path, "w");
        mc_check_bool("write nofpp.ini", f != NULL);
        if (f) { fprintf(f, "[machine]\ncpu = 110\n"); fclose(f); }
    }
    MachineConfig_InitBaseline(&cfg);
    mc_check_bool("LoadFile(no fpp key)",
                  MachineConfig_LoadFile(&cfg, path, err, sizeof(err)));
    mc_check("fpp_bits stays default", 48, cfg.fpp_bits);
    mc_check("cpu_type read", 110, cfg.cpu_type);

    /* fpp = 99 must be rejected with a clear error. */
    snprintf(path, sizeof(path), "%s/bad.ini", dir);
    {
        FILE *f = fopen(path, "w");
        mc_check_bool("write bad.ini", f != NULL);
        if (f) { fprintf(f, "[machine]\nfpp = 99\n"); fclose(f); }
    }
    MachineConfig_InitBaseline(&cfg);
    err[0] = '\0';
    mc_check_bool("LoadFile(fpp=99) rejected",
                  !MachineConfig_LoadFile(&cfg, path, err, sizeof(err)));
    mc_check_bool("fpp=99 error mentions 'fpp'", strstr(err, "fpp") != NULL);

    printf("machine_config tests: %d checks, %d failed\n", mc_total, mc_failed);
    return mc_failed ? 1 : 0;
}
