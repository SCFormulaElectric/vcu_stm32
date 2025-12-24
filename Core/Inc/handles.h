#ifndef TASK_HANLDES_H
#define TASK_HANLDES_H
#include "app.h"

typedef struct {
    const char *name;
    TaskHandle_t *handle;
} cli_task_entry_t;

extern const cli_task_entry_t cli_tasks[];
extern const size_t CLI_TASK_COUNT;
#endif