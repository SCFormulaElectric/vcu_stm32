#include "Tasks/DAQ/Logging/cli_input_task.h"
#include "Tasks/Critical/throttle_task.h"

// Task: CLI Input
cli_output_entry_t cli_output[NUM_PERIODIC_OUTPUTS] = {0};

static void populate_array(void);
static void print_toggled_outputs(void);
static void start_stop_task(app_data_t *app, const char *task_name, int value);
static void print_help(app_data_t *app, const char *topic);
static void print_tasks(app_data_t *app);
static void print_io(void);
static void print_can(const app_data_t *data);
static void print_storage(const app_data_t *data);
static void print_reset_cause(void);
static void print_health(const app_data_t *data);
static void print_self_test(const app_data_t *data);
static void print_pedal(const app_data_t *data);
static void print_pedal_preview(const app_data_t *data);
static void print_config(const app_data_t *data);
static uint8_t config_is_valid(const app_data_t *data);
static uint8_t config_change_allowed(const app_data_t *data);
static uint8_t service_mode_active(void);

static uint8_t cli_service_mode = 0U;
static TickType_t cli_service_deadline = 0;

static const char *car_state_name(car_state_t state) {
    switch (state) {
        case CAR_IDLE: return "IDLE";
        case CAR_PREPARE: return "PREPARE";
        case CAR_ENABLE: return "ENABLE";
        default: return "UNKNOWN";
    }
}

static void print_status(const app_data_t *data) {
    serial_log("STATUS state=%s rtd=%u rtd_sound=%u faults=0x%08lX apps=%u bpps=%u inverter_fresh=%u throttle=%u pedal_command=%u brake=%u",
        car_state_name(data->car_state), (unsigned)data->ready_to_drive,
        (unsigned)data->rtd_sound_active, (unsigned long)data->safety_faults,
        (unsigned)data->motorControl.input_faults.apps_fault,
        (unsigned)data->motorControl.input_faults.bpps_fault,
        (unsigned)data->motorControl.fault_codes_valid,
        (unsigned)data->throttle_level,
        (unsigned)data->motorControl.torqueCommand,
        (unsigned)data->brake_level);
}

static void print_inspection_status(const app_data_t *data) {
    const uint8_t tsms = (HAL_GPIO_ReadPin(GPIOA, TSMS_PIN) == GPIO_PIN_SET);
    const uint8_t bms = (HAL_GPIO_ReadPin(GPIOA, BMS_PIN) == GPIO_PIN_SET);
    serial_log("INSPECTION torque_permitted=%u", (data->car_state == CAR_ENABLE &&
        data->safety_faults == SAFETY_FAULT_NONE));
    serial_log("INSPECTION tsms=%u bms=%u apps_fault=%u bpps_fault=%u inverter_fault=%u",
        tsms, bms, data->motorControl.input_faults.apps_fault,
        data->motorControl.input_faults.bpps_fault,
        is_fault(&data->motorControl.fault_codes));
    serial_log("INSPECTION note=verify physical shutdown circuit, BSPD, IMD, AMS, inertia switch, and indicators at vehicle");
}

static void print_io(void) {
    serial_log("IO tsms=%u r2d=%u bms=%u brake_light=%u fault_indicator=%u",
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, TSMS_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, R2D_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, BMS_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, BRAKE_LIGHT_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, HOOP_LIGHT_PIN) == GPIO_PIN_SET));
    serial_log("ADC throttle1=%u throttle2=%u brake1=%u brake2=%u thermistor1=%u thermistor2=%u",
        adc_buffer[0], adc_buffer[1], adc_buffer[2], adc_buffer[3], adc_buffer[4], adc_buffer[5]);
}

static void print_can(const app_data_t *data) {
    serial_log("CAN bitrate=500000 rx_pending=%u rx_dropped=%lu tx_pending=%u tx_errors=%lu bus_off=%lu",
        (unsigned)uxQueueMessagesWaiting(data->can_bus.can_rx_queue),
        (unsigned long)data->can_bus.rx_dropped,
        (unsigned)uxQueueMessagesWaiting(data->can_bus.can_tx_queue),
        (unsigned long)data->can_bus.tx_errors,
        (unsigned long)data->can_bus.bus_off_count);
}

static void print_storage(const app_data_t *data) {
    const char *owner = (sd_card_owner == MCU_SD_CARD) ? "MCU" : "USB";
    serial_log("STORAGE owner=%s file_open=%u log_number=%lu queue_pending=%u",
        owner, (unsigned)data->sd_card.file_opened,
        (unsigned long)data->sd_card.log_number,
        (unsigned)uxQueueMessagesWaiting(data->sd_card.sd_card_q));
}

static void print_reset_cause(void) {
    serial_log("RESET iwdg=%u software=%u pin=%u brownout=%u power=%u low_power=%u",
        (unsigned)(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET),
        (unsigned)(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET),
        (unsigned)(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET),
        (unsigned)(__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) != RESET),
        (unsigned)(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET),
        (unsigned)(__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET));
}

static void print_health(const app_data_t *data) {
    const uint8_t tsms = (HAL_GPIO_ReadPin(GPIOA, TSMS_PIN) == GPIO_PIN_SET);
    const uint8_t bms = (HAL_GPIO_ReadPin(GPIOA, BMS_PIN) == GPIO_PIN_SET);
    const uint8_t r2d = (HAL_GPIO_ReadPin(GPIOA, R2D_PIN) == GPIO_PIN_SET);
    const TickType_t now = xTaskGetTickCount();
    const uint8_t inverter_fresh = (data->motorControl.fault_codes_valid != 0U) &&
        ((now - data->motorControl.fault_codes_last_tick) <= pdMS_TO_TICKS(250));
    const uint8_t healthy = (data->safety_faults == SAFETY_FAULT_NONE) &&
        (tsms != 0U) && (bms != 0U) && (r2d != 0U) &&
        (data->motorControl.input_faults.apps_fault == 0U) &&
        (data->motorControl.input_faults.bpps_fault == 0U) &&
        (inverter_fresh != 0U) && !is_fault(&data->motorControl.fault_codes);

    serial_log("HEALTH result=%s state=%s tsms=%u bms=%u r2d=%u apps=%u bpps=%u inverter_fresh=%u faults=0x%08lX",
        healthy ? "PASS" : "WARN", car_state_name(data->car_state), tsms, bms, r2d,
        (unsigned)data->motorControl.input_faults.apps_fault,
        (unsigned)data->motorControl.input_faults.bpps_fault,
        inverter_fresh, (unsigned long)data->safety_faults);
}

static void print_self_test(const app_data_t *data) {
    uint8_t tasks_ok = 1U;
    for (size_t i = 0; i < NUM_TASKS; i++) {
        if (data->task_entries[i].handle == NULL) {
            tasks_ok = 0U;
            break;
        }
    }

    const uint8_t queues_ok = (data->cli_queue != NULL) &&
        (data->can_bus.can_rx_queue != NULL) &&
        (data->can_bus.can_tx_queue != NULL) &&
        (data->sd_card.sd_card_q != NULL);
    uint8_t adc_ok = 1U;
    for (size_t i = 0; i < ADC_CHANNEL_COUNT; i++) {
        if (adc_buffer[i] > 4095U) {
            adc_ok = 0U;
            break;
        }
    }
    const uint8_t config_ok = config_is_valid(data);
    const uint8_t pass = tasks_ok && queues_ok && adc_ok && config_ok;

    serial_log("SELFTEST result=%s tasks=%u queues=%u adc=%u config=%u",
        pass ? "PASS" : "FAIL", tasks_ok, queues_ok, adc_ok, config_ok);
    serial_log("SELFTEST note=software checks only; physical shutdown and HV tests remain required");
}

static void print_pedal(const app_data_t *data) {
    const uint16_t raw = data->throttle_level;
    const uint16_t shaped = pedal_response_apply(&data->pedal_response, raw);
    serial_log("PEDAL mode=%s strength=%u percent=%u raw=%u shaped=%u torque_x10=%u",
        pedal_response_mode_name(data->pedal_response.mode),
        (unsigned)data->pedal_response.strength,
        (unsigned)(data->pedal_response.strength / 10U),
        (unsigned)raw, (unsigned)shaped,
        (unsigned)data->motorControl.torqueCommand);
}

static void print_pedal_preview(const app_data_t *data) {
    serial_log("PEDAL_PREVIEW mode=%s strength_percent=%u",
        pedal_response_mode_name(data->pedal_response.mode),
        (unsigned)(data->pedal_response.strength / 10U));
    for (uint16_t input = 0U; input <= 1000U; input = (uint16_t)(input + 100U)) {
        const uint16_t output = pedal_response_apply(&data->pedal_response, input);
        serial_log("PEDAL_PREVIEW input=%u output=%u", (unsigned)input, (unsigned)output);
    }
}

static void print_config(const app_data_t *data) {
    serial_log("CONFIG target=%s firmware=%s", VCU_TARGET_MCU, VCU_FIRMWARE_VERSION);
    serial_log("CONFIG pedal_mode=%s pedal_strength=%u percent=%u",
        pedal_response_mode_name(data->pedal_response.mode),
        (unsigned)data->pedal_response.strength,
        (unsigned)(data->pedal_response.strength / 10U));
    serial_log("CONFIG persistence=runtime_only; reset restores defaults");
}

static uint8_t config_is_valid(const app_data_t *data) {
    return (data->pedal_response.mode <= PEDAL_RESPONSE_PROGRESSIVE) &&
        (data->pedal_response.strength <= 1000U);
}

static uint8_t config_change_allowed(const app_data_t *data) {
    if (data->car_state != CAR_IDLE || data->ready_to_drive != 0U) {
        serial_log("Refusing pedal configuration change: vehicle is not IDLE");
        return 0U;
    }
    return 1U;
}

static uint8_t service_mode_active(void) {
    if (cli_service_mode == 0U) {
        return 0U;
    }
    if ((int32_t)(xTaskGetTickCount() - cli_service_deadline) >= 0) {
        cli_service_mode = 0U;
        serial_log("SERVICE mode expired");
        return 0U;
    }
    return 1U;
}

void cli_input_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    char cli_buffer[CLI_BUFFER_SIZE] = {0};
    uint8_t index = 0;

    populate_array();

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        (void)service_mode_active();
        uint8_t ch;
        while(xQueueReceive(data->cli_queue, &ch, CLI_TICKS_TO_WAIT) == pdTRUE){
            if (ch == '\b' || ch == 0x7fU) {
                if (index > 0U) {
                    index--;
                }
                continue;
            }
            if (ch == '\n' || ch == '\r') {
                cli_buffer[index] = '\0';
                process_cmd(data, cli_buffer);
                index = 0;
                break;
            } else {
                if (index < (CLI_BUFFER_SIZE - 1U)) {
                    cli_buffer[index++] = (char)ch;
                } else {
                    serial_log("Command too long! Please refresh the buffer");
                    index = 0;
                }
            }
        }

        print_toggled_outputs();
        xEventGroupSetBits(data->idwg_group, WD_CLI_INPUT);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(CLI_TASK_DELAY_MS));
    }
}

void populate_array() {
    cli_output[IO_output_index].flag = 0;
    cli_output[IO_output_index].last_tick_time = 0;
}

void print_toggled_outputs() {
    TickType_t now = xTaskGetTickCount();
    if (cli_output[IO_output_index].flag && (now - cli_output[IO_output_index].last_tick_time) >= pdMS_TO_TICKS(PRINT_IO_PERIOD_MS)) {
        cli_output[IO_output_index].last_tick_time = now;

        GPIO_PinState dpin1 = HAL_GPIO_ReadPin(GPIOA, TSMS_PIN);
        GPIO_PinState dpin2 = HAL_GPIO_ReadPin(GPIOA, R2D_PIN);
        GPIO_PinState dpin3 = HAL_GPIO_ReadPin(GPIOA, BMS_PIN);
        uint16_t a_value1 = adc_buffer[0];
        uint16_t a_value2 = adc_buffer[1];
        uint16_t a_value3 = adc_buffer[2];
        uint16_t a_value4 = adc_buffer[3];
        uint16_t a_value5 = adc_buffer[4];
        uint16_t a_value6 = adc_buffer[5];
        serial_log("ADC1: %u ADC2: %u ADC3: %u ADC4: %u ADC5: %u ADC6: %u",
                    a_value1, a_value2, a_value3, a_value4, a_value5, a_value6);
        serial_log("DIN tsms=%d r2d=%d bms=%d", dpin1, dpin2, dpin3);
    }
}

void process_cmd(app_data_t *app, const char *cmd) {
    char task_name[32];
    int value = -1;

    if (strcmp(cmd, "h") == 0 || strcmp(cmd, "H") == 0 ||
        strcmp(cmd, "?") == 0 || strcmp(cmd, "help") == 0) {
        print_help(app, NULL);
        return;
    }

    if (strncmp(cmd, "help ", 5) == 0) {
        print_help(app, cmd + 5);
        return;
    }

    if (strcmp(cmd, "version") == 0) {
        serial_log("VERSION firmware=%s target=%s", VCU_FIRMWARE_VERSION, VCU_TARGET_MCU);
        return;
    }

    if (strcmp(cmd, "status") == 0) {
        print_status(app);
        return;
    }

    if (strcmp(cmd, "health") == 0) {
        print_health(app);
        return;
    }

    if (strcmp(cmd, "self-test") == 0 || strcmp(cmd, "selftest") == 0) {
        print_self_test(app);
        return;
    }

    if (strcmp(cmd, "service status") == 0) {
        if (service_mode_active()) {
            const uint32_t remaining_ms = (uint32_t)(
                (cli_service_deadline - xTaskGetTickCount()) * portTICK_PERIOD_MS);
            serial_log("SERVICE active=1 remaining_ms=%lu", (unsigned long)remaining_ms);
        } else {
            serial_log("SERVICE active=0");
        }
        return;
    }

    if (strcmp(cmd, "service begin") == 0) {
        if (app->car_state != CAR_IDLE || app->ready_to_drive != 0U) {
            serial_log("Refusing service mode: vehicle is not IDLE");
            return;
        }
        cli_service_mode = 1U;
        cli_service_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(SERVICE_MODE_TIMEOUT_MS);
        serial_log("SERVICE mode active; timeout_ms=%u", SERVICE_MODE_TIMEOUT_MS);
        return;
    }

    if (strcmp(cmd, "service end") == 0) {
        cli_service_mode = 0U;
        serial_log("SERVICE mode inactive");
        return;
    }

    if (strcmp(cmd, "inspection") == 0 || strcmp(cmd, "inspect") == 0) {
        print_inspection_status(app);
        return;
    }

    if (strcmp(cmd, "faults") == 0) {
        serial_log("FAULTS active=0x%08lX apps=%u bpps=%u inverter=%u",
            (unsigned long)app->safety_faults,
            app->motorControl.input_faults.apps_fault,
            app->motorControl.input_faults.bpps_fault,
            is_fault(&app->motorControl.fault_codes));
        return;
    }

    if (strcmp(cmd, "io") == 0) {
        print_io();
        return;
    }

    if (strcmp(cmd, "can") == 0) {
        print_can(app);
        return;
    }

    if (strcmp(cmd, "storage") == 0) {
        print_storage(app);
        return;
    }

    if (strcmp(cmd, "reset-cause") == 0) {
        print_reset_cause();
        return;
    }

    if (strcmp(cmd, "tasks") == 0) {
        print_tasks(app);
        return;
    }

    if (strcmp(cmd, "pedal") == 0 || strcmp(cmd, "pedal show") == 0) {
        print_pedal(app);
        return;
    }

    if (strcmp(cmd, "pedal preview") == 0) {
        print_pedal_preview(app);
        return;
    }

    if (strncmp(cmd, "pedal mode ", 11) == 0) {
        if (!config_change_allowed(app)) {
            return;
        }
        const char *mode = cmd + 11;
        if (strcmp(mode, "linear") == 0) {
            app->pedal_response.mode = PEDAL_RESPONSE_LINEAR;
        } else if (strcmp(mode, "early") == 0) {
            app->pedal_response.mode = PEDAL_RESPONSE_EARLY;
        } else if (strcmp(mode, "balanced") == 0) {
            app->pedal_response.mode = PEDAL_RESPONSE_BALANCED;
        } else if (strcmp(mode, "progressive") == 0) {
            app->pedal_response.mode = PEDAL_RESPONSE_PROGRESSIVE;
        } else {
            serial_log("Invalid pedal mode: %s; use linear, early, balanced, or progressive", mode);
            return;
        }
        serial_log("PEDAL mode=%s", pedal_response_mode_name(app->pedal_response.mode));
        return;
    }

    unsigned int strength_percent = 0U;
    if (sscanf(cmd, "pedal strength %u", &strength_percent) == 1) {
        if (!config_change_allowed(app)) {
            return;
        }
        if (strength_percent > 100U) {
            serial_log("Invalid pedal strength %u; use 0 to 100", strength_percent);
            return;
        }
        app->pedal_response.strength = (uint16_t)(strength_percent * 10U);
        serial_log("PEDAL strength=%u percent=%u", (unsigned)app->pedal_response.strength,
            strength_percent);
        return;
    }

    if (strcmp(cmd, "config") == 0 || strcmp(cmd, "config show") == 0) {
        print_config(app);
        return;
    }

    if (strcmp(cmd, "config validate") == 0) {
        serial_log("CONFIG result=%s", config_is_valid(app) ? "PASS" : "FAIL");
        return;
    }

    if (strcmp(cmd, "reboot") == 0) {
        serial_log("REBOOT requires 'reboot confirm' and an IDLE vehicle state");
        return;
    }

    if (strcmp(cmd, "reboot confirm") == 0) {
        if (app->car_state != CAR_IDLE || app->ready_to_drive != 0U) {
            serial_log("Refusing reboot: vehicle is not IDLE");
            return;
        }
        serial_log("REBOOT resetting VCU");
        vTaskDelay(pdMS_TO_TICKS(100));
        NVIC_SystemReset();
        return;
    }

    if (strcmp(cmd, "sensors") == 0) {
        print_io();
        return;
    }

    if (strcmp(cmd, "watch sensors") == 0) {
        cli_output[IO_output_index].flag = 1U;
        cli_output[IO_output_index].last_tick_time = 0;
        serial_log("WATCH sensors enabled; interval_ms=%u", PRINT_IO_PERIOD_MS);
        return;
    }

    if (strcmp(cmd, "watch off") == 0 || strcmp(cmd, "t") == 0 || strcmp(cmd, "T") == 0) {
        cli_output[IO_output_index].flag = 0U;
        serial_log("WATCH disabled");
        return;
    }

    if (sscanf(cmd, "%31[^=]=%d", task_name, &value) == 2) {
        start_stop_task(app, task_name, value);
        return;
    }
    
    serial_log("Invalid command format: %s\n", cmd);
}

static void start_stop_task(app_data_t *app, const char* task_name, int value) {
    if (!service_mode_active()) {
        serial_log("Task control refused: use 'service begin' while vehicle is IDLE");
        return;
    }
    if (app->car_state != CAR_IDLE || app->ready_to_drive != 0U) {
        serial_log("Task control refused: vehicle is not IDLE");
        return;
    }
    if (value == 0 && (strcmp(task_name, "throttle") == 0 ||
        strcmp(task_name, "bpps") == 0 || strcmp(task_name, "motor_controller") == 0 ||
        strcmp(task_name, "state_machine") == 0 || strcmp(task_name, "idwg") == 0)) {
        serial_log("Refusing to stop safety task: %s", task_name);
        return;
    }
    for (size_t i = 0; i < NUM_TASKS; i++) {
        if (strcmp(task_name, app->task_entries[i].name) == 0) {
            TaskHandle_t handle = app->task_entries[i].handle;
            if (value == 0) {
                vTaskSuspend(handle);
                serial_log("%s suspended\n", task_name);
            } else if (value == 1) {
                vTaskResume(handle);
                serial_log("%s resumed\n", task_name);
            } else {
                serial_log("Invalid value %d for %s, use 0 or 1\n", value, task_name);
            }
            return;
        }
    }
    serial_log("Unknown task: %s", task_name);
}

static void print_tasks(app_data_t *app) {
    for (size_t i = 0; i < NUM_TASKS; i++) {
        eTaskState state = eTaskGetState(app->task_entries[i].handle);
        const char *state_str;
        switch (state) {
            case eRunning:   state_str = "Running"; break;
            case eReady:     state_str = "Ready"; break;
            case eBlocked:   state_str = "Blocked"; break;
            case eSuspended: state_str = "Suspended"; break;
            case eDeleted:   state_str = "Deleted"; break;
            default:         state_str = "Unknown"; break;
        }
        serial_log("%s: %s\r\n", app->task_entries[i].name, state_str);
    }

}

static void print_help(app_data_t *app, const char *topic) {
    if (topic == NULL || topic[0] == '\0') {
        serial_log("COMMANDS status health self-test inspection faults io sensors can storage reset-cause version tasks pedal config service reboot watch help");
        serial_log("READ-ONLY status health self-test inspection faults io sensors can storage reset-cause version tasks pedal config");
        serial_log("PEDAL pedal show | pedal preview | pedal mode linear|early|balanced|progressive | pedal strength 0..100");
        serial_log("CONFIG config show | config validate; settings are runtime-only");
        serial_log("SERVICE service begin|status|end; task control requires active service mode");
        serial_log("REBOOT reboot confirm; only allowed while vehicle is IDLE");
        serial_log("WATCH watch sensors | watch off");
        serial_log("HELP help <command>");
        serial_log("SERVICE <task_name>=1; safety tasks cannot be stopped");
        return;
    }
    if (strcmp(topic, "status") == 0) {
        serial_log("status: show state, RTD status, pedal levels, faults, and inverter freshness");
    } else if (strcmp(topic, "health") == 0) {
        serial_log("health: give a one-line software health result and safety-input summary");
    } else if (strcmp(topic, "self-test") == 0) {
        serial_log("self-test: check task handles, queues, ADC range, and runtime configuration");
    } else if (strcmp(topic, "inspection") == 0) {
        serial_log("inspection: show safety inputs and whether the VCU currently permits torque");
    } else if (strcmp(topic, "faults") == 0) {
        serial_log("faults: show active safety and inverter fault status; faults cannot be cleared from the CLI");
    } else if (strcmp(topic, "io") == 0) {
        serial_log("io: show TSMS, R2D, BMS, outputs, and raw ADC values");
    } else if (strcmp(topic, "can") == 0) {
        serial_log("can: show bitrate, queue depth, dropped RX frames, TX errors, and bus-off count");
    } else if (strcmp(topic, "storage") == 0) {
        serial_log("storage: show SD ownership, active log, and pending log records");
    } else if (strcmp(topic, "reset-cause") == 0) {
        serial_log("reset-cause: show hardware reset flags since the last reset");
    } else if (strcmp(topic, "version") == 0) {
        serial_log("version: show firmware identifier and target MCU");
    } else if (strcmp(topic, "pedal") == 0) {
        serial_log("pedal show: display raw and shaped pedal values");
        serial_log("pedal preview: display the 0..1000 response table");
        serial_log("pedal mode linear|early|balanced|progressive: select response table");
        serial_log("pedal strength 0..100: blend selected response with linear");
    } else if (strcmp(topic, "config") == 0) {
        serial_log("config show: display runtime configuration; config validate: check ranges");
    } else if (strcmp(topic, "reboot") == 0) {
        serial_log("reboot confirm: reset the VCU only when the vehicle is IDLE");
    } else if (strcmp(topic, "service") == 0) {
        serial_log("service begin: enable task control for five minutes while IDLE");
        serial_log("service status: show service mode state; service end: disable it");
    } else if (strcmp(topic, "tasks") == 0) {
        serial_log("tasks: list task states; use task control only in a controlled service mode");
        print_tasks(app);
    } else if (strcmp(topic, "watch") == 0) {
        serial_log("watch sensors: print raw inputs periodically; watch off: stop periodic output");
    } else {
        serial_log("No help available for: %s", topic);
    }
}


task_entry_t create_cli_input_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        cli_input_task,            
        "CLI Input",               // Task name (string)
        CLI_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        CLI_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "cli";
    return entry;
}
