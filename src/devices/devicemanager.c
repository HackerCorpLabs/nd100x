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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "devices_types.h"
#include "devices_protos.h"

#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"

// For DRIVE_TYPE and machine-level block IO callbacks
#include "../machine/machine_types.h"
#include "../machine/machine_protos.h"

#define INITIAL_DEVICE_CAPACITY 16

//#define LOG_DEVICE_NOT_FOUND

// Define the level strings array

static DeviceManager deviceManager = {0}; // Initialize to zero

void DeviceManager_Init(LogLevel level)
{
    // Set the minimum log level    
    deviceManager.minLogLevel = level;
    Log_SetMinLevel(level);

    Log(LOG_INFO, "Initializing device manager (min log level: %s)\n", level_str[level]);

    deviceManager.deviceCapacity = INITIAL_DEVICE_CAPACITY;
    deviceManager.deviceCount = 0;
    deviceManager.devices = malloc(sizeof(DeviceInfo) * INITIAL_DEVICE_CAPACITY);
    if (deviceManager.devices)
    {
        // Zero initialize the device array
        memset(deviceManager.devices, 0, sizeof(DeviceInfo) * INITIAL_DEVICE_CAPACITY);
        Log(LOG_INFO, "Successfully allocated device array with capacity %d\n", deviceManager.deviceCapacity);
    }
    else
    {
        Log(LOG_ERROR, "Failed to allocate device array\n");
        // Should handle allocation failure
        exit(1);
    }
}

void DeviceManager_Destroy(void)
{
    // Clean up all devices
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {
        if (deviceManager.devices[i].device)
        {
            Device_Destroy(deviceManager.devices[i].device);
            free(deviceManager.devices[i].device); // Free the device itself
            deviceManager.devices[i].device = NULL;
        }
    }

    if (deviceManager.devices)
    {
        free(deviceManager.devices);
        deviceManager.devices = NULL;
    }

    deviceManager.deviceCount = 0;
    deviceManager.deviceCapacity = 0;
}

void DeviceManager_AddAllDevices(void)
{

    // Initialize the panel controller
    setup_pap();

    // Add the RTC at octal 1570-1577
    DeviceManager_AddDevice(DEVICE_TYPE_RTC, 0);

    // Add the Console at octal 300-307
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 0);

    // Add the PaperTape (TapeReader) at octal 400-403
    //DeviceManager_AddDevice(DEVICE_TYPE_PAPER_TAPE, 0);

    // Add the FloppyPIO at octal 1560-1567
    // DeviceManager_AddDevice(DEVICE_TYPE_FLOPPY_PIO, 0);

    // Add the FloppyDMA at octal 1560-1567
    DeviceManager_AddDevice(DEVICE_TYPE_FLOPPY_DMA, 0);

    // Add the SMD at octal 1540-1547
    DeviceManager_AddDevice(DEVICE_TYPE_DISC_SMD, 0);

    // Note: HDLC device is added conditionally via DeviceManager_AddHDLCDevice()
    // based on command line configuration
}

bool DeviceManager_AddHDLCDevice(void)
{
    // Add HDLC device at octal 1640-1657 (thumbwheel 1)
    return DeviceManager_AddDevice(DEVICE_TYPE_HDLC, 1);
}

bool DeviceManager_AddHDLCDevice_WithConfig(bool isServer, const char *address, int port)
{
    // TODO: For now, just add the device. Later this will configure networking.
    // The HDLC device configuration (server/client mode, address, port) will be
    // passed to the device during creation or after creation via a configuration function.

    bool success = DeviceManager_AddDevice(DEVICE_TYPE_HDLC, 1);

    if (success) {
        // TODO: Configure the HDLC device with network settings
        // This would involve getting the device and calling a configuration function
        // For example:
        // Device *hdlcDevice = DeviceManager_GetDeviceByAddress(01640);
        // HDLC_ConfigureNetwork(hdlcDevice, isServer, address, port);

        Log(LOG_INFO, "HDLC device network configuration: %s mode %s:%d\n",
            isServer ? "server" : "client",
            address ? address : "localhost",
            port);
    }

    return success;
}

static Device *CreateDevice(DeviceType type, uint8_t thumbwheel)
{
    Device *dev = NULL;

    // Set up device-specific initialization based on type
    switch (type)
    {
    case DEVICE_TYPE_RTC:
        dev = CreateRTCDevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create RTC device\n");
            return NULL;
        }
        break;
    case DEVICE_TYPE_TERMINAL:
        dev = CreateTerminalDevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create terminal device\n");
            return NULL;
        }
        break;
    case DEVICE_TYPE_PAPER_TAPE:
        dev = CreatePaperTapeDevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create paper tape device\n");
            return NULL;
        }
        break;
    case DEVICE_TYPE_FLOPPY_PIO:
        dev = CreateFloppyPIODevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create floppy PIO device\n");
            return NULL;
        }
        break;

    case DEVICE_TYPE_DISC_SMD:
        dev = CreateSMDDevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create SMD device\n");
            return NULL;
        }
        break;
    case DEVICE_TYPE_FLOPPY_DMA:
        dev = CreateFloppyDMADevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create floppy DMA device\n");
            return NULL;
        }
        break;
    case DEVICE_TYPE_HDLC:
        dev = CreateHDLCDevice(thumbwheel);
        if (!dev)
        {
            Log(LOG_ERROR, "Failed to create HDLC device\n");
            return NULL;
        }
        break;
    default:
        Log(LOG_ERROR, "Unknown device type: %d\n", type);
        return NULL;
    }

    // Reset the device
    if (dev)
    {
        // Record the concrete type for downstream logic (read-only property)
        dev->type = type;
        Device_Reset(dev);
    }

    return dev;
}

void DeviceManager_MasterClear(void)
{
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {
        if (deviceManager.devices[i].device)
        {
            Device_Reset(deviceManager.devices[i].device);
        }
    }
}

bool DeviceManager_AddDevice(DeviceType type, uint8_t thumbwheel)
{
    // Check if we have capacity
    if (deviceManager.deviceCount >= deviceManager.deviceCapacity)
    {
        Log(LOG_ERROR, "Failed to add device: device array is full (capacity: %d, count: %d)\n",
            deviceManager.deviceCapacity, deviceManager.deviceCount);
        return false;
    }

    // Create and add new device
    Device *dev = CreateDevice(type, thumbwheel);
    if (dev)
    {
        deviceManager.devices[deviceManager.deviceCount].device = dev;
        // If this is a block device, hook up machine-level block IO callbacks
        if (dev->deviceClass == DEVICE_CLASS_BLOCK) {            
            Device_SetBlockRead(dev, (BlockDeviceReadFunc)machine_block_read, NULL);
            Device_SetBlockWrite(dev, (BlockDeviceWriteFunc)machine_block_write, NULL);
            Device_SetBlockDiskInfo(dev, (BlockDeviceDiskInfoFunc)machine_block_disk_info, NULL);
        }
        deviceManager.deviceCount++;
        return true;
    }
    else
    {
        Log(LOG_ERROR, "Failed to create device\n");
    }

    return false;
}

uint16_t DeviceManager_Read(uint32_t address)
{
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {

        Device *dev = deviceManager.devices[i].device;

        if (dev && Device_IsInAddress(dev, address))
        {
            // Log(LOG_DEBUG, "Device found for READ address: %o\n", address);
            return Device_Read(dev, address);
        }
    }

    interrupt(14, 1 << 7); /* IOX error lvl14 */
#ifdef LOG_DEVICE_NOT_FOUND    
    Log(LOG_WARNING, "No device found for READ address: %o\n", address);
#endif
    return 0;
}

void DeviceManager_Write(uint32_t address, uint16_t value)
{
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {
        Device *dev = deviceManager.devices[i].device;
        if (!dev)
        {
            Log(LOG_ERROR, "Device at index %d is NULL\n", i);
            continue;
        }

        if (Device_IsInAddress(dev, address))
        {
            //Log(LOG_DEBUG, "Device found for WRITE address: %o\n", address);
            Device_Write(dev, address, value);
            return;
        }
    }

    interrupt(14, 1 << 7); /* IOX error lvl14 */
#ifdef LOG_DEVICE_NOT_FOUND    
    Log(LOG_WARNING, "No device found for WRITE address: %o\n", address);
#endif
}

int DeviceManager_Ident(uint16_t level)
{
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {
        Device *dev = deviceManager.devices[i].device;
        if (dev && (dev->interruptBits & (1 << level)))
        {
            uint16_t id = Device_Ident(dev, level);
            if (id > 0)
            {
                return id;
            }
        }
    }

#ifdef LOG_DEVICE_NOT_FOUND
    // interrupt(14,1<<7); /* IOX error lvl14 */
     Log(LOG_WARNING, "No device found for IDENT level: %d\n", level);
#endif    

    return 0;
}

uint16_t DeviceManager_Tick(void)
{
    uint16_t interruptBits = 0;
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {
        Device *dev = deviceManager.devices[i].device;
        if (dev)
        {
            interruptBits |= Device_Tick(dev);
        }
    }

    return interruptBits;
}
Device *DeviceManager_GetDeviceByAddress(uint32_t address)
{
    for (int i = 0; i < deviceManager.deviceCount; i++)
    {
        Device *dev = deviceManager.devices[i].device;
        if (dev && Device_IsInAddress(dev, address))
        {
            return dev;
        }
    }

    return NULL;
}

// Loads boot code from disk to memory. Returns the boot address, or -1 if error
int DeviceManager_Boot(uint16_t device_id)
{

    Device *dev = DeviceManager_GetDeviceByAddress(device_id & ~(1<<15 | 1<<13)); // mask off bit 15 and 13 when searching for device
    if (!dev) return -1;


    // Boot the device
    // Autodetect if the boot is a BPUN, MEMORY BOOT or BOOTSTRAP
    //
    // If BPUN, then we need to load the BPUN from the device 
    // If MEMORY BOOT, then we need to load the memory image from the device 
    // If BOOTSTRAP, then we need to load the bootstrap code from the device 

    // Load the BPUN image IF bit 15 in device_id is 1 - Typical paper-tape or floppy disk (400 or 1560)
    // Load using "Bootstrap"" IF bit 13 in device is 1 - Used for device 500 (Winchester disk) and 1540 (SMD disk)
    // Load the memory image IF bit 15 in device_id is 0 - Winchester disk or SMD disk (1540) (first 2KB of disk is loaded to memory at 000000-001777)

    
    // At the moment.. 
    // Only implemented for SMD, and only MEMORY boot
    
    return Device_Boot(dev,device_id);
    
    
#ifdef LOG_DEVICE_NOT_FOUND
    // interrupt(14,1<<7); /* IOX error lvl14 */s
     Log(LOG_WARNING, "No device found for BOOT id: %d\n", device_id);
#endif    

    return -1;
}
