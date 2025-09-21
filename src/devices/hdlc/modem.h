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

#ifndef MODEM_H
#define MODEM_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct Device Device;

// Modem signal callback function types
typedef void (*ModemDataCallback)(Device *device, const uint8_t *data, int length);
typedef void (*ModemSignalCallback)(Device *device, bool pinValue);

// Modem state structure
typedef struct {
    // Signal states
    bool ringIndicator;     // RI
    bool dataSetReady;      // DSR
    bool signalDetector;    // SD
    bool clearToSend;       // CTS
    bool requestToSend;     // RTS
    bool dataTerminalReady; // DTR

    // Connection state
    bool isServer;
    char address[256];
    int port;
    bool connected;

    // Callbacks to HDLC device
    Device *hdlcDevice;
    ModemDataCallback onReceivedData;
    ModemSignalCallback onRingIndicator;
    ModemSignalCallback onDataSetReady;
    ModemSignalCallback onSignalDetector;
    ModemSignalCallback onClearToSend;
    ModemSignalCallback onRequestToSend;
    ModemSignalCallback onDataTerminalReady;

} ModemState;

// Function declarations
void Modem_Init(ModemState *modem, Device *hdlcDevice);
void Modem_Destroy(ModemState *modem);
void Modem_StartModem(ModemState *modem, bool isServer, const char *address, int port);
void Modem_Tick(ModemState *modem);

// Signal control functions
void Modem_SetDTR(ModemState *modem, bool value);
void Modem_SetRTS(ModemState *modem, bool value);
void Modem_SetDSR(ModemState *modem, bool value);
void Modem_SetCTS(ModemState *modem, bool value);

// Data transmission functions
void Modem_SendByte(ModemState *modem, uint8_t data);
void Modem_SendBytes(ModemState *modem, const uint8_t *data, int length);

// Callback setup functions
void Modem_SetReceivedDataCallback(ModemState *modem, ModemDataCallback callback);
void Modem_SetRingIndicatorCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetDataSetReadyCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetSignalDetectorCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetClearToSendCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetRequestToSendCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetDataTerminalReadyCallback(ModemState *modem, ModemSignalCallback callback);

#endif // MODEM_H