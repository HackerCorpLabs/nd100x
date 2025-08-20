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


#ifndef DEVICES_TYPES_H
#define DEVICES_TYPES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#include "../ndlib/ndlib_types.h" // for LogLevel def

// External function declaration

void interrupt(uint16_t lvl, uint16_t sub); // cpu.c

// Physical memory functions in cpu_mms.c
extern int ReadPhysicalMemory(int physicalAddress, bool privileged);
extern void WritePhysicalMemory(int physicalAddress, uint16_t value, bool privileged);

// ** Device **

#define MAX_DEVICES 16
#define MAX_DEVICE_NAME 64

// IO Delay definitions
#define IODELAY_TERMINAL 100
#define IODELAY_FLOPPY 300
#define IODELAY_HDD 10
#define IODELAY_HDD_SMD 10
#define IODELAY_SLOW 10
#define IODELAY_SCSI_SHORT 10
#define IODELAY_SCSI_TIMEOUT 0xFFFF

// Parity table size
#define PARITY_TABLE_SIZE 256
extern const uint8_t Device_OddParityTable[PARITY_TABLE_SIZE];


// Device initialization/classification types
typedef enum {
    DEVICE_CLASS_STANDARD = 0,  // Standard device (no character or block I/O)
    DEVICE_CLASS_CHARACTER,     // Character device (terminal, serial, etc.)
    DEVICE_CLASS_BLOCK,         // Block device (disk, tape, etc.)
    DEVICE_CLASS_RTC            // Real-time clock device
} DeviceClass;

// Forward declaration of Device structure
struct Device;

// Generic Character Device callback function types
typedef void (*CharacterDeviceOutputFunc)(struct Device *device, char c);
typedef void (*CharacterDeviceInputFunc)(struct Device *device, char c);

// Character Device callback structure
typedef struct {
    CharacterDeviceOutputFunc outputFunc;  // Called when device outputs a character
    CharacterDeviceInputFunc inputFunc;    // Called when device receives input
} CharacterDeviceCallbacks;

// Generic Block Device callback function types
#define MAX_BLOCK_SIZE 2048


typedef int (*BlockDeviceReadFunc)(struct Device *device, uint8_t *buffer, size_t size, uint32_t blockAddress, int unit);
typedef int (*BlockDeviceWriteFunc)(struct Device *device, const uint8_t *buffer, size_t size, uint32_t blockAddress, int unit);
typedef int (*BlockDeviceDiskInfoFunc)(struct Device *device, size_t *image_size, bool *is_write_protected, int unit);

// Block Device callback structure
typedef struct {
    BlockDeviceReadFunc readFunc;        // Called when device reads a block
    BlockDeviceWriteFunc writeFunc;      // Called when device writes a block
    BlockDeviceDiskInfoFunc diskInfoFunc; // Called to read disk info (size, write-protect)
    void *userData;                      // User-defined data passed to callbacks (optional)
} BlockDeviceCallbacks;

// IO Delay callback function type
typedef bool (*IODelayedCallback)(void *context, int param);

// IO Delay information structure
typedef struct {
    int delayTicks;
    IODelayedCallback callback;
    void *context;
    int parameter;
    uint8_t level;
} DelayedIoInfo;

// Device types
typedef enum {
    DEVICE_TYPE_NONE = 0,
    DEVICE_TYPE_RTC,
    DEVICE_TYPE_TERMINAL,
    DEVICE_TYPE_PAPER_TAPE,
    DEVICE_TYPE_FLOPPY_PIO,
    DEVICE_TYPE_FLOPPY_DMA,    
    DEVICE_TYPE_DISC_SMD,
    DEVICE_TYPE_MAX
} DeviceType;

// Device structure
typedef struct Device {
    // Device memory range
    uint32_t startAddress;
    uint32_t endAddress;
    
    // Interrupt handling
    uint16_t interruptBits;
    uint16_t interruptLevel;  // Default interrupt level
    uint16_t identCode;      // Identcode for this device
    uint16_t logicalDevice;  // Logical device ID for this device
    DeviceType type;         // Read-only: concrete device type (set at creation)
    
    // Device name
    char memoryName[MAX_DEVICE_NAME];
    
    // IO Delay handling
    DelayedIoInfo *ioDelays;
    int ioDelayCount;
    int ioDelayCapacity;
    
    // Device functions
    void (*Reset)(struct Device *self);
    uint16_t (*Tick)(struct Device *self);
    int (*Boot)(struct Device *self, uint16_t device_id);
    uint16_t (*Read)(struct Device *self, uint32_t address);
    void (*Write)(struct Device *self, uint32_t address, uint16_t value);
    uint16_t (*Ident)(struct Device *self, uint16_t level);

    void (*Destroy)(struct Device *self);
    
    // Device classification
    DeviceClass deviceClass;  // Type of device (standard, character, block, RTC)
    
    // Block device properties (valid when deviceClass == DEVICE_CLASS_BLOCK)
    size_t blockSizeBytes;    // Sector/block size in bytes; set by the concrete device
    
    // Device callbacks
    CharacterDeviceCallbacks charCallbacks;  // Character device callbacks (if deviceClass == DEVICE_CLASS_CHARACTER)
    BlockDeviceCallbacks blockCallbacks;     // Block device callbacks (if deviceClass == DEVICE_CLASS_BLOCK)
    
    // Device-specific data
    void *deviceData;
    
} Device;


typedef struct {
    Device *devices[MAX_DEVICES];
    int count;
} DeviceList;


// ** Device Manager **

// Device info structure
typedef struct {
    Device *device;
} DeviceInfo;

// Device manager structure
typedef struct {
    DeviceInfo *devices;
    int deviceCount;
    int deviceCapacity;
    LogLevel minLogLevel;  // Minimum log level for filtering messages
} DeviceManager;



#include "./floppy/deviceFloppyPIO.h"
#include "./floppy/deviceFloppyDMA.h"   
#include "./papertape/devicePapertape.h"
#include "./rtc/deviceRTC.h"
#include "./smd/deviceSMD.h"
#include "./terminal/deviceTerminal.h"
#include "./panel/panel.h"
#endif // DEVICES_TYPES_H

