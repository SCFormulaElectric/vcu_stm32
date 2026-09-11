#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H
/* These positions mirror the task_entry_t order in handles.h. */
enum {
    WD_THROTTLE = (1U << 0),
    WD_BPPS = (1U << 1),
    WD_CAN_RX = (1U << 2),
    WD_CAN_TX = (1U << 3),
    WD_CLI_INPUT = (1U << 4),
    WD_COOLING = (1U << 5),
    WD_DASH = (1U << 6),
    WD_DEFAULT = (1U << 7),
    WD_IDWG = (1U << 8),
    WD_LIGHT_CONTROLLER = (1U << 9),
    WD_MOTOR_CONTROLLER = (1U << 10),
    WD_SD_CARD = (1U << 11),
    WD_STATE_MACHINE = (1U << 12),
    WD_TELEMETRY = (1U << 13)
};

/* Availability supervision only: the IWDG is not a torque-safety mechanism.
 * Optional logging/UI/telemetry/cooling/light/default services are excluded. */
#define WD_REQUIRED_TASKS (WD_THROTTLE | WD_BPPS | WD_CAN_RX | WD_CAN_TX | \
    WD_MOTOR_CONTROLLER | WD_STATE_MACHINE)
#endif
