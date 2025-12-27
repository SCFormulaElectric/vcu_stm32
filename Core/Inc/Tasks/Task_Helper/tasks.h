#ifndef TASKS_H
#define TASKS_H

#include "Tasks/Critical/throttle_task.h"
#include "Tasks/Critical/brake_pedal_plausibility_check_task.h"
#include "Tasks/Critical/motor_controller_task.h"
#include "Tasks/Critical/state_machine_task.h"
#include "Tasks/Critical/independent_watchdog_task.h"
#include "Tasks/CAN/can_receiver_task.h"
#include "Tasks/CAN/can_transmitter_task.h"
#include "Tasks/DAQ/telemetry_task.h"
#include "Tasks/DAQ/Logging/cli_input_task.h"
#include "Tasks/DAQ/Logging/sd_card_task.h"
#include "Tasks/Misc/cooling_task.h"
#include "Tasks/Misc/dash_task.h"
#include "Tasks/Misc/default_task_task.h"
#include "Tasks/Misc/light_controller_task.h"

#endif /* TASKS_H */

