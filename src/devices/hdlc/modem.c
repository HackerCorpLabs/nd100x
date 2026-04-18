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

//#define DEBUG_MODEM

// Poll interval: check socket every 1000 CPU ticks (~25us at 40MHz)
#define MODEM_POLL_INTERVAL 1000

// Connect timeout in poll intervals (~5 seconds at 1000-tick intervals, 40MHz)
#define MODEM_CONNECT_TIMEOUT 5000

// Reconnect delays in poll intervals (with exponential backoff)
#define MODEM_RECONNECT_BASE   1000   // ~1 second
#define MODEM_RECONNECT_MAX   30000   // ~30 seconds max

#ifndef __EMSCRIPTEN__

static bool set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

static void set_nodelay(int fd)
{
    int flag = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

static void close_fd(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

static void modem_on_connected(ModemState *modem)
{
    modem->connected = true;
    modem->clientState = CLIENT_CONNECTED;
    modem->reconnectAttempts = 0;

    printf("Modem: Connected to %s:%d\n", modem->address[0] ? modem->address : "peer", modem->port);

    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, true);
    }
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, true);
    }
}

static void modem_on_disconnected(ModemState *modem)
{
    close_fd(&modem->clientFd);
    modem->connected = false;

    printf("Modem: Disconnected from %s:%d\n", modem->address[0] ? modem->address : "peer", modem->port);

    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, false);
    }
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, false);
    }

    // Client mode: schedule reconnect with backoff
    if (!modem->isServer && modem->addrResolved) {
        modem->reconnectAttempts++;
        int delay = MODEM_RECONNECT_BASE;
        for (int i = 0; i < modem->reconnectAttempts && i < 5; i++) {
            delay *= 2;
        }
        if (delay > MODEM_RECONNECT_MAX) delay = MODEM_RECONNECT_MAX;
        modem->connectTickTimer = delay;
        modem->clientState = CLIENT_WAIT_RECONNECT;
        printf("Modem: Will reconnect in ~%d seconds\n", delay / 1000);
    }

    // Server mode: just wait for next accept (already handled in Tick)
}

// Initiate a non-blocking connect using cached resolved address
static void modem_start_connect(ModemState *modem)
{
    if (!modem->addrResolved) return;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("Modem: Failed to create socket: %s\n", strerror(errno));
        modem->clientState = CLIENT_WAIT_RECONNECT;
        modem->connectTickTimer = MODEM_RECONNECT_BASE;
        return;
    }

    if (!set_nonblocking(fd)) {
        close(fd);
        modem->clientState = CLIENT_WAIT_RECONNECT;
        modem->connectTickTimer = MODEM_RECONNECT_BASE;
        return;
    }

    int ret = connect(fd, (struct sockaddr *)&modem->resolvedAddr, sizeof(modem->resolvedAddr));
    if (ret == 0) {
        // Connected immediately (unlikely but possible for localhost)
        set_nodelay(fd);
        modem->clientFd = fd;
        modem_on_connected(modem);
    } else if (errno == EINPROGRESS) {
        modem->clientFd = fd;
        modem->clientState = CLIENT_CONNECTING;
        modem->connectTickTimer = MODEM_CONNECT_TIMEOUT;
#ifdef DEBUG_MODEM
        printf("Modem: Connect in progress to %s:%d\n", modem->address, modem->port);
#endif
    } else {
        printf("Modem: Connect failed: %s\n", strerror(errno));
        close(fd);
        modem->clientState = CLIENT_WAIT_RECONNECT;
        modem->reconnectAttempts++;
        modem->connectTickTimer = MODEM_RECONNECT_BASE;
    }
}

#endif /* !__EMSCRIPTEN__ */


void Modem_Init(ModemState *modem, Device *hdlcDevice)
{
    if (!modem) return;
    memset(modem, 0, sizeof(ModemState));

    modem->listenFd = -1;
    modem->clientFd = -1;
    modem->clientState = CLIENT_IDLE;
    modem->hdlcDevice = hdlcDevice;
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

#ifndef __EMSCRIPTEN__
    if (isServer) {
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

        set_nonblocking(fd);
        modem->listenFd = fd;
        modem->networkStarted = true;
        printf("Modem: Listening on port %d\n", port);

    } else {
        // Client mode: resolve hostname NOW (blocking, but only once at startup)
        const char *host = address ? address : "localhost";

        memset(&modem->resolvedAddr, 0, sizeof(modem->resolvedAddr));
        modem->resolvedAddr.sin_family = AF_INET;
        modem->resolvedAddr.sin_port = htons((uint16_t)port);

        // Try numeric IP first (instant, no DNS)
        if (inet_pton(AF_INET, host, &modem->resolvedAddr.sin_addr) == 1) {
            modem->addrResolved = true;
        } else {
            // DNS lookup (may block briefly)
            struct hostent *he = gethostbyname(host);
            if (he) {
                memcpy(&modem->resolvedAddr.sin_addr, he->h_addr_list[0], (size_t)he->h_length);
                modem->addrResolved = true;
            } else {
                printf("Modem: Failed to resolve '%s' - will not connect\n", host);
                modem->addrResolved = false;
                modem->clientState = CLIENT_IDLE;
            }
        }

        modem->networkStarted = true;

        if (modem->addrResolved) {
            printf("Modem: Resolved %s -> %s\n", host, inet_ntoa(modem->resolvedAddr.sin_addr));
            modem_start_connect(modem);
        }
    }

    // Signal DSR and SD immediately so SINTRAN sees the hardware as present.
    // TCP connection state is separate from hardware presence.
    modem->dataSetReady = true;
    modem->signalDetector = true;
    if (modem->onDataSetReady) {
        modem->onDataSetReady(modem->hdlcDevice, true);
    }
    if (modem->onSignalDetector) {
        modem->onSignalDetector(modem->hdlcDevice, true);
    }

#else
    // WASM: simulate connected
    modem->networkStarted = true;
    modem->connected = true;
    modem->dataSetReady = true;
    modem->signalDetector = true;
    if (modem->onDataSetReady) modem->onDataSetReady(modem->hdlcDevice, true);
    if (modem->onSignalDetector) modem->onSignalDetector(modem->hdlcDevice, true);
#endif
}

void Modem_Tick(ModemState *modem)
{
    if (!modem || !modem->networkStarted) return;

    if (++modem->pollTickCounter < MODEM_POLL_INTERVAL) return;
    modem->pollTickCounter = 0;

#ifndef __EMSCRIPTEN__

    // === SERVER MODE ===
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
    }

    // Server mode: auto-accept new connection after disconnect
    if (modem->isServer && modem->clientFd < 0 && modem->connected) {
        modem->connected = false;
    }

    // === CLIENT MODE STATE MACHINE ===
    if (!modem->isServer) {
        switch (modem->clientState) {
            case CLIENT_IDLE:
                break;

            case CLIENT_CONNECTING:
                // Check if async connect completed
                if (modem->clientFd >= 0) {
                    struct pollfd pfd = { .fd = modem->clientFd, .events = POLLOUT };
                    int ret = poll(&pfd, 1, 0);
                    if (ret > 0) {
                        if (pfd.revents & (POLLERR | POLLHUP)) {
                            // Connect failed
                            printf("Modem: Connect to %s:%d failed\n", modem->address, modem->port);
                            close_fd(&modem->clientFd);
                            modem->reconnectAttempts++;
                            int delay = MODEM_RECONNECT_BASE;
                            for (int i = 0; i < modem->reconnectAttempts && i < 5; i++) delay *= 2;
                            if (delay > MODEM_RECONNECT_MAX) delay = MODEM_RECONNECT_MAX;
                            modem->connectTickTimer = delay;
                            modem->clientState = CLIENT_WAIT_RECONNECT;
                            printf("Modem: Retry in ~%d seconds\n", delay / 1000);
                        } else if (pfd.revents & POLLOUT) {
                            int err = 0;
                            socklen_t errlen = sizeof(err);
                            getsockopt(modem->clientFd, SOL_SOCKET, SO_ERROR, &err, &errlen);
                            if (err == 0) {
                                set_nodelay(modem->clientFd);
                                modem_on_connected(modem);
                            } else {
                                printf("Modem: Connect to %s:%d failed: %s\n",
                                       modem->address, modem->port, strerror(err));
                                close_fd(&modem->clientFd);
                                modem->reconnectAttempts++;
                                modem->connectTickTimer = MODEM_RECONNECT_BASE;
                                modem->clientState = CLIENT_WAIT_RECONNECT;
                            }
                        }
                    }

                    // Connect timeout
                    if (modem->clientState == CLIENT_CONNECTING) {
                        modem->connectTickTimer--;
                        if (modem->connectTickTimer <= 0) {
                            printf("Modem: Connect to %s:%d timed out\n", modem->address, modem->port);
                            close_fd(&modem->clientFd);
                            modem->reconnectAttempts++;
                            modem->connectTickTimer = MODEM_RECONNECT_BASE;
                            modem->clientState = CLIENT_WAIT_RECONNECT;
                        }
                    }
                }
                break;

            case CLIENT_CONNECTED:
                // Normal operation — handled below in recv section
                break;

            case CLIENT_WAIT_RECONNECT:
                modem->connectTickTimer--;
                if (modem->connectTickTimer <= 0) {
                    printf("Modem: Reconnecting to %s:%d (attempt %d)...\n",
                           modem->address, modem->port, modem->reconnectAttempts + 1);
                    modem_start_connect(modem);
                }
                break;
        }
    }

    // === READ INCOMING TCP DATA ===
    if (modem->clientFd >= 0 && modem->connected) {
        uint8_t buf[MODEM_RECV_BUF_SIZE];
        ssize_t n = recv(modem->clientFd, buf, sizeof(buf), 0);
        if (n > 0) {
            if (modem->onReceivedData) {
                modem->onReceivedData(modem->hdlcDevice, buf, (int)n);
            }
        } else if (n == 0) {
            printf("Modem: Remote closed connection\n");
            modem_on_disconnected(modem);
        } else {
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
        if (modem->onClearToSend) {
            modem->onClearToSend(modem->hdlcDevice, value);
        }
    }
}

void Modem_SendByte(ModemState *modem, uint8_t data)
{
    if (!modem) return;

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

#ifndef __EMSCRIPTEN__
    if (modem->clientFd >= 0 && modem->connected) {
        int sent = 0;
        while (sent < length) {
            ssize_t n = send(modem->clientFd, data + sent, (size_t)(length - sent), MSG_NOSIGNAL);
            if (n > 0) {
                sent += (int)n;
            } else if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                printf("Modem: send error: %s\n", strerror(errno));
                modem_on_disconnected(modem);
                return;
            }
        }
    }
#endif
}

void Modem_SetReceivedDataCallback(ModemState *modem, ModemDataCallback callback)     { if (modem) modem->onReceivedData = callback; }
void Modem_SetRingIndicatorCallback(ModemState *modem, ModemSignalCallback callback)  { if (modem) modem->onRingIndicator = callback; }
void Modem_SetDataSetReadyCallback(ModemState *modem, ModemSignalCallback callback)   { if (modem) modem->onDataSetReady = callback; }
void Modem_SetSignalDetectorCallback(ModemState *modem, ModemSignalCallback callback) { if (modem) modem->onSignalDetector = callback; }
void Modem_SetClearToSendCallback(ModemState *modem, ModemSignalCallback callback)    { if (modem) modem->onClearToSend = callback; }
void Modem_SetRequestToSendCallback(ModemState *modem, ModemSignalCallback callback)  { if (modem) modem->onRequestToSend = callback; }
void Modem_SetDataTerminalReadyCallback(ModemState *modem, ModemSignalCallback callback) { if (modem) modem->onDataTerminalReady = callback; }
