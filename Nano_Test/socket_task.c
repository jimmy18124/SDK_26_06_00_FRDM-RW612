/*
 * Copyright 2022-2023, 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "socket_task.h"

#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app.h"
#include "fsl_debug_console.h"
#include "lwip/errno.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#define SOCKET_TASK_PRIORITY        (tskIDLE_PRIORITY + 2U)
#define SOCKET_TASK_STACK_DEPTH     1024U
#define SOCKET_RETRY_DELAY_MS       1000U
#define SOCKET_RECEIVE_TIMEOUT_MS   500U
#define SOCKET_RECEIVE_BUFFER_SIZE  1500U

static volatile int s_tcp_socket = -1;
static volatile int s_udp_socket = -1;
static struct sockaddr_in s_udp_peer_address;

static int set_receive_timeout(int socket_fd)
{
    struct timeval timeout;

    timeout.tv_sec  = SOCKET_RECEIVE_TIMEOUT_MS / 1000U;
    timeout.tv_usec = (SOCKET_RECEIVE_TIMEOUT_MS % 1000U) * 1000U;

    return setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

static void close_socket(volatile int *socket_fd)
{
    int fd = *socket_fd;

    *socket_fd = -1;
    if (fd >= 0)
    {
        (void)shutdown(fd, SHUT_RDWR);
        (void)close(fd);
    }
}

static void handle_tcp_data(const uint8_t *data, size_t length)
{
    (void)data;
    PRINTF("[TCP] Received %u bytes\r\n", (unsigned int)length);
}

static void handle_udp_data(const uint8_t *data, size_t length, const struct sockaddr_in *source)
{
    char source_ip[16] = {0};

    (void)data;
    (void)inet_ntop(AF_INET, &source->sin_addr, source_ip, sizeof(source_ip));
    PRINTF("[UDP] Received %u bytes from %s:%u\r\n", (unsigned int)length, source_ip,
           (unsigned int)ntohs(source->sin_port));
}

static void tcp_client_task(void *param)
{
    struct sockaddr_in server_address;
    uint8_t receive_buffer[SOCKET_RECEIVE_BUFFER_SIZE];
    int keep_alive = 1;

    (void)param;
    (void)memset(&server_address, 0, sizeof(server_address));
    server_address.sin_len    = sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons(APP_TCP_SERVER_PORT);

    if (inet_pton(AF_INET, APP_TCP_SERVER_IP, &server_address.sin_addr) != 1)
    {
        PRINTF("[TCP] Invalid server address: %s\r\n", APP_TCP_SERVER_IP);
        vTaskDelete(NULL);
    }

    for (;;)
    {
        int received;

        s_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_tcp_socket < 0)
        {
            PRINTF("[TCP] socket() failed, errno=%d; retrying\r\n", errno);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        (void)setsockopt(s_tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));
        PRINTF("[TCP] Connecting to %s:%u\r\n", APP_TCP_SERVER_IP, (unsigned int)APP_TCP_SERVER_PORT);

        if (connect(s_tcp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
        {
            PRINTF("[TCP] Connect failed, errno=%d; retrying\r\n", errno);
            close_socket(&s_tcp_socket);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        PRINTF("[TCP] Connected to %s:%u (socket=%d)\r\n", APP_TCP_SERVER_IP,
               (unsigned int)APP_TCP_SERVER_PORT, s_tcp_socket);

        if (set_receive_timeout(s_tcp_socket) != 0)
        {
            PRINTF("[TCP] Failed to set receive timeout, errno=%d\r\n", errno);
        }

        for (;;)
        {
            received = recv(s_tcp_socket, receive_buffer, sizeof(receive_buffer), 0);
            if (received > 0)
            {
                handle_tcp_data(receive_buffer, (size_t)received);
            }
            else if (received == 0)
            {
                PRINTF("[TCP] Server closed the connection\r\n");
                break;
            }
            else if (errno != EWOULDBLOCK)
            {
                PRINTF("[TCP] Connection lost, errno=%d\r\n", errno);
                break;
            }
        }

        close_socket(&s_tcp_socket);
        PRINTF("[TCP] Disconnected; reconnecting in %u ms\r\n", (unsigned int)SOCKET_RETRY_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
    }
}

static void udp_task(void *param)
{
    struct sockaddr_in local_address;
    uint8_t receive_buffer[SOCKET_RECEIVE_BUFFER_SIZE];
    int reuse_address = 1;

    (void)param;
    (void)memset(&local_address, 0, sizeof(local_address));
    local_address.sin_len         = sizeof(local_address);
    local_address.sin_family      = AF_INET;
    local_address.sin_port        = htons(APP_UDP_LOCAL_PORT);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);

    for (;;)
    {
        s_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s_udp_socket < 0)
        {
            PRINTF("[UDP] socket() failed, errno=%d; retrying\r\n", errno);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        (void)setsockopt(s_udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address));
        if (bind(s_udp_socket, (struct sockaddr *)&local_address, sizeof(local_address)) != 0)
        {
            PRINTF("[UDP] Bind port %u failed, errno=%d; retrying\r\n", (unsigned int)APP_UDP_LOCAL_PORT,
                   errno);
            close_socket(&s_udp_socket);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        if (set_receive_timeout(s_udp_socket) != 0)
        {
            PRINTF("[UDP] Failed to set receive timeout, errno=%d\r\n", errno);
        }

        PRINTF("[UDP] Listening on 0.0.0.0:%u; peer=%s:%u (socket=%d)\r\n",
               (unsigned int)APP_UDP_LOCAL_PORT, APP_UDP_PEER_IP, (unsigned int)APP_UDP_PEER_PORT, s_udp_socket);

        for (;;)
        {
            struct sockaddr_in source_address;
            socklen_t source_length = sizeof(source_address);
            int received = recvfrom(s_udp_socket, receive_buffer, sizeof(receive_buffer), 0,
                                    (struct sockaddr *)&source_address, &source_length);

            if (received > 0)
            {
                handle_udp_data(receive_buffer, (size_t)received, &source_address);
            }
            else if ((received < 0) && (errno != EWOULDBLOCK))
            {
                PRINTF("[UDP] Receive failed, errno=%d; recreating socket\r\n", errno);
                break;
            }
        }

        close_socket(&s_udp_socket);
        vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
    }
}

int socket_tasks_start(void)
{
    TaskHandle_t tcp_task_handle = NULL;

    (void)memset(&s_udp_peer_address, 0, sizeof(s_udp_peer_address));
    s_udp_peer_address.sin_len    = sizeof(s_udp_peer_address);
    s_udp_peer_address.sin_family = AF_INET;
    s_udp_peer_address.sin_port   = htons(APP_UDP_PEER_PORT);

    if (inet_pton(AF_INET, APP_UDP_PEER_IP, &s_udp_peer_address.sin_addr) != 1)
    {
        PRINTF("[UDP] Invalid peer address: %s\r\n", APP_UDP_PEER_IP);
        return -1;
    }

    if (xTaskCreate(tcp_client_task, "tcp_client", SOCKET_TASK_STACK_DEPTH, NULL, SOCKET_TASK_PRIORITY,
                    &tcp_task_handle) != pdPASS)
    {
        return -1;
    }

    if (xTaskCreate(udp_task, "udp", SOCKET_TASK_STACK_DEPTH, NULL, SOCKET_TASK_PRIORITY, NULL) != pdPASS)
    {
        vTaskDelete(tcp_task_handle);
        return -1;
    }

    return 0;
}

int socket_tcp_send(const void *data, size_t length)
{
    int socket_fd = s_tcp_socket;

    if ((socket_fd < 0) || (data == NULL) || (length == 0U))
    {
        return -1;
    }

    return send(socket_fd, data, length, 0);
}

int socket_udp_send(const void *data, size_t length)
{
    int socket_fd = s_udp_socket;

    if ((socket_fd < 0) || (data == NULL) || (length == 0U))
    {
        return -1;
    }

    return sendto(socket_fd, data, length, 0, (const struct sockaddr *)&s_udp_peer_address,
                  sizeof(s_udp_peer_address));
}
