#ifndef COOLING_TASK_H
#define COOLING_TASK_H

#include "task.h"


void cooling_task(void *argument);


TaskHandle_t create_cooling_task(void);

#endif /* COOLING_TASK_H */

