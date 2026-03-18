/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
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
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "telnetserver.h"
#include "ndlib_types.h"
#include "ndlib_protos.h"

#define TELNET_MAX_TERMINALS 16
#define TELNET_OUTPUT_BUF_SIZE 4096

// Telnet protocol constants
#define IAC   255
#define WILL  251
#define WONT  252
#define DO    253
#define DONT  254
#define SB    250
#define SE    240

#define OPT_ECHO     1
#define OPT_SGA      3
#define OPT_LINEMODE 34

// Telnet IAC state machine states
typedef enum {
    TELNET_STATE_DATA,
    TELNET_STATE_IAC,
    TELNET_STATE_WILL,
    TELNET_STATE_WONT,
    TELNET_STATE_DO,
    TELNET_STATE_DONT,
    TELNET_STATE_SB,
    TELNET_STATE_SB_DATA,
    TELNET_STATE_SB_IAC,
} TelnetIACState;

typedef struct {
    TelnetTerminalInfo info;
    int clientFd;
    pthread_t clientThread;
    bool clientThreadActive;
    bool locallyActive;         // Terminal claimed by local VScreen
    char clientAddrStr[48];
    // Output ring buffer (emulation thread -> client thread)
    uint8_t outputBuf[TELNET_OUTPUT_BUF_SIZE];
    volatile int outputHead;
    volatile int outputTail;
    pthread_mutex_t outputMutex;
} RegisteredTerminal;

struct TelnetServer {
    TelnetServerConfig config;
    RegisteredTerminal terminals[TELNET_MAX_TERMINALS];
    int terminalCount;
    pthread_t acceptThread;
    int listenFd;
    atomic_bool shouldExit;
    bool running;
    int shutdownPipe[2];
};

// Telnet initialization sequence: character mode, server echo
static const uint8_t telnet_init[] = {
    IAC, WILL, OPT_ECHO,       // Server will echo
    IAC, WILL, OPT_SGA,        // Suppress go-ahead (character-at-a-time)
    IAC, DONT, OPT_LINEMODE,   // Don't use linemode
};

// Forward declarations
static void *accept_thread_func(void *arg);
static void *client_thread_func(void *arg);
static RegisteredTerminal *find_by_device(TelnetServer *server, struct Device *device);

// Ring buffer write (called from emulation thread via output handler)
static void ringbuf_write(RegisteredTerminal *rt, uint8_t byte)
{
    pthread_mutex_lock(&rt->outputMutex);
    int next = (rt->outputHead + 1) % TELNET_OUTPUT_BUF_SIZE;
    if (next != rt->outputTail) {
        rt->outputBuf[rt->outputHead] = byte;
        rt->outputHead = next;
    }
    // If full, drop the byte (avoid blocking emulation thread)
    pthread_mutex_unlock(&rt->outputMutex);
}

// Ring buffer read (called from client thread)
static int ringbuf_read(RegisteredTerminal *rt, uint8_t *buf, int maxlen)
{
    pthread_mutex_lock(&rt->outputMutex);
    int count = 0;
    while (count < maxlen && rt->outputTail != rt->outputHead) {
        buf[count++] = rt->outputBuf[rt->outputTail];
        rt->outputTail = (rt->outputTail + 1) % TELNET_OUTPUT_BUF_SIZE;
    }
    pthread_mutex_unlock(&rt->outputMutex);
    return count;
}

// Output handler that chains to VScreen and queues for telnet client
void telnet_output_handler(struct Device *device, char c)
{
    // We need to find the server and terminal from device pointer.
    // Since we store a reference in a static, we use per-terminal approach.
    // The server pointer is embedded via closure-like technique using the
    // registered terminal's info.origOutput for chaining.
    // We'll use a global server pointer for the lookup.
    // This is set during TelnetServer_Start.
    extern TelnetServer *g_telnetServer;
    if (!g_telnetServer) return;

    RegisteredTerminal *rt = find_by_device(g_telnetServer, device);
    if (rt) {
        if (rt->clientFd >= 0) {
            ringbuf_write(rt, (uint8_t)c);
        }
        if (rt->info.origOutput) {
            rt->info.origOutput(device, c);
        }
    }
}

/*
 * Global server pointer for the telnet output handler.
 *
 * Thread safety: set once in TelnetServer_Start(), read by
 * telnet_output_handler() (called from emulation thread), and
 * cleared in TelnetServer_Stop(). Not protected by a mutex —
 * relies on start/stop ordering (start before emulation begins,
 * stop after emulation ends).
 */
TelnetServer *g_telnetServer = NULL;

static RegisteredTerminal *find_by_device(TelnetServer *server, struct Device *device)
{
    for (int i = 0; i < server->terminalCount; i++) {
        if (server->terminals[i].info.device == device) {
            return &server->terminals[i];
        }
    }
    return NULL;
}

TelnetServer *TelnetServer_Create(const TelnetServerConfig *config)
{
    TelnetServer *server = calloc(1, sizeof(TelnetServer));
    if (!server) return NULL;

    server->config = *config;
    if (server->config.port <= 0) server->config.port = 9000;
    if (server->config.maxConnections <= 0) server->config.maxConnections = 8;

    server->listenFd = -1;
    server->shutdownPipe[0] = -1;
    server->shutdownPipe[1] = -1;
    atomic_store(&server->shouldExit, false);

    for (int i = 0; i < TELNET_MAX_TERMINALS; i++) {
        server->terminals[i].clientFd = -1;
        pthread_mutex_init(&server->terminals[i].outputMutex, NULL);
    }

    return server;
}

bool TelnetServer_RegisterTerminal(TelnetServer *server, const TelnetTerminalInfo *info)
{
    if (!server || !info) return false;
    if (server->terminalCount >= TELNET_MAX_TERMINALS) return false;

    RegisteredTerminal *rt = &server->terminals[server->terminalCount];
    rt->info = *info;
    rt->clientFd = -1;
    rt->clientThreadActive = false;
    rt->outputHead = 0;
    rt->outputTail = 0;
    memset(rt->clientAddrStr, 0, sizeof(rt->clientAddrStr));

    server->terminalCount++;
    return true;
}

bool TelnetServer_Start(TelnetServer *server)
{
    if (!server || server->running) return false;
    if (server->terminalCount == 0) return false;

    // Set global pointer for output handler
    g_telnetServer = server;

    // Create shutdown pipe
    if (pipe(server->shutdownPipe) < 0) {
        perror("telnet: pipe");
        return false;
    }

    // Create listening socket
    server->listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listenFd < 0) {
        perror("telnet: socket");
        return false;
    }

    int opt = 1;
    setsockopt(server->listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(server->config.port);

    if (bind(server->listenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("telnet: bind");
        close(server->listenFd);
        server->listenFd = -1;
        return false;
    }

    if (listen(server->listenFd, 4) < 0) {
        perror("telnet: listen");
        close(server->listenFd);
        server->listenFd = -1;
        return false;
    }

    atomic_store(&server->shouldExit, false);

    if (pthread_create(&server->acceptThread, NULL, accept_thread_func, server) != 0) {
        perror("telnet: pthread_create");
        close(server->listenFd);
        server->listenFd = -1;
        return false;
    }

    server->running = true;
    Log(LOG_INFO, "Telnet server started on port %d (%d terminals available)\n",
           server->config.port, server->terminalCount);
    return true;
}

void TelnetServer_Stop(TelnetServer *server)
{
    if (!server || !server->running) return;

    atomic_store(&server->shouldExit, true);

    // Wake accept thread via shutdown pipe
    if (server->shutdownPipe[1] >= 0) {
        char dummy = 'x';
        ssize_t __attribute__((unused)) n = write(server->shutdownPipe[1], &dummy, 1);
    }

    // Close listening socket to unblock accept
    if (server->listenFd >= 0) {
        close(server->listenFd);
        server->listenFd = -1;
    }

    // Wait for accept thread
    pthread_join(server->acceptThread, NULL);

    // Disconnect all clients
    for (int i = 0; i < server->terminalCount; i++) {
        RegisteredTerminal *rt = &server->terminals[i];
        if (rt->clientFd >= 0) {
            close(rt->clientFd);
            rt->clientFd = -1;
        }
        if (rt->clientThreadActive) {
            pthread_join(rt->clientThread, NULL);
            rt->clientThreadActive = false;
        }
    }

    // Close shutdown pipe
    if (server->shutdownPipe[0] >= 0) {
        close(server->shutdownPipe[0]);
        server->shutdownPipe[0] = -1;
    }
    if (server->shutdownPipe[1] >= 0) {
        close(server->shutdownPipe[1]);
        server->shutdownPipe[1] = -1;
    }

    server->running = false;
    g_telnetServer = NULL;
    Log(LOG_INFO, "Telnet server stopped\n");
}

void TelnetServer_Destroy(TelnetServer *server)
{
    if (!server) return;

    if (server->running) {
        TelnetServer_Stop(server);
    }

    for (int i = 0; i < TELNET_MAX_TERMINALS; i++) {
        pthread_mutex_destroy(&server->terminals[i].outputMutex);
    }

    free(server);
}

int TelnetServer_GetTerminalCount(TelnetServer *server)
{
    return server ? server->terminalCount : 0;
}

bool TelnetServer_GetTerminalStatus(TelnetServer *server, int index,
    const char **name, uint16_t *identCode, bool *connected, bool *locallyActive,
    char *clientAddr, int addrLen)
{
    if (!server || index < 0 || index >= server->terminalCount) return false;

    RegisteredTerminal *rt = &server->terminals[index];
    if (name) *name = rt->info.name;
    if (identCode) *identCode = rt->info.identCode;
    if (connected) *connected = (rt->clientFd >= 0);
    if (locallyActive) *locallyActive = rt->locallyActive;
    if (clientAddr && addrLen > 0) {
        strncpy(clientAddr, rt->clientAddrStr, addrLen - 1);
        clientAddr[addrLen - 1] = '\0';
    }
    return true;
}

bool TelnetServer_DisconnectTerminal(TelnetServer *server, int index)
{
    if (!server || index < 0 || index >= server->terminalCount) return false;

    RegisteredTerminal *rt = &server->terminals[index];
    if (rt->clientFd < 0) return false;

    // Close socket - client thread will detect and clean up
    close(rt->clientFd);
    rt->clientFd = -1;

    // Wait for client thread to finish
    if (rt->clientThreadActive) {
        pthread_join(rt->clientThread, NULL);
        rt->clientThreadActive = false;
    }

    // Set carrier missing
    if (rt->info.carrierFunc) {
        rt->info.carrierFunc(rt->info.device, true);
    }

    memset(rt->clientAddrStr, 0, sizeof(rt->clientAddrStr));
    Log(LOG_INFO, "Telnet: disconnected %s\n", rt->info.name);
    return true;
}

bool TelnetServer_DisconnectDevice(TelnetServer *server, struct Device *device)
{
    if (!server || !device) return false;
    for (int i = 0; i < server->terminalCount; i++) {
        if (server->terminals[i].info.device == device) {
            return TelnetServer_DisconnectTerminal(server, i);
        }
    }
    return false;
}

int TelnetServer_GetPort(TelnetServer *server)
{
    return server ? server->config.port : 0;
}

bool TelnetServer_SetTerminalLocallyActive(TelnetServer *server, int index, bool active)
{
    if (!server || index < 0 || index >= server->terminalCount) return false;
    server->terminals[index].locallyActive = active;
    return true;
}

bool TelnetServer_SetDeviceLocallyActive(TelnetServer *server, struct Device *device, bool active)
{
    if (!server || !device) return false;
    RegisteredTerminal *rt = find_by_device(server, device);
    if (!rt) return false;
    rt->locallyActive = active;
    return true;
}

bool TelnetServer_IsDeviceConnected(TelnetServer *server, struct Device *device)
{
    if (!server || !device) return false;
    RegisteredTerminal *rt = find_by_device(server, device);
    return (rt && rt->clientFd >= 0);
}

void TelnetServer_ClearDeviceCarrier(TelnetServer *server, struct Device *device)
{
    if (!server || !device) return;
    RegisteredTerminal *rt = find_by_device(server, device);
    if (rt && rt->info.carrierFunc) {
        rt->info.carrierFunc(rt->info.device, false);  // false = carrier present
    }
}

// Menu index mapping: displayed menu numbers -> terminal indices
// Only available (not locally active, not connected) terminals are shown
static int menuMap[TELNET_MAX_TERMINALS];
static int menuMapCount = 0;

// Send terminal selection menu to client (only shows available terminals)
static void send_menu(TelnetServer *server, int clientFd)
{
    char buf[2048];
    int pos = 0;

    // Build mapping of available terminals
    menuMapCount = 0;
    for (int i = 0; i < server->terminalCount; i++) {
        RegisteredTerminal *rt = &server->terminals[i];
        if (rt->clientFd < 0 && !rt->locallyActive) {
            menuMap[menuMapCount++] = i;
        }
    }

    pos += snprintf(buf + pos, sizeof(buf) - pos,
        "\r\n"
        "ND-100/CX Terminal Server (port %d)\r\n"
        "\r\n",
        server->config.port);

    if (menuMapCount == 0) {
        pos += snprintf(buf + pos, sizeof(buf) - pos,
            "No terminals available.\r\n"
            "All terminals are in use by local console or other telnet clients.\r\n"
            "\r\nPress Q to quit: ");
    } else {
        pos += snprintf(buf + pos, sizeof(buf) - pos,
            "Available terminals:\r\n");

        for (int m = 0; m < menuMapCount; m++) {
            RegisteredTerminal *rt = &server->terminals[menuMap[m]];
            pos += snprintf(buf + pos, sizeof(buf) - pos,
                "  %d) %s\r\n",
                m + 1, rt->info.name);
        }

        pos += snprintf(buf + pos, sizeof(buf) - pos,
            "\r\nSelect terminal (1-%d), or Q to quit: ",
            menuMapCount);
    }

    send(clientFd, buf, pos, MSG_NOSIGNAL);
}

// Accept thread: listens for new connections, shows menu, spawns client threads
static void *accept_thread_func(void *arg)
{
    TelnetServer *server = (TelnetServer *)arg;

    struct pollfd pfds[2];
    pfds[0].fd = server->listenFd;
    pfds[0].events = POLLIN;
    pfds[1].fd = server->shutdownPipe[0];
    pfds[1].events = POLLIN;

    while (!atomic_load(&server->shouldExit)) {
        int ret = poll(pfds, 2, 1000);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (ret == 0) continue;

        // Shutdown signal
        if (pfds[1].revents & POLLIN) break;

        // New connection
        if (!(pfds[0].revents & POLLIN)) continue;

        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientFd = accept(server->listenFd, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientFd < 0) {
            if (errno == EINTR || errno == EBADF) continue;
            perror("telnet: accept");
            continue;
        }

        // Enable TCP_NODELAY for responsiveness
        int opt = 1;
        setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

        // Send telnet negotiation
        send(clientFd, telnet_init, sizeof(telnet_init), MSG_NOSIGNAL);

        // Format client address string
        char addrStr[48];
        snprintf(addrStr, sizeof(addrStr), "%s:%d",
                 inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        Log(LOG_INFO, "Telnet: connection from %s\n", addrStr);

        // Terminal selection loop
        bool assigned = false;
        while (!atomic_load(&server->shouldExit) && !assigned) {
            send_menu(server, clientFd);

            // Read selection with poll timeout
            struct pollfd cpfd[2];
            cpfd[0].fd = clientFd;
            cpfd[0].events = POLLIN;
            cpfd[1].fd = server->shutdownPipe[0];
            cpfd[1].events = POLLIN;

            uint8_t inputBuf[64];
            TelnetIACState iacState = TELNET_STATE_DATA;
            int selection = -1;

            while (!atomic_load(&server->shouldExit) && selection == -1) {
                int pret = poll(cpfd, 2, 5000);
                if (pret < 0) {
                    if (errno == EINTR) continue;
                    break;
                }
                if (cpfd[1].revents & POLLIN) break;
                if (pret == 0) continue;
                if (!(cpfd[0].revents & POLLIN)) continue;

                int n = recv(clientFd, inputBuf, sizeof(inputBuf), 0);
                if (n <= 0) {
                    // Client disconnected during menu
                    close(clientFd);
                    Log(LOG_INFO, "Telnet: %s disconnected during menu\n", addrStr);
                    goto next_connection;
                }

                // Process received bytes through IAC state machine
                for (int i = 0; i < n; i++) {
                    uint8_t byte = inputBuf[i];

                    switch (iacState) {
                    case TELNET_STATE_DATA:
                        if (byte == IAC) {
                            iacState = TELNET_STATE_IAC;
                        } else if (byte == 'q' || byte == 'Q') {
                            // Quit
                            const char *bye = "\r\nGoodbye.\r\n";
                            send(clientFd, bye, strlen(bye), MSG_NOSIGNAL);
                            close(clientFd);
                            Log(LOG_INFO, "Telnet: %s quit\n", addrStr);
                            goto next_connection;
                        } else if (byte >= '1' && byte <= '9') {
                            selection = byte - '1';
                        }
                        // Ignore CR, LF, and other control chars
                        break;
                    case TELNET_STATE_IAC:
                        switch (byte) {
                        case WILL: iacState = TELNET_STATE_WILL; break;
                        case WONT: iacState = TELNET_STATE_WONT; break;
                        case DO:   iacState = TELNET_STATE_DO;   break;
                        case DONT: iacState = TELNET_STATE_DONT; break;
                        case SB:   iacState = TELNET_STATE_SB;   break;
                        case IAC:  iacState = TELNET_STATE_DATA;  break;  // Escaped 255
                        default:   iacState = TELNET_STATE_DATA;  break;
                        }
                        break;
                    case TELNET_STATE_WILL:
                    case TELNET_STATE_WONT:
                    case TELNET_STATE_DO:
                    case TELNET_STATE_DONT:
                        // Consume the option byte and return to data state
                        iacState = TELNET_STATE_DATA;
                        break;
                    case TELNET_STATE_SB:
                        iacState = TELNET_STATE_SB_DATA;
                        break;
                    case TELNET_STATE_SB_DATA:
                        if (byte == IAC) iacState = TELNET_STATE_SB_IAC;
                        break;
                    case TELNET_STATE_SB_IAC:
                        if (byte == SE) iacState = TELNET_STATE_DATA;
                        else iacState = TELNET_STATE_SB_DATA;
                        break;
                    }
                }
            }

            if (selection < 0 || selection >= menuMapCount) {
                if (atomic_load(&server->shouldExit)) break;
                continue;
            }

            // Map menu selection to actual terminal index
            int termIdx = menuMap[selection];
            RegisteredTerminal *rt = &server->terminals[termIdx];
            if (rt->clientFd >= 0 || rt->locallyActive) {
                // State changed since menu was shown — redisplay
                continue;
            }

            // Wait for previous client thread to finish if needed
            if (rt->clientThreadActive) {
                pthread_join(rt->clientThread, NULL);
                rt->clientThreadActive = false;
            }

            // Assign client to terminal
            rt->clientFd = clientFd;
            strncpy(rt->clientAddrStr, addrStr, sizeof(rt->clientAddrStr) - 1);
            rt->clientAddrStr[sizeof(rt->clientAddrStr) - 1] = '\0';

            // Clear the ring buffer
            pthread_mutex_lock(&rt->outputMutex);
            rt->outputHead = 0;
            rt->outputTail = 0;
            pthread_mutex_unlock(&rt->outputMutex);

            // Send connected message
            char connMsg[128];
            snprintf(connMsg, sizeof(connMsg),
                     "\r\nConnected to %s\r\n\r\n", rt->info.name);
            send(clientFd, connMsg, strlen(connMsg), MSG_NOSIGNAL);

            // Clear carrier missing
            if (rt->info.carrierFunc) {
                rt->info.carrierFunc(rt->info.device, false);
            }

            Log(LOG_INFO, "Telnet: %s connected to %s\n", addrStr, rt->info.name);

            // Spawn client I/O thread
            if (pthread_create(&rt->clientThread, NULL, client_thread_func, rt) != 0) {
                perror("telnet: client thread");
                close(clientFd);
                rt->clientFd = -1;
                memset(rt->clientAddrStr, 0, sizeof(rt->clientAddrStr));
            } else {
                rt->clientThreadActive = true;
            }

            assigned = true;
        }
next_connection:;
    }

    return NULL;
}

// Client I/O thread: handles bidirectional data between telnet client and terminal
static void *client_thread_func(void *arg)
{
    RegisteredTerminal *rt = (RegisteredTerminal *)arg;
    if (!rt || !g_telnetServer) return NULL;

    TelnetServer *server = g_telnetServer;
    int fd = rt->clientFd;
    TelnetIACState iacState = TELNET_STATE_DATA;

    struct pollfd pfds[2];
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = server->shutdownPipe[0];
    pfds[1].events = POLLIN;

    while (!atomic_load(&server->shouldExit) && rt->clientFd >= 0) {
        int ret = poll(pfds, 2, 50);  // 50ms timeout to check ring buffer

        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        // Shutdown signal
        if (pfds[1].revents & POLLIN) break;

        // Handle incoming data from client
        if (ret > 0 && (pfds[0].revents & POLLIN)) {
            uint8_t buf[256];
            int n = recv(fd, buf, sizeof(buf), 0);
            if (n <= 0) {
                // Client disconnected
                break;
            }

            // Process through IAC state machine, pass data bytes to terminal
            for (int i = 0; i < n; i++) {
                uint8_t byte = buf[i];

                switch (iacState) {
                case TELNET_STATE_DATA:
                    if (byte == IAC) {
                        iacState = TELNET_STATE_IAC;
                    } else {
                        // Map CR to CR (ND doesn't like LF)
                        if (byte == '\n') byte = '\r';
                        if (rt->info.inputFunc) {
                            rt->info.inputFunc(rt->info.device, byte);
                        }
                    }
                    break;
                case TELNET_STATE_IAC:
                    switch (byte) {
                    case WILL: iacState = TELNET_STATE_WILL; break;
                    case WONT: iacState = TELNET_STATE_WONT; break;
                    case DO:   iacState = TELNET_STATE_DO;   break;
                    case DONT: iacState = TELNET_STATE_DONT; break;
                    case SB:   iacState = TELNET_STATE_SB;   break;
                    case IAC:
                        // Escaped 255 - pass as data
                        if (rt->info.inputFunc) {
                            rt->info.inputFunc(rt->info.device, byte);
                        }
                        iacState = TELNET_STATE_DATA;
                        break;
                    default:
                        iacState = TELNET_STATE_DATA;
                        break;
                    }
                    break;
                case TELNET_STATE_WILL:
                case TELNET_STATE_WONT:
                case TELNET_STATE_DO:
                case TELNET_STATE_DONT:
                    iacState = TELNET_STATE_DATA;
                    break;
                case TELNET_STATE_SB:
                    iacState = TELNET_STATE_SB_DATA;
                    break;
                case TELNET_STATE_SB_DATA:
                    if (byte == IAC) iacState = TELNET_STATE_SB_IAC;
                    break;
                case TELNET_STATE_SB_IAC:
                    if (byte == SE) iacState = TELNET_STATE_DATA;
                    else iacState = TELNET_STATE_SB_DATA;
                    break;
                }
            }
        }

        // Drain output ring buffer and send to client
        uint8_t outbuf[512];
        int outlen = ringbuf_read(rt, outbuf, sizeof(outbuf));
        if (outlen > 0 && rt->clientFd >= 0) {
            int sent = send(fd, outbuf, outlen, MSG_NOSIGNAL);
            if (sent < 0) {
                break;  // Client disconnected
            }
        }
    }

    // Clean up
    if (rt->clientFd >= 0) {
        close(rt->clientFd);
        rt->clientFd = -1;
    }
    memset(rt->clientAddrStr, 0, sizeof(rt->clientAddrStr));

    // Set carrier missing
    if (rt->info.carrierFunc) {
        rt->info.carrierFunc(rt->info.device, true);
    }

    Log(LOG_INFO, "Telnet: client disconnected from %s\n", rt->info.name);
    return NULL;
}
