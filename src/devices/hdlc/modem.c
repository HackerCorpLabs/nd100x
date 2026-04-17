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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef __EMSCRIPTEN__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#endif

#include "../devices_types.h"
#include "modem.h"

// Debug flag
//#define DEBUG_MODEM

#ifndef __EMSCRIPTEN__

// Set socket to non-blocking mode
static bool set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

// Set TCP_NODELAY to disable Nagle's algorithm
static void set_nodelay(int fd)
{
    int flag = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

// Close a socket fd and reset to -1
static void close_fd(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

// Handle a new TCP connection (either accepted or connected)
static void modem_on_connected(ModemState *modem)
{
    modem->connected = true;
    modem->dataSetReady = true;
    modem->signalDetector = true;

#ifdef DEBUG_MODEM
    printf("Modem: TCP connected (fd=%d)\n", modem->clientFd);
#endif

    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, true);
    }
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, true);
    }
}

// Handle TCP disconnection
static void modem_on_disconnected(ModemState *modem)
{
    close_fd(&modem->clientFd);
    modem->connected = false;
    modem->dataSetReady = false;
    modem->signalDetector = false;

#ifdef DEBUG_MODEM
    printf("Modem: TCP disconnected\n");
#endif

    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, false);
    }
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, false);
    }
}

#endif /* !__EMSCRIPTEN__ */


void Modem_Init(ModemState *modem, Device *hdlcDevice)
{
    if (!modem) return;

    memset(modem, 0, sizeof(ModemState));

    modem->ringIndicator = false;
    modem->dataSetReady = false;
    modem->signalDetector = false;
    modem->clearToSend = false;
    modem->requestToSend = false;
    modem->dataTerminalReady = false;

    modem->isServer = false;
    modem->connected = false;
    modem->port = 0;
    modem->networkStarted = false;
    modem->listenFd = -1;
    modem->clientFd = -1;
    modem->pollTickCounter = 0;

    modem->hdlcDevice = hdlcDevice;

#ifdef DEBUG_MODEM
    printf("Modem: Initialized\n");
#endif
}

void Modem_Destroy(ModemState *modem)
{
    if (!modem) return;

#ifndef __EMSCRIPTEN__
    close_fd(&modem->clientFd);
    close_fd(&modem->listenFd);
#endif

    modem->connected = false;
    modem->networkStarted = false;

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
    printf("Modem: Starting as %s on %s:%d\n",
           isServer ? "server" : "client",
           address ? address : "localhost",
           port);
#endif

#ifndef __EMSCRIPTEN__
    if (isServer) {
        // Create listening socket
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            printf("Modem: Failed to create listen socket: %s\n", strerror(errno));
            return;
        }

        int reuse = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((uint16_t)port);

        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("Modem: Failed to bind port %d: %s\n", port, strerror(errno));
            close(fd);
            return;
        }

        if (listen(fd, 1) < 0) {
            printf("Modem: Failed to listen: %s\n", strerror(errno));
            close(fd);
            return;
        }

        if (!set_nonblocking(fd)) {
            printf("Modem: Failed to set non-blocking on listen socket\n");
            close(fd);
            return;
        }

        modem->listenFd = fd;
        modem->networkStarted = true;
        printf("Modem: Listening on port %d\n", port);

    } else {
        // Client mode: initiate non-blocking connect
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            printf("Modem: Failed to create client socket: %s\n", strerror(errno));
            return;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)port);

        // Resolve hostname
        struct hostent *he = gethostbyname(address ? address : "localhost");
        if (!he) {
            printf("Modem: Failed to resolve '%s'\n", address ? address : "localhost");
            close(fd);
            return;
        }
        memcpy(&addr.sin_addr, he->h_addr_list[0], (size_t)he->h_length);

        if (!set_nonblocking(fd)) {
            printf("Modem: Failed to set non-blocking on client socket\n");
            close(fd);
            return;
        }

        int ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
        if (ret < 0 && errno != EINPROGRESS) {
            printf("Modem: Failed to connect to %s:%d: %s\n",
                   address ? address : "localhost", port, strerror(errno));
            close(fd);
            return;
        }

        modem->clientFd = fd;
        modem->networkStarted = true;

        if (ret == 0) {
            // Connected immediately
            set_nodelay(fd);
            modem_on_connected(modem);
        } else {
            printf("Modem: Connecting to %s:%d...\n",
                   address ? address : "localhost", port);
        }
    }
#else
    // WASM: no TCP sockets, simulate connected for loopback testing
    modem->networkStarted = true;
    modem->connected = true;
    modem->dataSetReady = true;
    modem->signalDetector = true;
    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, true);
    }
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, true);
    }
#endif
}

// Poll interval: check socket every 1000 CPU ticks (~25us at 40MHz)
// Avoids millions of syscalls/sec while keeping latency low
#define MODEM_POLL_INTERVAL 1000

void Modem_Tick(ModemState *modem)
{
    if (!modem || !modem->networkStarted) return;

    // Rate-limit socket polling
    if (++modem->pollTickCounter < MODEM_POLL_INTERVAL) return;
    modem->pollTickCounter = 0;

#ifndef __EMSCRIPTEN__
    // Server mode: try to accept a new connection if none active
    if (modem->isServer && modem->listenFd >= 0 && modem->clientFd < 0) {
        struct sockaddr_in peer;
        socklen_t peerlen = sizeof(peer);
        int fd = accept(modem->listenFd, (struct sockaddr *)&peer, &peerlen);
        if (fd >= 0) {
            set_nonblocking(fd);
            set_nodelay(fd);
            modem->clientFd = fd;
            printf("Modem: Accepted connection from %s:%d\n",
                   inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
            modem_on_connected(modem);
        }
        // EAGAIN/EWOULDBLOCK is normal for non-blocking accept
    }

    // Client mode: check if async connect completed
    if (!modem->isServer && modem->clientFd >= 0 && !modem->connected) {
        struct pollfd pfd = { .fd = modem->clientFd, .events = POLLOUT };
        int ret = poll(&pfd, 1, 0);
        if (ret > 0 && (pfd.revents & POLLOUT)) {
            int err = 0;
            socklen_t errlen = sizeof(err);
            getsockopt(modem->clientFd, SOL_SOCKET, SO_ERROR, &err, &errlen);
            if (err == 0) {
                set_nodelay(modem->clientFd);
                modem_on_connected(modem);
            } else {
                printf("Modem: Connect failed: %s\n", strerror(err));
                close_fd(&modem->clientFd);
            }
        }
    }

    // Read incoming data from TCP
    if (modem->clientFd >= 0 && modem->connected) {
        uint8_t buf[MODEM_RECV_BUF_SIZE];
        ssize_t n = recv(modem->clientFd, buf, sizeof(buf), 0);
        if (n > 0) {
#ifdef DEBUG_MODEM
            printf("Modem: Received %zd bytes from TCP\n", n);
#endif
            if (modem->onReceivedData) {
                modem->onReceivedData(modem->hdlcDevice, buf, (int)n);
            }
        } else if (n == 0) {
            // Peer closed connection
            printf("Modem: Remote disconnected\n");
            modem_on_disconnected(modem);
        } else {
            // n < 0
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                printf("Modem: recv error: %s\n", strerror(errno));
                modem_on_disconnected(modem);
            }
        }
    }
#endif /* !__EMSCRIPTEN__ */
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

#ifndef __EMSCRIPTEN__
    if (modem->clientFd >= 0 && modem->connected) {
        ssize_t n = send(modem->clientFd, &data, 1, MSG_NOSIGNAL);
        if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            printf("Modem: send error: %s\n", strerror(errno));
            modem_on_disconnected(modem);
        }
    }
#endif
}

void Modem_SendBytes(ModemState *modem, const uint8_t *data, int length)
{
    if (!modem || !data || length <= 0) return;

#ifdef DEBUG_MODEM
    printf("Modem: Sending %d bytes\n", length);
#endif

#ifndef __EMSCRIPTEN__
    if (modem->clientFd >= 0 && modem->connected) {
        int sent = 0;
        while (sent < length) {
            ssize_t n = send(modem->clientFd, data + sent, (size_t)(length - sent), MSG_NOSIGNAL);
            if (n > 0) {
                sent += (int)n;
            } else if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Socket buffer full, try again next tick
                    break;
                }
                printf("Modem: send error: %s\n", strerror(errno));
                modem_on_disconnected(modem);
                return;
            }
        }
    }
#endif
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
