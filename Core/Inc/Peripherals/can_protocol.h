#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include "Peripherals/bms_can_protocol.h"

/*
 * CAN protocol identifiers used by the VCU and Cascadia motor controller.
 * Keep protocol numbers here instead of duplicating literals in task code.
 */

#define CAN_BUS_BITRATE_BPS                 500000U

#define CAN_ID_MC_TEMPERATURE_1             0x0A0U
#define CAN_ID_MC_TEMPERATURE_2             0x0A1U
#define CAN_ID_MC_TEMPERATURE_3             0x0A2U
#define CAN_ID_MC_ANALOG_INPUTS             0x0A3U
#define CAN_ID_MC_DIGITAL_INPUTS            0x0A4U
#define CAN_ID_MC_POSITION                  0x0A5U
#define CAN_ID_MC_CURRENT                   0x0A6U
#define CAN_ID_MC_VOLTAGE                   0x0A7U
#define CAN_ID_MC_FLUX                      0x0A8U
#define CAN_ID_MC_INTERNAL_VOLTAGES         0x0A9U
#define CAN_ID_MC_INTERNAL_STATES           0x0AAU
#define CAN_ID_MC_FAULT_CODES               0x0ABU
#define CAN_ID_MC_TORQUE_TIMER              0x0ACU
#define CAN_ID_MC_MODULATION_FLUX           0x0ADU
#define CAN_ID_MC_FIRMWARE                  0x0AEU
#define CAN_ID_MC_DIAGNOSTIC                0x0AFU
#define CAN_ID_MC_HIGH_SPEED                0x0B0U
#define CAN_ID_MC_TORQUE_CAPABILITY         0x0B1U

#define CAN_ID_MC_COMMAND                   0x0C0U
#define CAN_ID_MC_PARAMETER_RW              0x0C1U
#define CAN_ID_MC_PARAMETER_RESPONSE        0x0C2U

#define CAN_ID_MC_BROADCAST_MIN             CAN_ID_MC_TEMPERATURE_1
/* Preserve the complete Cascadia broadcast window, including reserved IDs. */
#define CAN_ID_MC_BROADCAST_MAX             0x0BFU

/* Dashboard identifiers are team-defined and must not overlap the MC range. */
#define CAN_ID_DASHBOARD_MIN                0x180U
#define CAN_ID_DASHBOARD_MAX                0x18FU

#define CAN_DLC_MC_STATUS                   8U
#define CAN_DLC_MC_COMMAND                  8U
#define CAN_DLC_MC_PARAMETER_RW             6U
#define CAN_DLC_MC_PARAMETER_RESPONSE       6U
#define CAN_DLC_BMS                         BMS_CAN_FRAME_DLC

/* Provisional software freshness threshold. Release validation must confirm
 * this against the authoritative Cascadia 0xAB broadcast configuration. */
#define CAN_INVERTER_FAULT_STATUS_TIMEOUT_MS 250U

/* Cascadia fault bit definitions used by the VCU. */
#define CAN_MC_FAULT_COMMAND_MESSAGE_LOST   0x00000800U

#define CAN_ID_IS_MOTOR_CONTROLLER(id) \
    ((id) >= CAN_ID_MC_BROADCAST_MIN && (id) <= CAN_ID_MC_BROADCAST_MAX)

#define CAN_ID_IS_DASHBOARD(id) \
    ((id) >= CAN_ID_DASHBOARD_MIN && (id) <= CAN_ID_DASHBOARD_MAX)

#endif /* CAN_PROTOCOL_H */
