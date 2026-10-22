#include "socket_task.h"
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "app.h"
#include "fsl_debug_console.h"
#include "lwip/errno.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#define SOCKET_TASK_PRIORITY          (tskIDLE_PRIORITY + 2U)
#define TCP_MANAGER_TASK_STACK_DEPTH  512U
#define TCP_TX_TASK_STACK_DEPTH       1024U
#define TCP_RX_TASK_STACK_DEPTH       1024U
#define UDP_TASK_STACK_DEPTH          512U
#define SOCKET_RETRY_DELAY_MS         1000U
#define SOCKET_RECEIVE_TIMEOUT_MS     500U
#define SOCKET_RECEIVE_BUFFER_SIZE    1500U
#define TCP_SEND_BUFFER_CAPACITY      (4U * 1024U)
#define TCP_FRAME_OVERHEAD_BYTES      16U

#define TCP_EVENT_CONNECTED           (1U << 0)
#define TCP_EVENT_IO_ERROR            (1U << 1)
#define TCP_EVENT_TX_IDLE             (1U << 2)
#define TCP_EVENT_RX_IDLE             (1U << 3)
#define TCP_EVENT_WORKERS_IDLE        (TCP_EVENT_TX_IDLE | TCP_EVENT_RX_IDLE)

static volatile int s_tcp_socket = -1;
static volatile int s_udp_socket = -1;
static struct sockaddr_in s_tcp_server_address;
static struct sockaddr_in s_udp_peer_address;
static EventGroupHandle_t s_tcp_events;
static TaskHandle_t s_tcp_tx_task;
static TaskHandle_t s_tcp_rx_task;

/* TCP traffic settings: rate, bytes per send, and bytes per socket write. */
static const uint32_t s_tcp_sends_per_second = 20U;
static const size_t s_tcp_bytes_per_send = 100U * 1024U;
static const size_t s_tcp_chunk_size_bytes = 1U * 1024U;
static const uint32_t s_tcp_send_timeout_ms = 3000U;
static const uint32_t s_tcp_send_timeout_retries = 1U;
static const uint8_t s_tcp_frame_header[4] = {0xA0U, 0xB0U, 0xC0U, 0xD0U};
static const uint8_t s_tcp_frame_tail[4] = {0xAFU, 0xBFU, 0xCFU, 0xDFU};
static uint8_t s_tcp_transmit_buffer[TCP_SEND_BUFFER_CAPACITY];
static uint8_t s_tcp_receive_buffer[SOCKET_RECEIVE_BUFFER_SIZE];
static uint8_t s_udp_receive_buffer[SOCKET_RECEIVE_BUFFER_SIZE];

static void handle_tcp_data(const uint8_t *data, size_t length);

static int tcp_connection_active(void)
{
    return (s_tcp_events != NULL) && ((xEventGroupGetBits(s_tcp_events) & TCP_EVENT_CONNECTED) != 0U);
}

static int tcp_send_config_valid(void)
{
    TickType_t ticks_per_second = pdMS_TO_TICKS(1000U);

    if ((s_tcp_sends_per_second == 0U) || (s_tcp_sends_per_second > ticks_per_second) ||
        (s_tcp_bytes_per_send == 0U) || (s_tcp_chunk_size_bytes < TCP_FRAME_OVERHEAD_BYTES) ||
        (s_tcp_chunk_size_bytes > TCP_SEND_BUFFER_CAPACITY) ||
        ((s_tcp_bytes_per_send % s_tcp_chunk_size_bytes) != 0U) ||
        ((s_tcp_bytes_per_send / s_tcp_chunk_size_bytes) > UINT16_MAX))
    {
        PRINTF("[TCP] Invalid send config: rate=%u size=%u chunk=%u\r\n",
               (unsigned int)s_tcp_sends_per_second,
               (unsigned int)s_tcp_bytes_per_send,
               (unsigned int)s_tcp_chunk_size_bytes);
        return 0;
    }

    return 1;
}

static int init_ipv4_address(struct sockaddr_in *address, const char *ip, uint16_t port)
{
    (void)memset(address, 0, sizeof(*address));
    address->sin_len    = sizeof(*address);
    address->sin_family = AF_INET;
    address->sin_port   = htons(port);

    return (inet_pton(AF_INET, ip, &address->sin_addr) == 1) ? 0 : -1;
}

static int set_socket_timeout(int socket_fd, int option, uint32_t timeout_ms)
{
    struct timeval timeout;

    timeout.tv_sec  = timeout_ms / 1000U;
    timeout.tv_usec = (timeout_ms % 1000U) * 1000U;

    return setsockopt(socket_fd, SOL_SOCKET, option, &timeout, sizeof(timeout));
}

static int configure_tcp_keepalive(int socket_fd)
{
    int keep_alive = 1;
    int keep_idle = APP_TCP_KEEPALIVE_IDLE_SEC;
    int keep_interval = APP_TCP_KEEPALIVE_INTERVAL_SEC;
    int keep_count = APP_TCP_KEEPALIVE_COUNT;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)) != 0)
    {
        PRINTF("[TCP] Failed to enable keepalive, errno=%d\r\n", errno);
        return -1;
    }

    if ((setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle)) != 0) ||
        (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval)) != 0) ||
        (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keep_count, sizeof(keep_count)) != 0))
    {
        PRINTF("[TCP] Failed to configure keepalive, errno=%d\r\n", errno);
        return -1;
    }

    PRINTF("[TCP] Keepalive enabled: idle=%u s interval=%u s count=%u\r\n",
           (unsigned int)keep_idle, (unsigned int)keep_interval, (unsigned int)keep_count);
    return 0;
}

static int send_tcp_all(int socket_fd, const void *data, size_t length)
{
    const uint8_t *buffer = (const uint8_t *)data;
    size_t sent_total = 0U;
    uint32_t timeout_retries = 0U;

    while (sent_total < length)
    {
        if (!tcp_connection_active())
        {
            return -1;
        }

        ssize_t sent = send(socket_fd, &buffer[sent_total], length - sent_total, 0);

        if (sent > 0)
        {
            sent_total += (size_t)sent;
            timeout_retries = 0U;
        }
        else if ((sent < 0) && (errno == EINTR))
        {
            continue;
        }
        else if ((sent < 0) && (errno == EWOULDBLOCK) &&
                 (timeout_retries < s_tcp_send_timeout_retries))
        {
            timeout_retries++;
            PRINTF("[TCP] Send backpressure; retry %u/%u\r\n",
                   (unsigned int)timeout_retries,
                   (unsigned int)s_tcp_send_timeout_retries);
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

static void build_tcp_frame(uint8_t *frame,
                            size_t frame_size,
                            uint32_t block_index,
                            uint16_t frame_count,
                            uint16_t frame_number)
{
    size_t offset = 0U;
    uint32_t network_block_index;
    uint16_t network_frame_value;

    (void)memcpy(&frame[offset], s_tcp_frame_header, sizeof(s_tcp_frame_header));
    offset += sizeof(s_tcp_frame_header);

    network_block_index = htonl(block_index);
    (void)memcpy(&frame[offset], &network_block_index, sizeof(network_block_index));
    offset += sizeof(network_block_index);

    network_frame_value = htons(frame_count);
    (void)memcpy(&frame[offset], &network_frame_value, sizeof(network_frame_value));
    offset += sizeof(network_frame_value);

    network_frame_value = htons(frame_number);
    (void)memcpy(&frame[offset], &network_frame_value, sizeof(network_frame_value));

    (void)memcpy(&frame[frame_size - sizeof(s_tcp_frame_tail)], s_tcp_frame_tail, sizeof(s_tcp_frame_tail));
}

static int send_tcp_block(int socket_fd, uint32_t block_index)
{
    uint32_t frame_index;
    uint16_t frame_count = (uint16_t)(s_tcp_bytes_per_send / s_tcp_chunk_size_bytes);

    for (frame_index = 0U; frame_index < frame_count; frame_index++)
    {
        build_tcp_frame(s_tcp_transmit_buffer,
                        s_tcp_chunk_size_bytes,
                        block_index,
                        frame_count,
                        (uint16_t)(frame_index + 1U));

        if (send_tcp_all(socket_fd, s_tcp_transmit_buffer, s_tcp_chunk_size_bytes) != 0)
        {
            return -1;
        }
    }

    return 0;
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

static void tcp_tx_task(void *param)
{
    TickType_t send_interval_ticks = pdMS_TO_TICKS(1000U) / s_tcp_sends_per_second;
    uint32_t block_index = 0U;

    (void)param;
    (void)memset(s_tcp_transmit_buffer, 0xA5, sizeof(s_tcp_transmit_buffer));

    for (;;)
    {
        uint32_t sends_completed = 0U;

        (void)xEventGroupSetBits(s_tcp_events, TCP_EVENT_TX_IDLE);
        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (tcp_connection_active())
        {
            TickType_t block_start_tick = xTaskGetTickCount();

            if (send_tcp_block(s_tcp_socket, block_index) != 0)
            {
                if (tcp_connection_active())
                {
                    PRINTF("[TCP] Data transmission failed, errno=%d\r\n", errno);
                    (void)xEventGroupSetBits(s_tcp_events, TCP_EVENT_IO_ERROR);
                }
                break;
            }

            block_index++;
            sends_completed++;
            if (sends_completed == s_tcp_sends_per_second)
            {
                PRINTF("[TCP] Sent %u blocks (%u bytes)\r\n",
                       (unsigned int)s_tcp_sends_per_second,
                       (unsigned int)(s_tcp_sends_per_second * s_tcp_bytes_per_send));
                sends_completed = 0U;
            }

            {
                TickType_t elapsed_ticks = xTaskGetTickCount() - block_start_tick;

                if (elapsed_ticks < send_interval_ticks)
                {
                    vTaskDelay(send_interval_ticks - elapsed_ticks);
                }
            }
        }
    }
}

static void tcp_rx_task(void *param)
{
    (void)param;
    for (;;)
    {
        (void)xEventGroupSetBits(s_tcp_events, TCP_EVENT_RX_IDLE);
        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (tcp_connection_active())
        {
            int received = recv(s_tcp_socket, s_tcp_receive_buffer, sizeof(s_tcp_receive_buffer), 0);

            if (received > 0)
            {
                handle_tcp_data(s_tcp_receive_buffer, (size_t)received);
            }
            else if (received == 0)
            {
                if (tcp_connection_active())
                {
                    PRINTF("[TCP] Server closed the connection\r\n");
                    (void)xEventGroupSetBits(s_tcp_events, TCP_EVENT_IO_ERROR);
                }
                break;
            }
            else if (errno != EWOULDBLOCK)
            {
                if (tcp_connection_active())
                {
                    PRINTF("[TCP] Connection lost, errno=%d\r\n", errno);
                    (void)xEventGroupSetBits(s_tcp_events, TCP_EVENT_IO_ERROR);
                }
                break;
            }
        }
    }
}

static void tcp_connection_manager_task(void *param)
{
    (void)param;

    for (;;)
    {
        (void)xEventGroupWaitBits(s_tcp_events, TCP_EVENT_WORKERS_IDLE, pdFALSE, pdTRUE, portMAX_DELAY);

        s_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_tcp_socket < 0)
        {
            PRINTF("[TCP] socket() failed, errno=%d; retrying\r\n", errno);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        if (configure_tcp_keepalive(s_tcp_socket) != 0)
        {
            close_socket(&s_tcp_socket);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        PRINTF("[TCP] Connecting to %s:%u\r\n", APP_TCP_SERVER_IP, (unsigned int)APP_TCP_SERVER_PORT);
        if (connect(s_tcp_socket, (struct sockaddr *)&s_tcp_server_address, sizeof(s_tcp_server_address)) != 0)
        {
            PRINTF("[TCP] Connect failed, errno=%d; retrying\r\n", errno);
            close_socket(&s_tcp_socket);
            vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
            continue;
        }

        PRINTF("[TCP] Connected to %s:%u (socket=%d)\r\n", APP_TCP_SERVER_IP,
               (unsigned int)APP_TCP_SERVER_PORT, s_tcp_socket);

        if (set_socket_timeout(s_tcp_socket, SO_RCVTIMEO, SOCKET_RECEIVE_TIMEOUT_MS) != 0)
        {
            PRINTF("[TCP] Failed to set receive timeout, errno=%d\r\n", errno);
        }
        if (set_socket_timeout(s_tcp_socket, SO_SNDTIMEO, s_tcp_send_timeout_ms) != 0)
        {
            PRINTF("[TCP] Failed to set send timeout, errno=%d\r\n", errno);
        }

        (void)xEventGroupClearBits(s_tcp_events, TCP_EVENT_IO_ERROR | TCP_EVENT_WORKERS_IDLE);
        (void)xEventGroupSetBits(s_tcp_events, TCP_EVENT_CONNECTED);
        (void)xTaskNotifyGive(s_tcp_tx_task);
        (void)xTaskNotifyGive(s_tcp_rx_task);
        (void)xEventGroupWaitBits(s_tcp_events, TCP_EVENT_IO_ERROR, pdFALSE, pdTRUE, portMAX_DELAY);
        (void)xEventGroupClearBits(s_tcp_events, TCP_EVENT_CONNECTED);
        (void)shutdown(s_tcp_socket, SHUT_RDWR);
        (void)xEventGroupWaitBits(s_tcp_events, TCP_EVENT_WORKERS_IDLE, pdFALSE, pdTRUE, portMAX_DELAY);
        close_socket(&s_tcp_socket);
        PRINTF("[TCP] Disconnected; reconnecting in %u ms\r\n", (unsigned int)SOCKET_RETRY_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(SOCKET_RETRY_DELAY_MS));
    }
}

static void udp_task(void *param)
{
    struct sockaddr_in local_address;
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

        if (set_socket_timeout(s_udp_socket, SO_RCVTIMEO, SOCKET_RECEIVE_TIMEOUT_MS) != 0)
        {
            PRINTF("[UDP] Failed to set receive timeout, errno=%d\r\n", errno);
        }

        PRINTF("[UDP] Listening on 0.0.0.0:%u; peer=%s:%u (socket=%d)\r\n",
               (unsigned int)APP_UDP_LOCAL_PORT, APP_UDP_PEER_IP, (unsigned int)APP_UDP_PEER_PORT, s_udp_socket);

        for (;;)
        {
            struct sockaddr_in source_address;
            socklen_t source_length = sizeof(source_address);
            int received = recvfrom(s_udp_socket, s_udp_receive_buffer, sizeof(s_udp_receive_buffer), 0,
                                    (struct sockaddr *)&source_address, &source_length);

            if (received > 0)
            {
                handle_udp_data(s_udp_receive_buffer, (size_t)received, &source_address);
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
    TaskHandle_t udp_task_handle = NULL;

    if (s_tcp_events != NULL)
    {
        return 0;
    }

    if (!tcp_send_config_valid())
    {
        return -1;
    }

    PRINTF("[TCP] Send config: %u sends/s, %u bytes/send, %u bytes/frame, %u frames/send, timeout=%u ms, retries=%u\r\n",
           (unsigned int)s_tcp_sends_per_second,
           (unsigned int)s_tcp_bytes_per_send,
           (unsigned int)s_tcp_chunk_size_bytes,
           (unsigned int)(s_tcp_bytes_per_send / s_tcp_chunk_size_bytes),
           (unsigned int)s_tcp_send_timeout_ms,
           (unsigned int)s_tcp_send_timeout_retries);

    if (init_ipv4_address(&s_tcp_server_address, APP_TCP_SERVER_IP, APP_TCP_SERVER_PORT) != 0)
    {
        PRINTF("[TCP] Invalid server address: %s\r\n", APP_TCP_SERVER_IP);
        return -1;
    }

    if (init_ipv4_address(&s_udp_peer_address, APP_UDP_PEER_IP, APP_UDP_PEER_PORT) != 0)
    {
        PRINTF("[UDP] Invalid peer address: %s\r\n", APP_UDP_PEER_IP);
        return -1;
    }

    s_tcp_events = xEventGroupCreate();
    if (s_tcp_events == NULL)
    {
        PRINTF("[TCP] Failed to create TCP event group\r\n");
        return -1;
    }

    PRINTF("[MEM] Before socket tasks: free=%u min=%u\r\n", (unsigned int)xPortGetFreeHeapSize(),
           (unsigned int)xPortGetMinimumEverFreeHeapSize());

    if (xTaskCreate(tcp_tx_task, "tcp_tx", TCP_TX_TASK_STACK_DEPTH, NULL, SOCKET_TASK_PRIORITY,
                    &s_tcp_tx_task) != pdPASS)
    {
        PRINTF("[ERROR] Failed to create TCP transmit task\r\n");
        goto failed;
    }

    if (xTaskCreate(tcp_rx_task, "tcp_rx", TCP_RX_TASK_STACK_DEPTH, NULL, SOCKET_TASK_PRIORITY,
                    &s_tcp_rx_task) != pdPASS)
    {
        PRINTF("[ERROR] Failed to create TCP receive task\r\n");
        goto failed;
    }

    if (xTaskCreate(udp_task, "udp", UDP_TASK_STACK_DEPTH, NULL, SOCKET_TASK_PRIORITY, &udp_task_handle) != pdPASS)
    {
        PRINTF("[ERROR] Failed to create UDP task\r\n");
        goto failed;
    }

    if (xTaskCreate(tcp_connection_manager_task, "tcp_manager", TCP_MANAGER_TASK_STACK_DEPTH, NULL,
                    SOCKET_TASK_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("[ERROR] Failed to create TCP manager task\r\n");
        goto failed;
    }

    PRINTF("[MEM] After socket tasks: free=%u min=%u\r\n", (unsigned int)xPortGetFreeHeapSize(),
           (unsigned int)xPortGetMinimumEverFreeHeapSize());
    return 0;

failed:
    PRINTF("[MEM] Socket task creation failed: free=%u min=%u\r\n", (unsigned int)xPortGetFreeHeapSize(),
           (unsigned int)xPortGetMinimumEverFreeHeapSize());
    if (udp_task_handle != NULL)
    {
        vTaskDelete(udp_task_handle);
        close_socket(&s_udp_socket);
    }
    if (s_tcp_rx_task != NULL)
    {
        vTaskDelete(s_tcp_rx_task);
        s_tcp_rx_task = NULL;
    }
    if (s_tcp_tx_task != NULL)
    {
        vTaskDelete(s_tcp_tx_task);
        s_tcp_tx_task = NULL;
    }
    vEventGroupDelete(s_tcp_events);
    s_tcp_events = NULL;
    return -1;
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
