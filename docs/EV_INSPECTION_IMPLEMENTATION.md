# 2026 FSAE EV inspection implementation

This document records what the VCU firmware can support and what must still be demonstrated with vehicle hardware. It is not a substitute for the official rulebook, the team's ESF, or an event rules inquiry.

Target MCU: **STM32F407VET6** in the LQFP-100 package. The firmware uses the device's 168 MHz clock configuration, 512 KB flash, 128 KB main SRAM plus 64 KB CCM, ADC1 on PC0-PC5, CAN1 on PD0/PD1, SPI1 on PA5-PA7, and USB OTG FS on PA11/PA12.

## Official references

- [2026 FSAE Rules](https://www.fsaeonline.com/appcontent/58d21d53-c87f-4a49-8d22-73cf1faab9b2.pdf)
- [2026 EV Technical Inspection Handbook](https://www.fsaeonline.com/Page.aspx?pageid=f10c480f-d10c-49d2-9c5a-20ac15c1010f)
- [Tractive System Status Indicator FAQ](https://www.fsaeonline.com/cdsweb/rqa/ViewFAQ.aspx?faqnum=369)
- [Ready-to-Move Indicator FAQ](https://www.fsaeonline.com/cdsweb/rqa/ViewFAQ.aspx?faqnum=370)

## Firmware implemented in this revision

- The VCU starts in `CAR_IDLE` and initializes the brake light and fault indicator to safe states.
- Ready-to-Drive requires TSMS, BMS, APPS, BPPS, and a recent inverter fault-status CAN frame to be healthy.
- A deliberate brake-held/R2D request enters `CAR_PREPARE`; the state remains there for the configured RTD sound interval before `CAR_ENABLE`.
- Any loss of those preconditions returns to `CAR_IDLE` and the motor controller receives a periodic free-roll/zero-torque command.
- Inverter fault frames are length-checked and timestamped. Missing or stale frames inhibit torque.
- APPS and brake ADC channels are bounded before conversion; normalized pedal values are clamped to 0..1000.
- BPPS overlap is qualified for one second and remains faulted until throttle is released below the reset threshold.
- CAN is initialized for 500 kbit/s with FIFO0 reception, filtering, and bus-off notification.
- The serial interface includes `status`, `inspection`, `faults`, and `sensors` commands. It refuses to stop the safety tasks.

## Hardware/vehicle demonstrations still required

The following cannot be certified by this firmware alone:

1. The shutdown circuit must physically remove AIR control power when required by the rules. Verify the actual series path, de-energized-open behavior, interlocks, AMS, IMD, BSPD, inertia switch, shutdown buttons, and master switches.
2. BSPD must be an independent, rule-compliant hardware safety function. The VCU's APPS/BPPS diagnostics do not replace the BSPD.
3. The RTD sound must be produced by the approved buzzer/dashboard path. `rtd_sound_active` is the firmware request signal; the dashboard driver must consume it and the vehicle test must verify the audible timing and sound.
4. The tractive-system status indicator requires the actual approved separate red and green lights and their wiring. This project currently exposes only one `HOOP_LIGHT_PIN`; do not claim compliance until the hardware pin map is corrected and the lights are tested from the required viewing directions.
5. Confirm all GPIO polarity and pull resistors against the schematic. An open TSMS/BMS wire must produce the safe state; an internal pull-up must not make a broken wire appear healthy.
6. Run the EV Active inspection demonstrations with driven wheels removed and the required tools. Record the test result, reset behavior, CAN loss behavior, and time from each fault to zero torque and physical tractive-system shutdown.

## Required pre-drive test sequence

With the driven wheels removed and the tractive system handled under the team's high-voltage procedure:

1. Connect the laptop and run `inspection` and `status`.
2. Verify the VCU reports torque inhibited until a valid inverter status frame is present.
3. Verify each APPS channel open/short/out-of-range case inhibits torque.
4. Verify APPS/brake overlap produces a BPPS fault after the qualification interval.
5. Open TSMS, BMS, inertia, IMD, and BSPD paths one at a time and verify both the VCU indication and the physical shutdown path.
6. Verify removing BSPD power produces the required shutdown behavior; do not use a software command to simulate or bypass it.
7. Verify the RTD sound and indicators with TSMS/BMS healthy, brake applied, and a deliberate R2D request.
8. Disconnect CAN and verify the stale-frame interlock prevents or removes torque.
9. Remove/eject the SD card and disconnect USB while logging; verify the VCU remains safe and control timing is unaffected.
