/*
 * machine_config_apply.h - turn a MachineConfig into an actual machine.
 *
 * machine_config.h calls itself "the single in-memory model of what machine to
 * build". This is the other half: the code that BUILDS it. Until now that code
 * was a static function inside the native CLI frontend, which meant the model
 * was only single-source-of-truth for a caller that happened to be nd100x.c.
 * The browser parsed an INI, validated it, printed a friendly error - and then
 * threw the config away and built a hardcoded machine anyway.
 *
 * Nothing here needs a command line or a host filesystem beyond what the mount
 * helpers already need: mount_smd(), mount_floppy(), mount_winchester(),
 * mount_scsi() and machine_add_hdlc() all live in the machine library, and the
 * wasm build already calls the first of them.
 */
#ifndef MACHINE_CONFIG_APPLY_H
#define MACHINE_CONFIG_APPLY_H

#include "machine_config.h"

/* Things a command line may already have decided, which the .ini must not then
 * overwrite. The native frontend has --fpp and --rtc flags and the rule is
 * "the CLI wins over the .ini key", mirroring how --memory beats memory=.
 * A caller with no command line (the browser) passes zeroes and the config
 * decides everything. */
typedef struct MachineConfigApplyOpts {
    int fpp_already_set;   /* non-zero: leave CurrentFPPType alone */
    int rtc_already_set;   /* non-zero: leave the RTC time base alone */
} MachineConfigApplyOpts;

/* Build the config-driven parts of the machine: CPU type, FPP width, RTC time
 * base, terminals, disc controllers with their mounted images, and HDLC.
 *
 * Call AFTER machine_init(): the core devices (RTC, console, floppy DMA, SMD,
 * tape, printer) are added by DeviceManager_AddAllDevices inside it, and this
 * adds the configured parts on top. <opts> may be NULL, which means "nothing
 * was decided elsewhere". */
void MachineConfig_Apply(const MachineConfig *mc, const MachineConfigApplyOpts *opts);

#endif /* MACHINE_CONFIG_APPLY_H */
