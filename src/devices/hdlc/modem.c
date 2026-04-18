/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * TCP networking for HDLC modem.
 *
 * Architecture:
 *   - All socket operations run in a background worker thread.
 *   - The emulation thread (Modem_Tick / Modem_SendBytes) never touches a socket.
 *   - Communication is via two lock-protected byte queues (RX and TX).
 *   - Worker handles DNS, connect, accept, reconnect with backoff, recv, send.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include "../devices_types.h"
#include "modem.h"

#ifdef MODEM_HAS_NETWORKING
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#endif

// ============================================================================
// Thread-safe queue operations
// ============================================================================

static void queue_init(ModemQueue *q)
{
    q->head = 0;
    q->tail = 0;
    pthread_mutex_init(&q->mtx, NULL);
}

static void queue_destroy(ModemQueue *q)
{
    pthread_mutex_destroy(&q->mtx);
}

// Returns number of bytes actually enqueued (may be less if full)
static int queue_write(ModemQueue *q, const uint8_t *data, int len)
{
    if (len <= 0) return 0;
    pthread_mutex_lock(&q->mtx);

    int written = 0;
    for (int i = 0; i < len; i++) {
        int next = (q->head + 1) % MODEM_QUEUE_SIZE;
        if (next == q->tail) break; // full
        q->buf[q->head] = data[i];
        q->head = next;
        written++;
    }

    pthread_mutex_unlock(&q->mtx);
    return written;
}

// Returns number of bytes read into dst
static int queue_read(ModemQueue *q, uint8_t *dst, int maxlen)
{
    if (maxlen <= 0) return 0;
    pthread_mutex_lock(&q->mtx);

    int count = 0;
    while (count < maxlen && q->tail != q->head) {
        dst[count++] = q->buf[q->tail];
        q->tail = (q->tail + 1) % MODEM_QUEUE_SIZE;
    }

    pthread_mutex_unlock(&q->mtx);
    return count;
}

// Check if queue has data (lock-free peek for hot path)
static bool queue_has_data(ModemQueue *q)
{
    return q->head != q->tail;
}

// ============================================================================
// Socket helpers
// ============================================================================

#ifdef MODEM_HAS_NETWORKING

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

// ============================================================================
// Worker thread: handles ALL networking
// ============================================================================

// Resolve hostname. Returns 0 on success, -1 on failure.
// This may block for DNS — that's fine, we're in the worker thread.
static int resolve_address(const char *host, int port, struct sockaddr_in *out)
{
    memset(out, 0, sizeof(*out));
    out->sin_family = AF_INET;
    out->sin_port = htons((uint16_t)port);

    // Try numeric IP first (instant)
    if (inet_pton(AF_INET, host, &out->sin_addr) == 1) {
        return 0;
    }

    // DNS lookup (may take seconds — worker thread, so that's fine)
    struct hostent *he = gethostbyname(host);
    if (!he) {
        return -1;
    }
    memcpy(&out->sin_addr, he->h_addr_list[0], (size_t)he->h_length);
    return 0;
}

// Try to connect. Returns connected fd or -1.
// Uses poll with timeout so it never blocks indefinitely.
static int try_connect(struct sockaddr_in *addr, int timeout_ms)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    // Set non-blocking for connect
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    int ret = connect(fd, (struct sockaddr *)addr, sizeof(*addr));
    if (ret == 0) {
        // Connected instantly
        fcntl(fd, F_SETFL, flags); // restore blocking
        set_nodelay(fd);
        return fd;
    }

    if (errno != EINPROGRESS) {
        close(fd);
        return -1;
    }

    // Wait for connect with timeout
    struct pollfd pfd = { .fd = fd, .events = POLLOUT };
    ret = poll(&pfd, 1, timeout_ms);

    if (ret <= 0) {
        // Timeout or error
        close(fd);
        return -1;
    }

    if (pfd.revents & (POLLERR | POLLHUP)) {
        close(fd);
        return -1;
    }

    // Check SO_ERROR
    int err = 0;
    socklen_t errlen = sizeof(err);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen);
    if (err != 0) {
        close(fd);
        return -1;
    }

    fcntl(fd, F_SETFL, flags); // restore blocking
    set_nodelay(fd);
    return fd;
}

// Sleep helper that checks shutdown flag, returns true if shutdown requested
static bool sleep_check_shutdown(ModemState *modem, int seconds)
{
    for (int i = 0; i < seconds * 10; i++) {
        if (atomic_load(&modem->shutdownReq)) return true;
        usleep(100000); // 100ms
    }
    return atomic_load(&modem->shutdownReq);
}

// ---- SERVER WORKER ----
static void *server_worker(void *arg)
{
    ModemState *modem = (ModemState *)arg;

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        fprintf(stderr, "Modem: Failed to create listen socket: %s\n", strerror(errno));
        return NULL;
    }

    int reuse = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)modem->port);

    if (bind(listenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Modem: Failed to bind port %d: %s\n", modem->port, strerror(errno));
        close(listenFd);
        return NULL;
    }

    if (listen(listenFd, 1) < 0) {
        fprintf(stderr, "Modem: Failed to listen: %s\n", strerror(errno));
        close(listenFd);
        return NULL;
    }

    fprintf(stderr, "Modem: Listening on port %d\n", modem->port);

    while (!atomic_load(&modem->shutdownReq)) {
        // Accept with timeout so we can check shutdown
        struct pollfd pfd = { .fd = listenFd, .events = POLLIN };
        int ret = poll(&pfd, 1, 500); // 500ms timeout
        if (ret <= 0) continue;

        struct sockaddr_in peer;
        socklen_t peerlen = sizeof(peer);
        int clientFd = accept(listenFd, (struct sockaddr *)&peer, &peerlen);
        if (clientFd < 0) continue;

        set_nodelay(clientFd);
        fprintf(stderr, "Modem: Accepted connection from %s:%d\n",
                inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
        atomic_store(&modem->connected, true);

        // Service this connection: shuttle data between socket and queues
        while (!atomic_load(&modem->shutdownReq)) {
            struct pollfd fds = { .fd = clientFd, .events = POLLIN };

            // Check if we have TX data to send
            if (queue_has_data(&modem->txQueue)) {
                fds.events |= POLLOUT;
            }

            int pr = poll(&fds, 1, 100); // 100ms
            if (pr < 0) break;

            // Read from socket -> RX queue
            if (fds.revents & POLLIN) {
                uint8_t buf[4096];
                ssize_t n = recv(clientFd, buf, sizeof(buf), 0);
                if (n <= 0) break; // disconnect or error
                queue_write(&modem->rxQueue, buf, (int)n);
            }

            // Write TX queue -> socket
            if (fds.revents & POLLOUT) {
                uint8_t buf[4096];
                int n = queue_read(&modem->txQueue, buf, sizeof(buf));
                if (n > 0) {
                    int sent = 0;
                    while (sent < n) {
                        ssize_t w = send(clientFd, buf + sent, (size_t)(n - sent), MSG_NOSIGNAL);
                        if (w <= 0) break;
                        sent += (int)w;
                    }
                }
            }

            if (fds.revents & (POLLERR | POLLHUP)) break;
        }

        // Connection ended
        close(clientFd);
        atomic_store(&modem->connected, false);
        fprintf(stderr, "Modem: Client disconnected, waiting for new connection...\n");
    }

    close(listenFd);
    return NULL;
}

// ---- CLIENT WORKER ----
static void *client_worker(void *arg)
{
    ModemState *modem = (ModemState *)arg;
    const char *host = modem->address[0] ? modem->address : "localhost";
    int attempt = 0;

    // Resolve address (may block for DNS — fine, we're in worker thread)
    struct sockaddr_in addr;
    if (resolve_address(host, modem->port, &addr) < 0) {
        fprintf(stderr, "Modem: Failed to resolve '%s' - giving up\n", host);
        return NULL;
    }
    fprintf(stderr, "Modem: Resolved %s -> %s\n", host, inet_ntoa(addr.sin_addr));

    while (!atomic_load(&modem->shutdownReq)) {
        attempt++;
        fprintf(stderr, "Modem: Connecting to %s:%d (attempt %d)...\n", host, modem->port, attempt);

        int clientFd = try_connect(&addr, 5000); // 5 second timeout
        if (clientFd < 0) {
            // Backoff: 1s, 2s, 4s, 8s, 16s, 30s max
            int delay = 1;
            for (int i = 1; i < attempt && i < 5; i++) delay *= 2;
            if (delay > 30) delay = 30;
            fprintf(stderr, "Modem: Connect failed, retry in %d seconds\n", delay);
            if (sleep_check_shutdown(modem, delay)) break;
            continue;
        }

        fprintf(stderr, "Modem: Connected to %s:%d\n", host, modem->port);
        atomic_store(&modem->connected, true);
        attempt = 0; // reset backoff on success

        // Service connection
        while (!atomic_load(&modem->shutdownReq)) {
            struct pollfd fds = { .fd = clientFd, .events = POLLIN };

            if (queue_has_data(&modem->txQueue)) {
                fds.events |= POLLOUT;
            }

            int pr = poll(&fds, 1, 100);
            if (pr < 0) break;

            if (fds.revents & POLLIN) {
                uint8_t buf[4096];
                ssize_t n = recv(clientFd, buf, sizeof(buf), 0);
                if (n <= 0) break;
                queue_write(&modem->rxQueue, buf, (int)n);
            }

            if (fds.revents & POLLOUT) {
                uint8_t buf[4096];
                int n = queue_read(&modem->txQueue, buf, sizeof(buf));
                if (n > 0) {
                    int sent = 0;
                    while (sent < n) {
                        ssize_t w = send(clientFd, buf + sent, (size_t)(n - sent), MSG_NOSIGNAL);
                        if (w <= 0) goto disconnected;
                        sent += (int)w;
                    }
                }
            }

            if (fds.revents & (POLLERR | POLLHUP)) break;
        }

disconnected:
        close(clientFd);
        atomic_store(&modem->connected, false);
        fprintf(stderr, "Modem: Disconnected from %s:%d\n", host, modem->port);

        if (!atomic_load(&modem->shutdownReq)) {
            fprintf(stderr, "Modem: Will reconnect in 1 second\n");
            if (sleep_check_shutdown(modem, 1)) break;
        }
    }

    return NULL;
}

#endif /* MODEM_HAS_NETWORKING */

// ============================================================================
// Public API — called from emulation thread
// ============================================================================

void Modem_Init(ModemState *modem, Device *hdlcDevice)
{
    if (!modem) return;
    memset(modem, 0, sizeof(ModemState));
    modem->hdlcDevice = hdlcDevice;

#ifdef MODEM_HAS_NETWORKING
    queue_init(&modem->rxQueue);
    queue_init(&modem->txQueue);
    atomic_store(&modem->connected, false);
    atomic_store(&modem->networkStarted, false);
    atomic_store(&modem->shutdownReq, false);
#endif
}

void Modem_Destroy(ModemState *modem)
{
    if (!modem) return;

#ifdef MODEM_HAS_NETWORKING
    // Signal worker to stop and wait for it
    atomic_store(&modem->shutdownReq, true);
    if (modem->workerRunning) {
        pthread_join(modem->workerThread, NULL);
        modem->workerRunning = false;
    }
    queue_destroy(&modem->rxQueue);
    queue_destroy(&modem->txQueue);
#endif

    atomic_store(&modem->connected, false);
    atomic_store(&modem->networkStarted, false);
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

#ifdef MODEM_HAS_NETWORKING
    atomic_store(&modem->networkStarted, true);

    // Spawn worker thread — ALL socket ops happen there
    int err;
    if (isServer) {
        err = pthread_create(&modem->workerThread, NULL, server_worker, modem);
    } else {
        err = pthread_create(&modem->workerThread, NULL, client_worker, modem);
    }

    if (err != 0) {
        fprintf(stderr, "Modem: Failed to create worker thread: %s\n", strerror(err));
    } else {
        modem->workerRunning = true;
    }
#else
    atomic_store(&modem->networkStarted, true);
    atomic_store(&modem->connected, true);
#endif

    // Signal DSR and SD immediately so SINTRAN sees hardware as present
    modem->dataSetReady = true;
    modem->signalDetector = true;
    if (modem->onDataSetReady) modem->onDataSetReady(modem->hdlcDevice, true);
    if (modem->onSignalDetector) modem->onSignalDetector(modem->hdlcDevice, true);
}

// Called from CPU loop. Never touches a socket. Just drains RX queue.
void Modem_Tick(ModemState *modem)
{
    if (!modem || !atomic_load(&modem->networkStarted)) return;

#ifdef MODEM_HAS_NETWORKING
    // Drain RX queue into HDLC receiver (no syscalls, just memcpy from queue)
    if (queue_has_data(&modem->rxQueue) && modem->onReceivedData) {
        uint8_t buf[4096];
        int n = queue_read(&modem->rxQueue, buf, sizeof(buf));
        if (n > 0) {
            modem->bytesRx += n;
            modem->onReceivedData(modem->hdlcDevice, buf, n);
        }
    }
#endif
}

// Called from emulation. Enqueues to TX queue, worker sends it.
void Modem_SendByte(ModemState *modem, uint8_t data)
{
    if (!modem) return;
    if (!atomic_load(&modem->connected)) {
        // Not connected — byte dropped (SINTRAN may send before TCP connects)
        return;
    }

#ifdef MODEM_HAS_NETWORKING
    queue_write(&modem->txQueue, &data, 1);
    modem->bytesTx++;
    // Debug: print every 1000 bytes to confirm counting
    if ((modem->bytesTx % 1000) == 0) {
        fprintf(stderr, "Modem: bytesTx=%" PRIu64 "\n", modem->bytesTx);
    }
#endif
}

void Modem_SendBytes(ModemState *modem, const uint8_t *data, int length)
{
    if (!modem || !data || length <= 0 || !atomic_load(&modem->connected)) return;

#ifdef MODEM_HAS_NETWORKING
    queue_write(&modem->txQueue, data, length);
    modem->bytesTx += length;
#endif
}

// ============================================================================
// Modem signal functions — called from emulation thread
// ============================================================================

void Modem_SetDTR(ModemState *modem, bool value)
{
    if (!modem || modem->dataTerminalReady == value) return;
    modem->dataTerminalReady = value;
    Modem_SetDSR(modem, value);
    if (modem->onDataTerminalReady) modem->onDataTerminalReady(modem->hdlcDevice, value);
}

void Modem_SetRTS(ModemState *modem, bool value)
{
    if (!modem || modem->requestToSend == value) return;
    modem->requestToSend = value;
    Modem_SetCTS(modem, value);
    if (modem->onRequestToSend) modem->onRequestToSend(modem->hdlcDevice, value);
}

void Modem_SetDSR(ModemState *modem, bool value)
{
    if (!modem || modem->dataSetReady == value) return;
    modem->dataSetReady = value;
    if (modem->onDataSetReady) modem->onDataSetReady(modem->hdlcDevice, value);
}

void Modem_SetCTS(ModemState *modem, bool value)
{
    if (!modem || modem->clearToSend == value) return;
    modem->clearToSend = value;
    if (modem->onClearToSend) modem->onClearToSend(modem->hdlcDevice, value);
}

// ============================================================================
// Callback setup
// ============================================================================

void Modem_SetReceivedDataCallback(ModemState *modem, ModemDataCallback callback)     { if (modem) modem->onReceivedData = callback; }
void Modem_SetRingIndicatorCallback(ModemState *modem, ModemSignalCallback callback)  { if (modem) modem->onRingIndicator = callback; }
void Modem_SetDataSetReadyCallback(ModemState *modem, ModemSignalCallback callback)   { if (modem) modem->onDataSetReady = callback; }
void Modem_SetSignalDetectorCallback(ModemState *modem, ModemSignalCallback callback) { if (modem) modem->onSignalDetector = callback; }
void Modem_SetClearToSendCallback(ModemState *modem, ModemSignalCallback callback)    { if (modem) modem->onClearToSend = callback; }
void Modem_SetRequestToSendCallback(ModemState *modem, ModemSignalCallback callback)  { if (modem) modem->onRequestToSend = callback; }
void Modem_SetDataTerminalReadyCallback(ModemState *modem, ModemSignalCallback callback) { if (modem) modem->onDataTerminalReady = callback; }
