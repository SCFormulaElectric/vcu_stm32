/*
There are two main variables defined here, one is WATCHDOG_ALL_TASK which is the for every task
The other one is WD_{TASK_NAME} where the task_name is defined by the parameters passed into the "X" function
ex. WD_THROTTLE would be used to kick the bit for the throttle task
*/

#define WATCHDOG_TASK_LIST \
    X(THROTTLE) \
    X(BPPS) \
    X(MOTOR_CONTROLLER) \
    X(STATE_MACHINE) \
    X(IDWG) \
    X(CAN_RX) \
    X(CAN_TX) \
    X(TELEMETRY) \
    X(CLI_INPUT) \
    X(SD_CARD) \
    X(COOLING) \
    X(DASH) \
    X(DEFAULT) \
    X(LIGHT_CONTROLLER)

#define X(name) WD_##name = (1U << __COUNTER__),
enum {
    WATCHDOG_TASK_LIST
};
#undef X

#define X(name) | WD_##name
#define WD_ALL_TASKS (0 WATCHDOG_TASK_LIST)
#undef X