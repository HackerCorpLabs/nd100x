/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100x project and the RetroCore project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the nd100em
 * distribution in the file COPYING); if not, see <http://www.gnu.org/licenses/>.
 */
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "machine_types.h"
#include "machine_protos.h"

#include "../devices/devices_types.h"
#include "../devices/devices_protos.h"

#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"

#include "../../external/libsymbols/include/symbols.h"
#include "../../external/libsymbols/include/aout.h"


// Global arrays for mounted drive information
MountedDriveInfo_t* floppy_drives = NULL;
MountedDriveInfo_t* smd_drives = NULL;


const char* boot_type_str[] = {
    "none",
    "bpun",
    "aout",
    "bp",
    "floppy",
    "smd"
};


// Initialize drive arrays
void init_drive_arrays() {
    if (!floppy_drives) {
        floppy_drives = calloc(3, sizeof(MountedDriveInfo_t));
    }
    if (!smd_drives) {
        smd_drives = calloc(4, sizeof(MountedDriveInfo_t));
    }
}

// Clean up drive arrays
static void cleanup_drive_arrays() {
    if (floppy_drives) {
        free(floppy_drives);
        floppy_drives = NULL;
    }
    if (smd_drives) {
        free(smd_drives);
        smd_drives = NULL;
    }
}

void
machine_init (bool debuggerEnabled)
{

    // Initialize drive arrays
    init_drive_arrays();

    // Initialize the CPU
    cpu_init(debuggerEnabled);

    // Initialize the CPU debugger (if enabled, starts the debugger thread)
    init_cpu_debugger();

    // Initialize IO devices
    IO_Init();

    // Set the CPU to RUN mode
    set_cpu_run_mode(CPU_RUNNING);
}

void 
cleanup_machine (void)
{
    cleanup_cpu();
    IO_Destroy();    

    // Unmount all drives to prevent memory leaks
	
	// Unmount all floppy drives
	if (floppy_drives) {
		for (int i = 0; i < 3; i++) {
			if (floppy_drives[i].name[0] != '\0') {
				unmount_drive(DRIVE_FLOPPY, i);
			}
		}
	}
	
	// Unmount all SMD drives
	if (smd_drives) {
		for (int i = 0; i < 4; i++) {
			if (smd_drives[i].name[0] != '\0') {
				unmount_drive(DRIVE_SMD, i);
			}
		}
	}
	
	// Clean up drive arrays
	cleanup_drive_arrays();

}




/// @brief Do NOT call from debugger thread
/// @param ticks Number of ticks to run the CPU. Use -1 for infinite.
void  machine_run (int ticks)
{    
    // Run the CPU until it stops but also handle debugger requests
    while (get_cpu_run_mode() != CPU_SHUTDOWN)
    {    
        ticks = cpu_run(ticks);  

        // Check if DAP adapter has requested a pause
        if (gDebuggerEnabled)
        {
            if (get_debugger_request_pause())
            {
                if (!get_debugger_control_granted())
                {
                    set_debugger_control_granted(true);
                }
            }

#ifdef __EMSCRIPTEN__
            /* WASM: if debugger has control, exit immediately to avoid
               busy-looping in single-threaded environment */
            if (get_debugger_control_granted())
            {
                return;
            }
#else
            usleep(100000); // Sleep 100ms
#endif
            if ((get_cpu_run_mode() == CPU_PAUSED) || (get_cpu_run_mode() == CPU_BREAKPOINT))
            {
                //if (get_debugger_control_granted()) return; // exit back to main loop to handle keyboard input
                return; // exit back to main loop to handle keyboard input
            }
        }

        if (ticks == 0) return; // No more ticks to run
    } 
}

void machine_stop()
{
    // Stop the CPU
    set_cpu_run_mode(CPU_STOPPED);
}


/* 
 * 
 * 
 *  CONFIGURATION HANDLING
 * 
 * 
 */

 BOOT_TYPE	BootType; /* Variable holding the way we should boot up the emulator */
 
 void  setdefaultconfig (void)
 {
     // Set default configuration	
     BootType = BOOT_SMD;
     STARTADDR = 0;
     DISASM = 0;
 }
 


/* 
 * 
 * 
 *  BOOT HANDLING
 * 
 * 
 */


/// @brief Callback function to write to memory. Used by aout loader.  
/// @param address Address to write to
/// @param value Value to write
void write_memory(uint16_t address, uint16_t value)
{
    // Write the value to the memory at the given address
    WritePhysicalMemory(address, value, false);
    if (DISASM) disasm_addword(address, value);
}


void mount_floppy(const char *imageFile, int unit)
{
    const char *floppy_img = imageFile ? imageFile : "FLOPPY.IMG";
    
    // if file exists  mount it
    if (access(floppy_img, F_OK) != -1) {
        mount_drive(DRIVE_FLOPPY, unit, "md5-unknown", "Boot Floppy", "Boot floppy image", floppy_img);
    }
}


void mount_smd(const char *imageFile, int unit)
{
    char path[256];
    sprintf(path,"SMD%d.IMG",unit);

    const char *smd_img = imageFile ? imageFile : path;
    
    // if file exists  mount it
    if (access(smd_img, F_OK) != -1) {
        if (unit ==0)
        {
            mount_drive(DRIVE_SMD, unit, "md5-unknown", "Boot SMD", "Boot SMD image", smd_img);
        }
        else
        {
            mount_drive(DRIVE_SMD, unit, "md5-unknown", "DATA SMD", "DATA SMD image", smd_img);
        }
    }
}


// As a default, mount floppy and SMD drives (IF they exists)
void autoMountDrives()
{
    // Automount floppy if file "FLOPPY.IMG" exists
    if (!isMounted(DRIVE_FLOPPY,0))
    {
        mount_floppy(NULL,0);
    }

    // Automount SMD files
    for (int i=0; i<4;i++)
    {
        if (!isMounted(DRIVE_SMD,i))
        {
            mount_smd(NULL,i);
        }
    }

}

 void program_load(BOOT_TYPE bootType, const char *imageFile, bool verbose)
 {
     int bootAddress;
     int result;
     STARTADDR = 0;

     switch (bootType)
     {
     case BOOT_BP:
         bootAddress = bp_load(imageFile);
         if (bootAddress < 0)
         {
             printf("Error loading BP file '%s'\n", imageFile);
#ifdef __EMSCRIPTEN__
             return;
#else
             exit(1);
#endif
         }
         break;

     case BOOT_BPUN:
         bootAddress = LoadBPUN(imageFile,verbose);
         if (bootAddress < 0)
         {
             printf("Error loading BPUN file '%s'\n", imageFile);
#ifdef __EMSCRIPTEN__
             return;
#else
             exit(1);
#endif
         }
         STARTADDR = bootAddress;
         break;
    case BOOT_AOUT:
        bootAddress = load_aout(imageFile, verbose, write_memory);
        if (bootAddress < 0)
        {
            printf("Error loading AOUT file '%s'\n", imageFile);
#ifdef __EMSCRIPTEN__
            return;
#else
            exit(1);
#endif
        }
        STARTADDR = bootAddress;
        break;
     case BOOT_FLOPPY:
        // Record mount state for UI/menus; device still boots via BPUN for now
         mount_floppy(imageFile,0);

         bootAddress = LoadBPUN(imageFile, verbose);
         if (bootAddress < 0)
         {
             printf("Error loading BPUN file\n");
#ifdef __EMSCRIPTEN__
             return;
#else
             exit(1);
#endif
         }

         STARTADDR = bootAddress;

         //gPC = (CONFIG_OK) ? bootaddress : 0;

         /*
         result = sectorread(0, 0, 1, (ushort *)&VolatileMemory);
         if (result < 0) {
             printf("Error reading from floppy\n");
             exit(1);
         }
         gPC = 0;
         */
         break;
     case BOOT_SMD:

        // Ensure unit 0 is mounted before we ask the SMD device to memory-boot
        mount_smd(imageFile, 0);

         bootAddress = DeviceManager_Boot(01540);
         if (bootAddress < 0)
         {
             printf("Error booting from SMD device\n");
#ifdef __EMSCRIPTEN__
             return;
#else
             exit(10);
#endif
         }
         STARTADDR = bootAddress;
         break;
     }

     autoMountDrives();
 }
 
 /** DEVICE MOUNTING */

 // Return true if the drive is already mounted
 bool isMounted(DRIVE_TYPE drive_type, int unit)
 {
    MountedDriveInfo_t* drives = NULL;
    int max_units = 0;

    // Determine which array to use and max units
    if (drive_type == DRIVE_SMD) {
        drives = smd_drives;
        max_units = 4;  // SMD has units 0-3
    } else if (drive_type == DRIVE_FLOPPY) {
        drives = floppy_drives;
        max_units = 3;  // Floppy has units 0-2
    } else {
        //printf("Error: Invalid drive type\n");
        return false;
    }
    
    // Check if unit is valid
    if (unit < 0 || unit >= max_units) {
        return false;
    }

    // Safety check for NULL array
    if (!drives) {
        return false;
    }

    return drives[unit].is_mounted;
 }

 // Mount a drive to the specified unit
void mount_drive(DRIVE_TYPE drive_type, int unit, const char *md5, const char *name, const char *description, const char *image_path) {
    MountedDriveInfo_t* drives = NULL;
    int max_units = 0;

    // Determine which array to use and max units
    if (drive_type == DRIVE_SMD) {
        drives = smd_drives;
        max_units = 4;  // SMD has units 0-3
    } else if (drive_type == DRIVE_FLOPPY) {
        drives = floppy_drives;
        max_units = 3;  // Floppy has units 0-2
    } else {
        return;
    }

    // Check if unit is valid
    if (unit < 0 || unit >= max_units) {
        return;
    }

    // Lazy init if drive arrays not yet allocated
    if (!drives) {
        init_drive_arrays();
        drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
        if (!drives) {
            return;
        }
    }
    
    // Handle image path (HTTP download or local file)
    if (image_path) {
        // Set block size based on drive type
        if (drive_type == DRIVE_SMD) {
            drives[unit].block_size = 1024;  // 1KB for SMD
        } else {
            // For floppy, we'll use 512 bytes as default, but could be determined from file
            drives[unit].block_size = 512;   // 512 bytes for floppy
        }
        
        // Store the image path
        strncpy(drives[unit].image_path, image_path, sizeof(drives[unit].image_path) - 1);
        drives[unit].image_path[sizeof(drives[unit].image_path) - 1] = '\0';
        
        // Check if it's an HTTP URL (case insensitive)
        if (strncasecmp(image_path, "http", 4) == 0) {
            //printf("Downloading image from: %s\n", image_path);
            char* image_data = download_file(image_path);
            if (image_data) {
                drives[unit].is_remote = true;
                drives[unit].data.remote_data = image_data;
                drives[unit].data_size = get_downloaded_size();  // Use actual size instead of strlen()
                //printf("Downloaded %zu bytes of image data\n", drives[unit].data_size);
            } else {
                //printf("Error: Failed to download image from %s\n", image_path);
                return;
            }
        } else {

            
            drives[unit].is_writeprotected = false;

            // Local file - open for read-write binary
            FILE* file = fopen(image_path, "rb+");
            if (!file) {
                int saved_errno = errno;
                // If we dont have write access, try to open the file for read-only
                if (saved_errno == EACCES || saved_errno == EROFS || saved_errno == EPERM) {
                    file = fopen(image_path, "rb");  // fallback
                    if (file) drives[unit].is_writeprotected = true;
                }
                if (!file) {
                    return;
                }
            }
            if (file) {
                // Get file size
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                drives[unit].is_remote = false;
                drives[unit].data.local_file = file;
                drives[unit].data_size = (size_t)file_size;

                //printf("Opened local file: %s (size: %ld bytes)\n", image_path, file_size);
            } else {
                printf("mount_drive: Failed to open %s\n", image_path);
                drives[unit].is_mounted = false;
                return;
            }
        }
    }
    
    // Mount the drive
    drives[unit].is_mounted = true;

    strncpy(drives[unit].md5, md5, sizeof(drives[unit].md5) - 1);
    drives[unit].md5[sizeof(drives[unit].md5) - 1] = '\0';
    
    strncpy(drives[unit].name, name, sizeof(drives[unit].name) - 1);
    drives[unit].name[sizeof(drives[unit].name) - 1] = '\0';
    
    strncpy(drives[unit].description, description, sizeof(drives[unit].description) - 1);
    drives[unit].description[sizeof(drives[unit].description) - 1] = '\0';
    
#if _debug_    
    printf("Mounted %s to %s unit %d:\n", 
           drive_type == DRIVE_SMD ? "SMD" : "floppy",
           drive_type == DRIVE_SMD ? "SMD" : "floppy",
           unit);
    printf("  Name: %s\n", name);
    printf("  Description: %s\n", description);
    printf("  MD5: %s\n", md5);
    printf("  Image Path: %s\n", image_path ? image_path : "None");
#endif

}

// Unmount a drive from the specified unit
void unmount_drive(DRIVE_TYPE drive_type, int unit) {
    MountedDriveInfo_t* drives = NULL;
    int max_units = 0;
    
    // Determine which array to use and max units
    if (drive_type == DRIVE_SMD) {
        drives = smd_drives;
        max_units = 4;  // SMD has units 0-3
    } else if (drive_type == DRIVE_FLOPPY) {
        drives = floppy_drives;
        max_units = 3;  // Floppy has units 0-2
    } else {
        printf("Error: Invalid drive type\n");
        return;
    }
    
    // Check if unit is valid
    if (unit < 0 || unit >= max_units) {
        printf("Error: Invalid unit %d for drive type %d\n", unit, drive_type);
        return;
    }
    
    // Check if array is initialized
    if (!drives) {
        printf("Error: Drive arrays not initialized\n");
        return;
    }
    
    // Check if drive is mounted
    if (drives[unit].name[0] == '\0') {
        printf("Error: No drive mounted on %s unit %d\n", 
               drive_type == DRIVE_SMD ? "SMD" : "floppy", unit);
        return;
    }
    
    // Unmount the drive
    printf("Unmounting %s from %s unit %d:\n", 
           drives[unit].name,
           drive_type == DRIVE_SMD ? "SMD" : "floppy",
           unit);
    
    // Clean up data based on type
    if (drives[unit].is_remote) {
        // Free downloaded remote data
        if (drives[unit].data.remote_data) {
            free(drives[unit].data.remote_data);
            drives[unit].data.remote_data = NULL;
        }
    } else {
        // Close local file
        if (drives[unit].data.local_file) {
            fclose(drives[unit].data.local_file);
            drives[unit].data.local_file = NULL;
        }
    }
    
    // Clear the drive entry
    drives[unit].is_mounted = false;
    drives[unit].md5[0] = '\0';
    drives[unit].name[0] = '\0';
    drives[unit].description[0] = '\0';
    drives[unit].image_path[0] = '\0';
    drives[unit].is_remote = false;
    drives[unit].data_size = 0;
    drives[unit].block_size = 0;
}

// List mounted drives for the specified drive type
MountedDriveInfo_t* list_mount(DRIVE_TYPE drive_type) {
    if (drive_type == DRIVE_SMD) {
        return smd_drives;
    } else if (drive_type == DRIVE_FLOPPY) {
        return floppy_drives;
    } else {
        return NULL;  // Invalid drive type
    }
}

// Callback-based block READ for block devices
int machine_block_read(Device *device, uint8_t *buffer, size_t size, uint32_t blockAddress, int unit) {
    if (!device || !buffer || size == 0) return -1;

    DRIVE_TYPE drive_type = (device->type == DEVICE_TYPE_DISC_SMD) ? DRIVE_SMD : DRIVE_FLOPPY;
    MountedDriveInfo_t *drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
    if (!drives) return -1;

    // block size is determined by the device; size is number of blocks
    size_t bytes = size * device->blockSizeBytes;
    size_t offset = (size_t)blockAddress * device->blockSizeBytes;

    MountedDriveInfo_t *entry = &drives[unit];
    if (!entry->is_mounted) return -1; // not mounted    

    if (entry->is_remote) {
        if (!entry->data.remote_data) return -1;
        if (offset >= entry->data_size) return 0;
        size_t to_copy = bytes;
        if (offset + to_copy > entry->data_size) {
            to_copy = entry->data_size - offset;
        }
        memcpy(buffer, entry->data.remote_data + offset, to_copy);
        if (to_copy < bytes) {
            memset(buffer + to_copy, 0, bytes - to_copy);
        }
    } else {
        if (!entry->data.local_file) return -1;
        if (fseek(entry->data.local_file, (long)offset, SEEK_SET) != 0) return -1;
        size_t read_bytes = fread(buffer, 1, bytes, entry->data.local_file);
        if (read_bytes < bytes) {
            memset(buffer + read_bytes, 0, bytes - read_bytes);
        }
    }
    return (int)size; // number of blocks
}

// Callback-based block WRITE for block devices
int machine_block_write(Device *device, const uint8_t *buffer, size_t size, uint32_t blockAddress, int unit) {
    if (!device || !buffer || size == 0) return -1;

    DRIVE_TYPE drive_type = (device->type == DEVICE_TYPE_DISC_SMD) ? DRIVE_SMD : DRIVE_FLOPPY;
    MountedDriveInfo_t *drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
    if (!drives) return -1;

    // block size is determined by the device; size is number of blocks
    size_t bytes = size * device->blockSizeBytes;
    size_t offset = (size_t)blockAddress * device->blockSizeBytes;

    MountedDriveInfo_t *entry = &drives[unit];
    if (!entry->is_mounted) return -1; // not mounted        

    if (entry->is_remote) {
        // For remote images in-memory, allow write if buffer exists and fits
        if (!entry->data.remote_data) return -1;
        if (offset + bytes > entry->data_size) return -1; // out of bounds
        memcpy(entry->data.remote_data + offset, buffer, bytes);
    } else {
        if (!entry->data.local_file) return -1;
        if (fseek(entry->data.local_file, (long)offset, SEEK_SET) != 0) return -1;
        fwrite(buffer, 1, bytes, entry->data.local_file);
        fflush(entry->data.local_file);
    }
    return (int)size;
}

// Callback-based DISK INFO for block devices
// Needed to retrive info about image size and if its write protected
int machine_block_disk_info(Device *device, size_t *image_size, bool *is_write_protected, int unit) {
    if (!device) return -1;

    // Set
    *image_size = 0;
    *is_write_protected = true;

    DRIVE_TYPE drive_type = (device->type == DEVICE_TYPE_DISC_SMD) ? DRIVE_SMD : DRIVE_FLOPPY;
    MountedDriveInfo_t *drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
    if (!drives) return -1;

    MountedDriveInfo_t *entry = &drives[unit];
    if (!entry->is_mounted) return -1; // not mounted        

    *image_size = entry->data_size;

    // Remote files are always NOT write protected (as they are buffered in memory)
    // Local files are write protected if we dont have access to write to the file
    if (entry->is_remote) {
        *is_write_protected = false;
    }
    else {
        *is_write_protected = entry->is_writeprotected;
    }

    return 0;
}
