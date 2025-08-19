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
    // Allocate and initialize floppy drives array (2 units)
    floppy_drives = malloc(2 * sizeof(MountedDriveInfo_t));
    if (floppy_drives) {
        for (int i = 0; i < 2; i++) {
            floppy_drives[i].md5[0] = '\0';
            floppy_drives[i].name[0] = '\0';
            floppy_drives[i].description[0] = '\0';
            floppy_drives[i].image_path[0] = '\0';
            floppy_drives[i].is_remote = false;
            floppy_drives[i].data.local_file = NULL;
            floppy_drives[i].data_size = 0;
            floppy_drives[i].block_size = 0;
        }
    }
    
    // Allocate and initialize SMD drives array (4 units)
    smd_drives = malloc(4 * sizeof(MountedDriveInfo_t));
    if (smd_drives) {
        for (int i = 0; i < 4; i++) {
            smd_drives[i].md5[0] = '\0';
            smd_drives[i].name[0] = '\0';
            smd_drives[i].description[0] = '\0';
            smd_drives[i].image_path[0] = '\0';
            smd_drives[i].is_remote = false;
            smd_drives[i].data.local_file = NULL;
            smd_drives[i].data_size = 0;
            smd_drives[i].block_size = 0;
        }
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
		for (int i = 0; i < 2; i++) {
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
            
            usleep(100000); // Sleep 100ms
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
             exit(1);
         }
         break;
 
     case BOOT_BPUN:		
         bootAddress = LoadBPUN(imageFile,verbose);
         if (bootAddress < 0)
         {
             printf("Error loading BPUN file '%s'\n", imageFile);
             exit(1);
         }
         STARTADDR = bootAddress;
         break;
    case BOOT_AOUT:
        bootAddress = load_aout(imageFile, verbose, write_memory);
        if (bootAddress < 0)
        {
            printf("Error loading AOUT file '%s'\n", imageFile);
            exit(1);
        }
        STARTADDR = bootAddress;
        break;
     case BOOT_FLOPPY:
        const char *floppy_img = imageFile ? imageFile : "FLOPPY.IMG";
        // Record mount state for UI/menus; device still boots via BPUN for now
        mount_drive(DRIVE_FLOPPY, 0, "md5-unknown", "Boot Floppy", "Boot floppy image", floppy_img);
 

         bootAddress = LoadBPUN(imageFile, verbose);
         if (bootAddress < 0)
         {
             printf("Error loading BPUN file\n");
             exit(1);
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

        const char *smd_img = imageFile ? imageFile : "SMD0.IMG";
        // Ensure unit 0 is mounted before we ask the SMD device to memory-boot
        mount_drive(DRIVE_SMD, 0, "md5-unknown", "Boot Disk", "Boot SMD image", smd_img);

         bootAddress = DeviceManager_Boot(01540);
         if (bootAddress < 0)
         {
             exit(10);
         }
         STARTADDR = bootAddress;
         break;
     }
 }
 

 /** DEVICE MOUNTING */

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
        max_units = 2;  // Floppy has units 0-1
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
            printf("Downloading image from: %s\n", image_path);
            char* image_data = download_file(image_path);
            if (image_data) {
                drives[unit].is_remote = true;
                drives[unit].data.remote_data = image_data;
                drives[unit].data_size = get_downloaded_size();  // Use actual size instead of strlen()
                printf("Downloaded %zu bytes of image data\n", drives[unit].data_size);
            } else {
                printf("Error: Failed to download image from %s\n", image_path);
                return;
            }
        } else {

            // TODO: Check if we have write access to the file "image_path"
            
            // Local file - open for read-write binary
            FILE* file = fopen(image_path, "rb+");
            if (file) {
                // Get file size
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);
                
                drives[unit].is_remote = false;
                drives[unit].is_writeprotected = false;
                drives[unit].data.local_file = file;
                drives[unit].data_size = (size_t)file_size;
                
                printf("Opened local file: %s (size: %ld bytes)\n", image_path, file_size);
            } else {
                printf("Error: Failed to open local file %s\n", image_path);
                return;
            }
        }
    }
    
    // Mount the drive
    strncpy(drives[unit].md5, md5, sizeof(drives[unit].md5) - 1);
    drives[unit].md5[sizeof(drives[unit].md5) - 1] = '\0';
    
    strncpy(drives[unit].name, name, sizeof(drives[unit].name) - 1);
    drives[unit].name[sizeof(drives[unit].name) - 1] = '\0';
    
    strncpy(drives[unit].description, description, sizeof(drives[unit].description) - 1);
    drives[unit].description[sizeof(drives[unit].description) - 1] = '\0';
    
    printf("Mounted %s to %s unit %d:\n", 
           drive_type == DRIVE_SMD ? "SMD" : "floppy",
           drive_type == DRIVE_SMD ? "SMD" : "floppy",
           unit);
    printf("  Name: %s\n", name);
    printf("  Description: %s\n", description);
    printf("  MD5: %s\n", md5);
    printf("  Image Path: %s\n", image_path ? image_path : "None");
    // TODO: Implement actual mounting logic with the machine's floppy drive system
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
        max_units = 2;  // Floppy has units 0-1
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
    drives[unit].md5[0] = '\0';
    drives[unit].name[0] = '\0';
    drives[unit].description[0] = '\0';
    drives[unit].image_path[0] = '\0';
    drives[unit].is_remote = false;
    drives[unit].data_size = 0;
    drives[unit].block_size = 0;
    
    // TODO: Implement actual unmounting logic with the machine's drive system
}

// Read a block from the specified drive
int read_block(DRIVE_TYPE drive_type, int unit, void *buffer, int block_size) {
    if (!buffer || block_size <= 0) {
        return -1;  // Invalid parameters
    }
    
    // Check if drive is mounted
    MountedDriveInfo_t* drives = NULL;
    if (drive_type == DRIVE_SMD) {
        drives = smd_drives;
    } else if (drive_type == DRIVE_FLOPPY) {
        drives = floppy_drives;
    } else {
        return -1;  // Invalid drive type
    }
    
    if (!drives || drives[unit].name[0] == '\0') {
        return -1;  // Drive not mounted
    }
    
    // Check if block size matches drive's block size
    if (block_size != drives[unit].block_size) {
        printf("Warning: Requested block size %d doesn't match drive block size %d\n", 
               block_size, drives[unit].block_size);
    }
    
    // Read data based on type (local file or remote data)
    if (drives[unit].is_remote) {
        // Read from downloaded remote data
        if (drives[unit].data.remote_data) {
            // For now, just copy the first block_size bytes (or less if data is smaller)
            size_t copy_size = (drives[unit].data_size < block_size) ? drives[unit].data_size : block_size;
            memcpy(buffer, drives[unit].data.remote_data, copy_size);
            printf("Read %zu bytes from remote data on %s unit %d\n", copy_size,
                   drive_type == DRIVE_SMD ? "SMD" : "floppy", unit);
            return (int)copy_size;
        }
    } else {
        // Read from local file
        if (drives[unit].data.local_file) {
            size_t bytes_read = fread(buffer, 1, block_size, drives[unit].data.local_file);
            printf("Read %zu bytes from local file on %s unit %d\n", bytes_read,
                   drive_type == DRIVE_SMD ? "SMD" : "floppy", unit);
            return (int)bytes_read;
        }
    }
    
    return -1;  // Error reading data
}

// Write a block to the specified drive
int write_block(DRIVE_TYPE drive_type, int unit, const void *buffer, int block_size) {
    if (!buffer || block_size <= 0) {
        return -1;  // Invalid parameters
    }
    
    // Check if drive is mounted
    MountedDriveInfo_t* drives = NULL;
    if (drive_type == DRIVE_SMD) {
        drives = smd_drives;
    } else if (drive_type == DRIVE_FLOPPY) {
        drives = floppy_drives;
    } else {
        return -1;  // Invalid drive type
    }
    
    if (!drives || drives[unit].name[0] == '\0') {
        return -1;  // Drive not mounted
    }
    
    // Check if block size matches drive's block size
    if (block_size != drives[unit].block_size) {
        printf("Warning: Requested block size %d doesn't match drive block size %d\n", 
               block_size, drives[unit].block_size);
    }
    
    // Write data based on type (local file or remote data)
    if (drives[unit].is_remote) {
        // For remote data, we would need to implement a more sophisticated approach
        // For now, just log that we can't write to remote data
        printf("Warning: Cannot write to remote data on %s unit %d\n", 
               drive_type == DRIVE_SMD ? "SMD" : "floppy", unit);
        return -1;  // Cannot write to remote data
    } else {
        // Write to local file
        if (drives[unit].data.local_file) {
            size_t bytes_written = fwrite(buffer, 1, block_size, drives[unit].data.local_file);
            fflush(drives[unit].data.local_file);  // Ensure data is written to disk
            printf("Wrote %zu bytes to local file on %s unit %d\n", bytes_written,
                   drive_type == DRIVE_SMD ? "SMD" : "floppy", unit);
            return (int)bytes_written;
        }
    }
    
    return -1;  // Error writing data
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

// Callback-based block IO for block devices
int machine_block_read(Device *device, uint8_t *buffer, size_t size, uint32_t blockAddress, int unit) {
    if (!device || !buffer || size == 0) return -1;

    DRIVE_TYPE drive_type = (device->type == DEVICE_TYPE_DISC_SMD) ? DRIVE_SMD : DRIVE_FLOPPY;
    MountedDriveInfo_t *drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
    if (!drives) return -1;

    // block size is determined by the device; size is number of blocks
    size_t bytes = size * device->blockSizeBytes;
    size_t offset = (size_t)blockAddress * device->blockSizeBytes;

    MountedDriveInfo_t *entry = &drives[unit];
    if (entry->name[0] == '\0') return -1; // not mounted

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

int machine_block_write(Device *device, const uint8_t *buffer, size_t size, uint32_t blockAddress, int unit) {
    if (!device || !buffer || size == 0) return -1;

    DRIVE_TYPE drive_type = (device->type == DEVICE_TYPE_DISC_SMD) ? DRIVE_SMD : DRIVE_FLOPPY;
    MountedDriveInfo_t *drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
    if (!drives) return -1;

    // block size is determined by the device; size is number of blocks
    size_t bytes = size * device->blockSizeBytes;
    size_t offset = (size_t)blockAddress * device->blockSizeBytes;

    MountedDriveInfo_t *entry = &drives[unit];
    if (entry->name[0] == '\0') return -1; // not mounted

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

int machine_block_disk_info(Device *device, size_t *image_size, bool *is_write_protected, int unit) {
    if (!device) return -1;

    // Set
    *image_size = 0;
    *is_write_protected = true;

    DRIVE_TYPE drive_type = (device->type == DEVICE_TYPE_DISC_SMD) ? DRIVE_SMD : DRIVE_FLOPPY;
    MountedDriveInfo_t *drives = (drive_type == DRIVE_SMD) ? smd_drives : floppy_drives;
    if (!drives) return -1;

    MountedDriveInfo_t *entry = &drives[unit];
    if (entry->name[0] == '\0') return -1;

    *image_size = entry->data_size;

    // Remote files are always NOT write protected (as they are buffered in memory)
    // Local files are write protected if we dont have access to write to the file
    if (entry->is_remote) {
        *is_write_protected = false;
    }
    else {
        // TODO: Check if we have write access to the file
        // For now, we assume that we have write access to the file
        *is_write_protected = false;
    }

    return 0;
}
