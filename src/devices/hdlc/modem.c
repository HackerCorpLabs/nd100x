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

#include "../devices_types.h"
#include "modem.h"

// Debug flag
//#define DEBUG_MODEM

void Modem_Init(ModemState *modem, Device *hdlcDevice)
{
    if (!modem) return;

    memset(modem, 0, sizeof(ModemState));

    // Initialize default signal states
    modem->ringIndicator = false;
    modem->dataSetReady = false;
    modem->signalDetector = false;
    modem->clearToSend = false;
    modem->requestToSend = false;
    modem->dataTerminalReady = false;

    // Connection state
    modem->isServer = false;
    modem->connected = false;
    modem->port = 0;

    // Set HDLC device reference
    modem->hdlcDevice = hdlcDevice;

#ifdef DEBUG_MODEM
    printf("Modem: Initialized\n");
#endif
}

void Modem_Destroy(ModemState *modem)
{
    if (!modem) return;

    // TODO: Close any open connections
    modem->connected = false;

#ifdef DEBUG_MODEM
    printf("Modem: Destroyed\n");
#endif
}

void Modem_StartModem(ModemState *modem, bool isServer, const char *address, int port)
{
    if (!modem) return;

    modem->isServer = isServer;
    modem->port = port;

    if (address) {
        strncpy(modem->address, address, sizeof(modem->address) - 1);
        modem->address[sizeof(modem->address) - 1] = '\0';
    }

#ifdef DEBUG_MODEM
    printf("Modem: Started as %s on %s:%d\n",
           isServer ? "server" : "client",
           address ? address : "localhost",
           port);
#endif

    // TODO: Implement actual network connection
    // For now, simulate connection success
    modem->connected = true;
    modem->dataSetReady = true;
    modem->signalDetector = true;

    // Trigger DSR callback
    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, modem->dataSetReady);
    }

    // Trigger SD callback
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, modem->signalDetector);
    }
}

void Modem_Tick(ModemState *modem)
{
    if (!modem) return;

    // TODO: Process network I/O
    // TODO: Handle connection state changes
    // TODO: Process received data
}

void Modem_SetDTR(ModemState *modem, bool value)
{
    if (!modem) return;

    if (modem->dataTerminalReady != value) {
        modem->dataTerminalReady = value;

#ifdef DEBUG_MODEM
        printf("Modem: DTR set to %s\n", value ? "true" : "false");
#endif

        // In point-to-point setup, DTR connects to DSR
        Modem_SetDSR(modem, value);

        if (modem->onDataTerminalReady) {
            modem->onDataTerminalReady(modem->hdlcDevice, value);
        }
    }
}

void Modem_SetRTS(ModemState *modem, bool value)
{
    if (!modem) return;

    if (modem->requestToSend != value) {
        modem->requestToSend = value;

#ifdef DEBUG_MODEM
        printf("Modem: RTS set to %s\n", value ? "true" : "false");
#endif

        // In point-to-point setup, RTS connects to CTS
        Modem_SetCTS(modem, value);

        if (modem->onRequestToSend) {
            modem->onRequestToSend(modem->hdlcDevice, value);
        }
    }
}

void Modem_SetDSR(ModemState *modem, bool value)
{
    if (!modem) return;

    if (modem->dataSetReady != value) {
        modem->dataSetReady = value;

#ifdef DEBUG_MODEM
        printf("Modem: DSR set to %s\n", value ? "true" : "false");
#endif

        if (modem->onDataSetReady) {
            modem->onDataSetReady(modem->hdlcDevice, value);
        }
    }
}

void Modem_SetCTS(ModemState *modem, bool value)
{
    if (!modem) return;

    if (modem->clearToSend != value) {
        modem->clearToSend = value;

#ifdef DEBUG_MODEM
        printf("Modem: CTS set to %s\n", value ? "true" : "false");
#endif

        if (modem->onClearToSend) {
            modem->onClearToSend(modem->hdlcDevice, value);
        }
    }
}

void Modem_SendByte(ModemState *modem, uint8_t data)
{
    if (!modem) return;

#ifdef DEBUG_MODEM
    printf("Modem: Sending byte 0x%02X\n", data);
#endif

    // TODO: Send data over network connection
    // For now, just simulate transmission
}

void Modem_SendBytes(ModemState *modem, const uint8_t *data, int length)
{
    if (!modem || !data) return;

#ifdef DEBUG_MODEM
    printf("Modem: Sending %d bytes\n", length);
#endif

    // TODO: Send data over network connection
    // For now, just simulate transmission
}

// Callback setup functions
void Modem_SetReceivedDataCallback(ModemState *modem, ModemDataCallback callback)
{
    if (modem) modem->onReceivedData = callback;
}

void Modem_SetRingIndicatorCallback(ModemState *modem, ModemSignalCallback callback)
{
    if (modem) modem->onRingIndicator = callback;
}

void Modem_SetDataSetReadyCallback(ModemState *modem, ModemSignalCallback callback)
{
    if (modem) modem->onDataSetReady = callback;
}

void Modem_SetSignalDetectorCallback(ModemState *modem, ModemSignalCallback callback)
{
    if (modem) modem->onSignalDetector = callback;
}

void Modem_SetClearToSendCallback(ModemState *modem, ModemSignalCallback callback)
{
    if (modem) modem->onClearToSend = callback;
}

void Modem_SetRequestToSendCallback(ModemState *modem, ModemSignalCallback callback)
{
    if (modem) modem->onRequestToSend = callback;
}

void Modem_SetDataTerminalReadyCallback(ModemState *modem, ModemSignalCallback callback)
{
    if (modem) modem->onDataTerminalReady = callback;
}