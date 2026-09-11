#include "Tasks/DAQ/Logging/cli_input_task.h"
#include "Tasks/Critical/throttle_task.h"
#include "Tasks/Task_Helper/handles.h"
#include <stdlib.h>

/* CLI replies remain available when background logging is intentionally off.
 * This function bypasses the SD/log-level routing and writes only to USB CDC. */
static void cli_serial_log(const char *fmt, ...)
{
    char message[LOG_MSG_MAX_LEN - 2U];
    char line[LOG_MSG_MAX_LEN];
    va_list args;

    va_start(args, fmt);
    (void)vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);
    (void)snprintf(line, sizeof(line), "%s\r\n", message);
    __serial_print(line);
}

#define serial_log cli_serial_log

// Task: CLI Input
cli_output_entry_t cli_output[NUM_PERIODIC_OUTPUTS] = {0};

static void populate_array(void);
static void print_toggled_outputs(void);
static void start_stop_task(app_data_t *app, const char *task_name, int value);
static void print_help(app_data_t *app, const char *topic);
static void print_tasks(app_data_t *app);
static void print_io(void);
static void print_can(const app_data_t *data);
static void print_bms_status(const app_data_t *data);
static void print_bms_summary(const app_data_t *data);
static void print_bms_config(const app_data_t *data);
static void print_bms_signals(void);
static uint8_t queue_bms_command(app_data_t *data, uint8_t opcode,
    uint8_t transaction, uint8_t argument, uint32_t descriptor);
static void process_bms_slot_command(app_data_t *data, const char *cmd);
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
    inverter_fault_status_snapshot_t inverter_status = {0};
    motor_control_read_fault_status(&data->motorControl, &inverter_status);
    serial_log("STATUS state=%s rtd=%u rtd_sound=%u faults=0x%08lX apps=%u bpps=%u inverter_fresh=%u throttle=%u pedal_command=%u brake=%u",
        car_state_name(data->car_state), (unsigned)data->ready_to_drive,
        (unsigned)data->rtd_sound_active, (unsigned long)data->safety_faults,
        (unsigned)data->motorControl.input_faults.apps_fault,
        (unsigned)data->motorControl.input_faults.bpps_fault,
        (unsigned)inverter_status.valid,
        (unsigned)data->throttle_level,
        (unsigned)data->motorControl.torqueCommand,
        (unsigned)data->brake_level);
}

static void print_inspection_status(const app_data_t *data) {
    const uint8_t tsms = (HAL_GPIO_ReadPin(GPIOA, TSMS_PIN) == GPIO_PIN_SET);
    const uint8_t bms = (HAL_GPIO_ReadPin(GPIOA, BMS_PIN) == GPIO_PIN_SET);
    inverter_fault_status_snapshot_t inverter_status = {0};
    motor_control_read_fault_status(&data->motorControl, &inverter_status);
    serial_log("INSPECTION torque_permitted=%u", (data->car_state == CAR_ENABLE &&
        data->safety_faults == SAFETY_FAULT_NONE));
    serial_log("INSPECTION tsms=%u bms=%u apps_fault=%u bpps_fault=%u inverter_fault=%u",
        tsms, bms, data->motorControl.input_faults.apps_fault,
        data->motorControl.input_faults.bpps_fault,
        is_fault(&inverter_status.fault_codes));
    serial_log("INSPECTION note=verify physical shutdown circuit, BSPD, IMD, AMS, inertia switch, and indicators at vehicle");
}

static void print_io(void) {
    adc_snapshot_t adc_snapshot = {0};
    const adc_snapshot_status_t adc_status = adc_acquisition_read(
        &adc_snapshot, HAL_GetTick());

    serial_log("IO tsms=%u r2d=%u bms=%u brake_light=%u fault_indicator=%u",
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, TSMS_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, R2D_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, BMS_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, BRAKE_LIGHT_PIN) == GPIO_PIN_SET),
        (unsigned)(HAL_GPIO_ReadPin(GPIOA, HOOP_LIGHT_PIN) == GPIO_PIN_SET));
    if (adc_status == ADC_SNAPSHOT_OK) {
        serial_log("ADC throttle1=%u throttle2=%u brake1=%u brake2=%u thermistor1=%u thermistor2=%u sequence=%lu",
            adc_snapshot.channels[0], adc_snapshot.channels[1],
            adc_snapshot.channels[2], adc_snapshot.channels[3],
            adc_snapshot.channels[4], adc_snapshot.channels[5],
            (unsigned long)adc_snapshot.sample_sequence);
    } else {
        serial_log("ADC unavailable status=%u errors=0x%08lX sequence=%lu",
            (unsigned)adc_status, (unsigned long)adc_snapshot.error_flags,
            (unsigned long)adc_snapshot.sample_sequence);
    }
}

static void print_can(const app_data_t *data) {
    serial_log("CAN bitrate=%lu rx_pending=%u fault_pending=%u bms_pending=%u rx_drop=%lu bms_drop=%lu rx_reject=%lu rx_malformed=%lu critical_updates=%lu",
        (unsigned long)CAN_BUS_BITRATE_BPS,
        (unsigned)uxQueueMessagesWaiting(data->can_bus.can_rx_queue),
        (unsigned)uxQueueMessagesWaiting(data->can_bus.can_rx_fault_queue),
        (unsigned)uxQueueMessagesWaiting(data->can_bus.bms_rx_queue),
        (unsigned long)data->can_bus.rx_dropped,
        (unsigned long)data->can_bus.bms_rx_dropped,
        (unsigned long)data->can_bus.rx_rejected,
        (unsigned long)data->can_bus.rx_malformed,
        (unsigned long)data->can_bus.rx_critical_updates);
    serial_log("CAN tx_pending=%u motor_pending=%u tx_errors=%lu bus_off=%lu tx_latched=%u",
        (unsigned)uxQueueMessagesWaiting(data->can_bus.can_tx_queue),
        (unsigned)uxQueueMessagesWaiting(data->can_bus.motor_command_queue),
        (unsigned long)data->can_bus.tx_errors,
        (unsigned long)data->can_bus.bus_off_count,
        (unsigned)data->can_bus.tx_safety.fault_latched);
    serial_log("CAN_TX queue=%lu dlc=%lu mailbox=%lu hal=%lu requeue=%lu busoff=%lu generic_c0=%lu inhibit=%u",
        (unsigned long)data->can_bus.tx_safety.queue_failures,
        (unsigned long)data->can_bus.tx_safety.invalid_dlc_failures,
        (unsigned long)data->can_bus.tx_safety.mailbox_timeout_failures,
        (unsigned long)data->can_bus.tx_safety.hal_transmit_failures,
        (unsigned long)data->can_bus.tx_safety.requeue_failures,
        (unsigned long)data->can_bus.tx_safety.bus_off_failures,
        (unsigned long)data->can_bus.tx_safety.generic_motor_command_failures,
        (unsigned)data->can_bus.tx_safety.inhibit_requested);
}

static void print_bms_status(const app_data_t *data) {
    const uint32_t now = (uint32_t)xTaskGetTickCount();
    const uint32_t stale_ms = (uint32_t)data->bms.layout.period_ms * 2U + 1000U;
    const uint8_t online = (data->bms.last_rx_ms != 0U &&
        (uint32_t)(now - data->bms.last_rx_ms) <= stale_ms) ? 1U : 0U;

    serial_log("BMS online=%u layout_valid=%u generation=%u summaries=%u period_ms=%u seen=0x%04X",
        online, data->bms.layout_valid, data->bms.layout.generation,
        data->bms.layout.active_count, data->bms.layout.period_ms,
        data->bms.summary_seen_mask);
    serial_log("BMS_ACK count=%lu opcode=0x%02X transaction=%u status=%u open=%u rx_malformed=%lu",
        (unsigned long)data->bms.ack_count, data->bms.last_opcode,
        data->bms.last_transaction, data->bms.last_status,
        data->bms.transaction_open,
        (unsigned long)data->bms.malformed_frames);
}

static void print_bms_summary(const app_data_t *data) {
    const bms_client_values_t *values = &data->bms.values;

    serial_log("BMS_SUMMARY state=%u soc_permille=%u pack_mv=%ld pack_ma=%ld",
        values->state, values->soc_permille, (long)values->pack_voltage_mv,
        (long)values->pack_current_ma);
    serial_log("BMS_SUMMARY min_cell_mv=%u max_cell_mv=%u max_temp_dc=%d active=0x%08lX latched=0x%08lX",
        values->minimum_cell_mv, values->maximum_cell_mv,
        values->maximum_temperature_dc, (unsigned long)values->active_faults,
        (unsigned long)values->latched_faults);
    serial_log("BMS_SUMMARY outputs=0x%02X cells=%u temperatures=%u valid=0x%08lX",
        values->output_requests, values->valid_cell_count,
        values->valid_temperature_count,
        (unsigned long)values->valid_signals);
}

static void print_bms_config(const app_data_t *data) {
    uint32_t slot;

    serial_log("BMS_CONFIG generation=%u count=%u period_ms=%u valid=%u readback=%u",
        data->bms.layout.generation, data->bms.layout.active_count,
        data->bms.layout.period_ms, data->bms.layout_valid,
        data->bms.readback_in_progress);
    for (slot = 0U; slot < data->bms.layout.active_count; slot++) {
        char line[LOG_MSG_MAX_LEN] = {0};
        size_t used = (size_t)snprintf(line, sizeof(line),
            "BMS_SLOT %lu", (unsigned long)slot);
        uint32_t signal_index;

        for (signal_index = 0U;
            signal_index < BMS_CAN_SUMMARY_MAX_SIGNALS; signal_index++) {
            const uint8_t signal =
                data->bms.layout.slots[slot].signals[signal_index];
            int written;
            if (signal == BMS_CAN_SIGNAL_NONE || used >= sizeof(line)) {
                break;
            }
            written = snprintf(&line[used], sizeof(line) - used, " %s",
                bms_client_signal_name(signal));
            if (written < 0 || (size_t)written >= sizeof(line) - used) {
                break;
            }
            used += (size_t)written;
        }
        serial_log("%s", line);
    }
}

static void print_bms_signals(void) {
    uint8_t signal;
    char line[LOG_MSG_MAX_LEN] = "BMS_SIGNALS";
    size_t used = strlen(line);

    for (signal = 1U; signal <= BMS_CAN_SIGNAL_MAX; signal++) {
        const char *name = bms_client_signal_name(signal);
        const int written = snprintf(&line[used], sizeof(line) - used,
            " %s", name);
        if (written < 0 || (size_t)written >= sizeof(line) - used) {
            serial_log("%s", line);
            (void)snprintf(line, sizeof(line), "BMS_SIGNALS %s", name);
            used = strlen(line);
        } else {
            used += (size_t)written;
        }
    }
    serial_log("%s", line);
}

static uint8_t queue_bms_command(app_data_t *data, uint8_t opcode,
    uint8_t transaction, uint8_t argument, uint32_t descriptor) {
    can_tx_message_t message = {0};

    message.tx_id = BMS_CAN_COMMAND_ID;
    message.dlc = BMS_CAN_FRAME_DLC;
    bms_client_build_command(opcode, transaction, argument, descriptor,
        message.tx_packet);
    if (can_bus_queue_generic_message(&data->can_bus, &message) != pdPASS) {
        serial_log("BMS command queue failed opcode=0x%02X transaction=%u",
            opcode, transaction);
        return 0U;
    }
    serial_log("BMS command queued opcode=0x%02X transaction=%u",
        opcode, transaction);
    return 1U;
}

static void process_bms_slot_command(app_data_t *data, const char *cmd) {
    char copy[CLI_BUFFER_SIZE];
    char *token;
    uint8_t signals[BMS_CAN_SUMMARY_MAX_SIGNALS] = {0};
    uint32_t signal_count = 0U;
    uint32_t packed_bytes = 0U;
    unsigned long slot;

    (void)snprintf(copy, sizeof(copy), "%s", cmd);
    token = strtok(copy, " ");
    token = (token != NULL) ? strtok(NULL, " ") : NULL;
    token = (token != NULL) ? strtok(NULL, " ") : NULL;
    token = (token != NULL) ? strtok(NULL, " ") : NULL;
    if (token == NULL) {
        serial_log("Usage: bms summary slot <0..15> <signal...>");
        return;
    }
    slot = strtoul(token, NULL, 10);
    if (slot >= BMS_CAN_SUMMARY_MAX_FRAMES) {
        serial_log("Invalid BMS summary slot %lu", slot);
        return;
    }
    while ((token = strtok(NULL, " ")) != NULL) {
        uint8_t signal;
        uint8_t width;
        if (signal_count >= BMS_CAN_SUMMARY_MAX_SIGNALS ||
            bms_client_signal_from_name(token, &signal) == 0U) {
            serial_log("Invalid or excessive BMS signal: %s", token);
            return;
        }
        width = bms_client_signal_width(signal);
        if (packed_bytes + width > BMS_CAN_FRAME_DLC) {
            serial_log("BMS slot payload exceeds 8 bytes");
            return;
        }
        signals[signal_count++] = signal;
        packed_bytes += width;
    }
    if (signal_count == 0U) {
        serial_log("A BMS summary slot requires at least one signal");
        return;
    }
    (void)queue_bms_command(data, BMS_CAN_CMD_SET_SLOT,
        data->bms.transaction_id, (uint8_t)slot,
        bms_client_pack_descriptor(signals));
}

static void print_storage(const app_data_t *data) {
    const char *owner = (sd_card_owner == MCU_SD_CARD) ? "MCU" : "USB";
    serial_log("STORAGE owner=%s state=%u result=%u mounted=%u file_open=%u log_number=%lu queue_pending=%u",
        owner, (unsigned)data->sd_card.policy.state,
        (unsigned)data->sd_card.policy.last_result,
        (unsigned)data->sd_card.mounted, (unsigned)data->sd_card.file_opened,
        (unsigned long)data->sd_card.log_number,
        (unsigned)((data->sd_card.sd_card_q != NULL) ?
            uxQueueMessagesWaiting(data->sd_card.sd_card_q) : 0U));
    serial_log("STORAGE errors=%lu dropped=%lu written=%lu recovery_pending=%u msc_enabled=%u",
        (unsigned long)data->sd_card.policy.error_count,
        (unsigned long)data->sd_card.policy.dropped_records,
        (unsigned long)data->sd_card.policy.records_written,
        (unsigned)data->sd_card.policy.recovery_requested,
        (unsigned)VCU_USB_MSC_SD_ENABLED);
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
    inverter_fault_status_snapshot_t inverter_status = {0};
    motor_control_read_fault_status(&data->motorControl, &inverter_status);
    const uint8_t inverter_fresh = inverter_fault_status_snapshot_is_fresh(
        &inverter_status, (uint32_t)now,
        (uint32_t)pdMS_TO_TICKS(CAN_INVERTER_FAULT_STATUS_TIMEOUT_MS));
    const uint8_t healthy = (data->safety_faults == SAFETY_FAULT_NONE) &&
        (tsms != 0U) && (bms != 0U) && (r2d != 0U) &&
        (data->motorControl.input_faults.apps_fault == 0U) &&
        (data->motorControl.input_faults.bpps_fault == 0U) &&
        (inverter_fresh != 0U) && !is_fault(&inverter_status.fault_codes);

    serial_log("HEALTH result=%s state=%s tsms=%u bms=%u r2d=%u apps=%u bpps=%u inverter_fresh=%u faults=0x%08lX",
        healthy ? "PASS" : "WARN", car_state_name(data->car_state), tsms, bms, r2d,
        (unsigned)data->motorControl.input_faults.apps_fault,
        (unsigned)data->motorControl.input_faults.bpps_fault,
        inverter_fresh, (unsigned long)data->safety_faults);
}

static void print_self_test(const app_data_t *data) {
    uint8_t tasks_ok = 1U;
    for (size_t i = 0; i < NUM_TASKS; i++) {
        if (i != sd_card_task_index &&
            data->task_entries[i].handle == NULL) {
            tasks_ok = 0U;
            break;
        }
    }

    const uint8_t queues_ok = (data->cli_queue != NULL) &&
        (data->can_bus.can_rx_queue != NULL) &&
        (data->can_bus.can_rx_fault_queue != NULL) &&
        (data->can_bus.bms_rx_queue != NULL) &&
        (data->can_bus.can_tx_queue != NULL) &&
        (data->can_bus.motor_command_queue != NULL);
    adc_snapshot_t adc_snapshot = {0};
    const uint8_t adc_ok = (adc_acquisition_read(&adc_snapshot,
        HAL_GetTick()) == ADC_SNAPSHOT_OK);
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
        adc_snapshot_t adc_snapshot = {0};
        const adc_snapshot_status_t adc_status = adc_acquisition_read(
            &adc_snapshot, HAL_GetTick());
        if (adc_status == ADC_SNAPSHOT_OK) {
            serial_log("ADC1: %u ADC2: %u ADC3: %u ADC4: %u ADC5: %u ADC6: %u",
                adc_snapshot.channels[0], adc_snapshot.channels[1],
                adc_snapshot.channels[2], adc_snapshot.channels[3],
                adc_snapshot.channels[4], adc_snapshot.channels[5]);
        } else {
            serial_log("ADC unavailable status=%u errors=0x%08lX",
                (unsigned)adc_status,
                (unsigned long)adc_snapshot.error_flags);
        }
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
        inverter_fault_status_snapshot_t inverter_status = {0};
        motor_control_read_fault_status(&app->motorControl, &inverter_status);
        serial_log("FAULTS active=0x%08lX apps=%u bpps=%u inverter=%u",
            (unsigned long)app->safety_faults,
            app->motorControl.input_faults.apps_fault,
            app->motorControl.input_faults.bpps_fault,
            is_fault(&inverter_status.fault_codes));
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

    if (strcmp(cmd, "bms") == 0 || strcmp(cmd, "bms status") == 0) {
        print_bms_status(app);
        return;
    }

    if (strcmp(cmd, "bms summary") == 0 ||
        strcmp(cmd, "bms summary show") == 0) {
        print_bms_summary(app);
        return;
    }

    if (strcmp(cmd, "bms config show") == 0) {
        print_bms_config(app);
        return;
    }

    if (strcmp(cmd, "bms signals") == 0) {
        print_bms_signals();
        return;
    }

    if (strcmp(cmd, "bms config read") == 0) {
        const uint8_t transaction = bms_client_allocate_transaction(&app->bms);
        (void)queue_bms_command(app, BMS_CAN_CMD_READ_CONFIG,
            transaction, 0U, 0U);
        return;
    }

    if (strncmp(cmd, "bms config ", 11) == 0 ||
        strncmp(cmd, "bms summary count ", 18) == 0 ||
        strncmp(cmd, "bms summary slot ", 17) == 0) {
        if (service_mode_active() == 0U) {
            serial_log("BMS configuration requires: service begin");
            return;
        }
    }

    if (strcmp(cmd, "bms config begin") == 0) {
        const uint8_t transaction = bms_client_allocate_transaction(&app->bms);
        if (queue_bms_command(app, BMS_CAN_CMD_BEGIN, transaction, 0U,
            0U) != 0U) {
            serial_log("Wait for the BEGIN acknowledgement before editing");
        }
        return;
    }

    {
        unsigned int summary_count;
        if (sscanf(cmd, "bms summary count %u", &summary_count) == 1) {
            if (app->bms.transaction_open == 0U) {
                serial_log("Start a BMS configuration transaction first");
            } else if (summary_count > BMS_CAN_SUMMARY_MAX_FRAMES) {
                serial_log("BMS summary count must be 0 to %u",
                    BMS_CAN_SUMMARY_MAX_FRAMES);
            } else {
                (void)queue_bms_command(app, BMS_CAN_CMD_SET_COUNT,
                    app->bms.transaction_id, (uint8_t)summary_count, 0U);
            }
            return;
        }
    }

    if (strncmp(cmd, "bms summary slot ", 17) == 0) {
        if (app->bms.transaction_open == 0U) {
            serial_log("Start a BMS configuration transaction first");
        } else {
            process_bms_slot_command(app, cmd);
        }
        return;
    }

    if (strcmp(cmd, "bms config validate") == 0 ||
        strcmp(cmd, "bms config commit") == 0 ||
        strcmp(cmd, "bms config abort") == 0) {
        uint8_t opcode = BMS_CAN_CMD_VALIDATE;
        if (app->bms.transaction_open == 0U) {
            serial_log("No BMS configuration transaction is open");
            return;
        }
        if (strcmp(cmd, "bms config commit") == 0) {
            opcode = BMS_CAN_CMD_COMMIT;
        } else if (strcmp(cmd, "bms config abort") == 0) {
            opcode = BMS_CAN_CMD_ABORT;
        }
        (void)queue_bms_command(app, opcode, app->bms.transaction_id,
            0U, 0U);
        return;
    }

    if (strcmp(cmd, "bms config save") == 0) {
        const uint8_t transaction = bms_client_allocate_transaction(&app->bms);
        (void)queue_bms_command(app, BMS_CAN_CMD_SAVE, transaction, 0U, 0U);
        return;
    }

    if (strcmp(cmd, "storage") == 0) {
        print_storage(app);
        return;
    }

    if (strcmp(cmd, "storage retry") == 0) {
        storage_policy_request_recovery(&app->sd_card.policy);
        serial_log("STORAGE recovery requested");
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
        const char *mode = cmd + 11;
        pedal_response_mode_t selected_mode;
        uint8_t changed = 0U;
        if (strcmp(mode, "linear") == 0) {
            selected_mode = PEDAL_RESPONSE_LINEAR;
        } else if (strcmp(mode, "early") == 0) {
            selected_mode = PEDAL_RESPONSE_EARLY;
        } else if (strcmp(mode, "balanced") == 0) {
            selected_mode = PEDAL_RESPONSE_BALANCED;
        } else if (strcmp(mode, "progressive") == 0) {
            selected_mode = PEDAL_RESPONSE_PROGRESSIVE;
        } else {
            serial_log("Invalid pedal mode: %s; use linear, early, balanced, or progressive", mode);
            return;
        }
        if (!config_change_allowed(app)) {
            return;
        }
        taskENTER_CRITICAL();
        if (app->car_state == CAR_IDLE && app->ready_to_drive == 0U) {
            app->pedal_response.mode = selected_mode;
            changed = 1U;
        }
        taskEXIT_CRITICAL();
        if (changed == 0U) {
            serial_log("Refusing pedal configuration change: vehicle is not IDLE");
            return;
        }
        serial_log("PEDAL mode=%s", pedal_response_mode_name(app->pedal_response.mode));
        return;
    }

    unsigned int strength_percent = 0U;
    if (sscanf(cmd, "pedal strength %u", &strength_percent) == 1) {
        uint8_t changed = 0U;
        if (strength_percent > 100U) {
            serial_log("Invalid pedal strength %u; use 0 to 100", strength_percent);
            return;
        }
        if (!config_change_allowed(app)) {
            return;
        }
        taskENTER_CRITICAL();
        if (app->car_state == CAR_IDLE && app->ready_to_drive == 0U) {
            app->pedal_response.strength =
                (uint16_t)(strength_percent * 10U);
            changed = 1U;
        }
        taskEXIT_CRITICAL();
        if (changed == 0U) {
            serial_log("Refusing pedal configuration change: vehicle is not IDLE");
            return;
        }
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
#if !VCU_CLI_TASK_CONTROL_ENABLED
    (void)app;
    (void)task_name;
    (void)value;
    serial_log("Task control is disabled in this vehicle build");
    return;
#else
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
#endif
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
        serial_log("COMMANDS status health self-test inspection faults io sensors can bms storage reset-cause version tasks pedal config service reboot watch help");
        serial_log("READ-ONLY status health self-test inspection faults io sensors can storage reset-cause version tasks pedal config");
        serial_log("PEDAL pedal show | pedal preview | pedal mode linear|early|balanced|progressive | pedal strength 0..100");
        serial_log("CONFIG config show | config validate; settings are runtime-only");
        serial_log("SERVICE service begin|status|end; task control requires active service mode");
        serial_log("REBOOT reboot confirm; only allowed while vehicle is IDLE");
        serial_log("WATCH watch sensors | watch off");
        serial_log("BMS bms status|summary show|config show|config read|signals");
        serial_log("BMS EDIT service begin; bms config begin; bms summary count|slot; bms config validate|commit|save|abort");
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
    } else if (strcmp(topic, "bms") == 0) {
        serial_log("bms status: show link and last acknowledgement");
        serial_log("bms summary show | bms config show | bms config read | bms signals");
        serial_log("Editing requires service begin, then bms config begin");
        serial_log("bms summary count <0..16>");
        serial_log("bms summary slot <0..15> <signal...>");
        serial_log("bms config validate | commit | save | abort");
    } else if (strcmp(topic, "storage") == 0) {
        serial_log("storage: show optional SD state, errors, drops, and ownership");
        serial_log("storage retry: retry only after a latched storage failure");
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
