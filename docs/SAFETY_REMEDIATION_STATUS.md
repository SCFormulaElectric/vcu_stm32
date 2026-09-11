# VCU safety-remediation status

Review date: 2026-08-28

This is a closure record for the current remediation branch. It is not a safety certification, an authorization to energize the tractive system, or a replacement for the team's electrical safety process.

## Verified in software

- APPS/brake overlap inhibits torque immediately and remains latched until APPS is below the reset threshold.
- Both brake-pressure channels are range-checked and normalized disagreement is persistence-qualified.
- ADC samples are timer-triggered, DMA-backed complete scans with coherent snapshots, freshness checks, and latched DMA/ADC error handling.
- The inverter command has a dedicated latest-value path initialized to a canonical zero-torque frame. Only the state machine can release the global inhibit; command publishers can assert it but cannot clear it. CAN RX overflow, malformed accepted traffic, and HAL receive failure latch the inhibit until reboot.
- CAN acceptance filters and software validation reject unexpected identifiers, extended frames, remote frames, and invalid lengths. Critical inverter status is handled ahead of noncritical traffic.
- Missing or stale inverter fault status inhibits torque.
- SD-card logging is optional, bounded, and degraded on media errors; it is excluded from the watchdog-required task set. USB mass-storage access to the same card is disabled.
- The independent watchdog starts before peripheral and RTOS initialization. Assert, allocation-failure, stack-overflow, and fatal startup paths converge on a reset/fault response.
- The default pedal curve is linear, positive torque request rise is limited to 100 N.m/s at the current 25 ms task period, and reductions remain immediate.
- Runtime CLI task suspend/resume is disabled.
- `VCU_TORQUE_ENABLE_COMMISSIONED` defaults to zero, adding a persistent commissioning fault and preventing entry to the torque-enabled state while the physical evidence below remains open.

## Automated evidence

- Six host test suites pass with GCC at `-O0` and `-O2` (12 executions total).
- Full STM32F407 firmware compiles and links in Debug and Release with project-owned `Core` sources treated as warnings-as-errors.
- Release image: 61,724 bytes flash (11.77 percent) and 84,984 bytes RAM (64.84 percent).
- Debug image: 98,668 bytes flash (18.82 percent) and 84,984 bytes RAM (64.84 percent).
- Remaining compiler warnings are limited to unchanged ST HAL, generated FATFS target, and upstream FatFs unused parameters.

## Blocking evidence still required

Do not describe this branch as vehicle-safe or use it to enable tractive torque until all items below are closed with recorded evidence:

1. Review the schematic and harness pinout for TSMS, BMS/AMS, shutdown-loop, AIR, precharge, discharge, and indicator signals. Confirm polarity and prove that every open wire reaches the safe state; current internal pull-ups must not conceal a broken wire.
2. Supply the exact Cascadia inverter configuration/manual contract: command identifier and byte layout, signedness/scaling, enable bits, authoritative fault-status identifiers and lengths, broadcast period, and command timeout. Confirm the firmware constants against that contract.
3. Implement and verify the Ready-to-Drive sound output. The present `rtd_sound_active` value has no verified physical consumer.
4. Demonstrate the independent hardware shutdown path, including AMS, IMD, BSPD, BOTS/inertia switch, shutdown buttons, AIR control power, precharge, discharge, and reset behavior. Software torque inhibition does not replace this path.
5. Calibrate both APPS and both brake sensors on the actual vehicle and approve the range, disagreement, overlap, and reset thresholds.
6. Run HIL and vehicle tests with driven wheels removed: startup/brownout, task stall, CAN saturation/bus-off/disconnect, stale and malformed frames, ADC DMA faults, open/short sensors, SD absent/full/corrupt/removal, USB disconnect, and every shutdown input. Measure time to zero commanded torque and physical tractive-system shutdown.
7. Verify the inverter's own command-timeout behavior and prove that loss of the VCU or CAN transceiver produces zero torque independently of software execution.

## Release gate

A release commit and GitHub push conditioned on “once it is safe” must wait for the blocking evidence above. After closure and independent review, enabling torque requires an explicit source change to `VCU_TORQUE_ENABLE_COMMISSIONED`; it is not exposed through the CLI. A clearly marked work-in-progress push is a different release decision and requires explicit authorization.
