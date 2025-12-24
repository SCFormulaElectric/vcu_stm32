#ifndef can_transmitter_TASK_H
#define can_transmitter_TASK_H

#include "app.h"
#include "can_bus.h"


void can_transmitter_task(void *argument);


task_entry_t create_can_transmitter_task(void);

#endif /* can_transmitter_TASK_H */

