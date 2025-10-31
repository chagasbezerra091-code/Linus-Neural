// SPDX-License-Identifier: Apache-2.0
#ifndef SAFETY_CORE_H
#define SAFETY_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#define SAFETY_OK 0
#define SAFETY_WARN 1
#define SAFETY_ALERT 2

typedef struct {
    int level;
    const char *message;
} SafetyEvent;

void safety_init(void);
void safety_scan_processes(void);
void safety_check_permissions(void);
void safety_scan_integrity(void);
void safety_log_event(SafetyEvent e);
void safety_shutdown(void);

#endif
