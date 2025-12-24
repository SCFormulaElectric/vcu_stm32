#ifndef LIGHT_CONTROLLER_TASK_H
#define LIGHT_CONTROLLER_TASK_H
#include "task.h"
void light_controller_task(void *argument);


TaskHandle_t create_light_controller_task(void);

#endif /* LIGHT_CONTROLLER_TASK_H */

