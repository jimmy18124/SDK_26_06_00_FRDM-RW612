/*
 * Copyright 2022, 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCKET_TASK_H
#define SOCKET_TASK_H

#include <stddef.h>

int socket_tasks_start(void);
int socket_udp_send(const void *data, size_t length);

#endif /* SOCKET_TASK_H */
