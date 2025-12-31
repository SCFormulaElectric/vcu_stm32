#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H
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
#define WD_ALL_TASKS ((1U << NUM_TASKS) - 1U)
#undef X
#endif
