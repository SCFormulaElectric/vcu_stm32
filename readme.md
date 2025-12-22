# University of Southern California Formula SAE - VCU Repository

This is our repository containing our **Vehicle Control Unit (VCU)** running on an STM32. The VCU interfaces with the different subcomponents of the car to ensure the car runs smoothly. The VCU is also responsible for logging data from our CAN bus and sensors to help with testing and validation. Our car currently runs on one can bus at 500KB.

---

## Components on our Car

- **Motor Controller**
- **Dashboard**
- **Brake Pressure Sensors**
- **Throttle**
- **LoRa Module** for telemetry
- **Pump**
- **Thermistors**
- **Orion Battery Management System**

---

## Code Structure

We are using FreeRTOS APIs to create different tasks for different components. The bulk of the code can be found under: Core/Src/Tasks/

Data is shared between different tasks via a global data structure *app_data_t*. This global variable is passed to each task, which updates it each time the task is scheduled. Specific implementation can be found in Core/Src/app.c

When tasks need to send CAN messages, they send them safely to the CAN bus queues, which are handled by a single task. Another task is responsible for processing incoming CAN messages.

---

**Formula Electric USC 2025–2026**