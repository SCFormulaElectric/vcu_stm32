#include "app.h"
#include "Tasks/Task_Helper/tasks.h"
#include "Tasks/Task_Helper/handles.h"

app_data_t app = {0};

static StaticQueue_t CLI_QUEUE;
static uint8_t CLI_Q_STORAGE[ CLI_QUEUE_LENGTH * CLI_ITEM_SIZE ];

static StaticQueue_t CAN_RX_Q;
static uint8_t CAN_RX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_RX_MESSAGE_SIZE ];
static StaticQueue_t CAN_RX_FAULT_Q;
static uint8_t CAN_RX_FAULT_Q_STORAGE[ CAN_RX_MESSAGE_SIZE ];
static StaticQueue_t BMS_CAN_RX_Q;
static uint8_t BMS_CAN_RX_Q_STORAGE[
    BMS_CAN_QUEUE_LENGTH * CAN_RX_MESSAGE_SIZE ];
static StaticQueue_t CAN_TX_Q;
static uint8_t CAN_TX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_TX_MESSAGE_SIZE ];
static StaticQueue_t MOTOR_COMMAND_Q;
static uint8_t MOTOR_COMMAND_Q_STORAGE[ CAN_TX_MESSAGE_SIZE ];

static uint8_t LOG_Q_STORAGE[ LOG_QUEUE_LENGTH * LOG_MSG_SIZE ];
static StaticQueue_t SD_CARD_Q;
static QueueHandle_t sd_card_q_handle;
sd_card_t sd_card = {0};

static task_entry_t entries[NUM_TASKS] = {0};
static EventGroupHandle_t wd_event_group;
void create_app(){
    // STARTUP CONFIGURATIONS
#if VCU_IWDG_ENABLE
    app.startup_mode = START_ALL;
#else
    app.startup_mode = START_NO_IDWG;
#endif
    /* Logging is optional; the safe default requires neither SD nor USB CDC. */
    app.log_level = LOG_NONE;

    // IDWG
    wd_event_group = xEventGroupCreate();
    configASSERT(wd_event_group);
    app.idwg_group = wd_event_group;

    // MOTOR CONTROLLER STUFF
    app.car_state = CAR_IDLE;
    app.boot_healthy = 0U;
    app.throttle_level = 0;
    app.brake_level = 0;
    app.pedal_response.mode = PEDAL_RESPONSE_DEFAULT_MODE;
    app.pedal_response.strength = PEDAL_RESPONSE_DEFAULT_STRENGTH;
    app.motorControl = (MotorControl_t){0};
    
    // CAN BUS STUFF
    app.can_bus = (can_bus_t){0};
    app.can_bus.hcan = &hcan1;
    QueueHandle_t can_rx_q_handle;
    QueueHandle_t can_rx_fault_q_handle;
    QueueHandle_t bms_can_rx_q_handle;
    QueueHandle_t can_tx_q_handle;
    QueueHandle_t motor_command_q_handle;
    can_tx_message_t initial_motor_command;
    can_rx_q_handle = xQueueCreateStatic( CAN_QUEUE_LENGTH,
                                CAN_RX_MESSAGE_SIZE,
                                CAN_RX_Q_STORAGE,
                                 &CAN_RX_Q );
    can_rx_fault_q_handle = xQueueCreateStatic( 1U,
                                CAN_RX_MESSAGE_SIZE,
                                CAN_RX_FAULT_Q_STORAGE,
                                &CAN_RX_FAULT_Q );
    bms_can_rx_q_handle = xQueueCreateStatic(BMS_CAN_QUEUE_LENGTH,
                                CAN_RX_MESSAGE_SIZE,
                                BMS_CAN_RX_Q_STORAGE,
                                &BMS_CAN_RX_Q);
    can_tx_q_handle = xQueueCreateStatic( CAN_QUEUE_LENGTH,
                                CAN_TX_MESSAGE_SIZE,
                                CAN_TX_Q_STORAGE,
                                &CAN_TX_Q );
    motor_command_q_handle = xQueueCreateStatic( 1U,
                                CAN_TX_MESSAGE_SIZE,
                                MOTOR_COMMAND_Q_STORAGE,
                                &MOTOR_COMMAND_Q );
    configASSERT(can_rx_q_handle);
    configASSERT(can_rx_fault_q_handle);
    configASSERT(bms_can_rx_q_handle);
    configASSERT(can_tx_q_handle);
    configASSERT(motor_command_q_handle);
    app.can_bus.can_rx_queue = can_rx_q_handle;
    app.can_bus.can_rx_fault_queue = can_rx_fault_q_handle;
    app.can_bus.bms_rx_queue = bms_can_rx_q_handle;
    app.can_bus.can_tx_queue = can_tx_q_handle;
    app.can_bus.motor_command_queue = motor_command_q_handle;
    can_tx_safety_initialize(&app.can_bus.tx_safety, &initial_motor_command);
    if (xQueueOverwrite(motor_command_q_handle, &initial_motor_command) != pdPASS) {
        can_tx_safety_latch_failure(&app.can_bus.tx_safety,
            CAN_TX_FAILURE_QUEUE);
    }
    bms_client_initialize(&app.bms);
    {
        can_tx_message_t read_command = {0};
        const uint8_t transaction = bms_client_allocate_transaction(&app.bms);
        read_command.tx_id = BMS_CAN_COMMAND_ID;
        read_command.dlc = BMS_CAN_FRAME_DLC;
        bms_client_build_command(BMS_CAN_CMD_READ_CONFIG, transaction, 0U,
            0U, read_command.tx_packet);
        (void)can_bus_queue_generic_message(&app.can_bus, &read_command);
    }

    // CLI STUFF
    QueueHandle_t cli_q_handle;
    cli_q_handle = xQueueCreateStatic( CLI_QUEUE_LENGTH,
                                CLI_ITEM_SIZE,
                                CLI_Q_STORAGE,
                                &CLI_QUEUE );
    configASSERT(cli_q_handle);
    app.cli_queue = cli_q_handle;

    // SD CARD STUFF
    sd_card_q_handle = xQueueCreateStatic( LOG_QUEUE_LENGTH,
                                LOG_MSG_SIZE,
                                LOG_Q_STORAGE,
                                &SD_CARD_Q );
    sd_card.sd_card_q = sd_card_q_handle;
    storage_policy_initialize(&sd_card.policy, 0U);
    app.sd_card = sd_card;


    (void)entries;
    app.task_entries[throttle_task_index] = create_throttle_task(&app);
    app.task_entries[brake_pedal_plausibility_check_task_index] = create_brake_pedal_plausibility_check_task(&app);
    app.task_entries[can_receiver_task_index] = create_can_receiver_task(&app);
    app.task_entries[can_transmitter_task_index] = create_can_transmitter_task(&app);
    app.task_entries[cli_input_task_index] = create_cli_input_task(&app);
    app.task_entries[cooling_task_index] = create_cooling_task(&app);
    app.task_entries[dash_task_index] = create_dash_task(&app);
    app.task_entries[default_task_task_index] = create_default_task_task(&app);
    app.task_entries[independent_watchdog_task_index] = create_independent_watchdog_task(&app);
    app.task_entries[light_controller_task_index] = create_light_controller_task(&app);
    app.task_entries[motor_controller_task_index] = create_motor_controller_task(&app);
    app.task_entries[sd_card_task_index] = create_sd_card_task(&app);
    app.task_entries[state_machine_task_index] = create_state_machine_task(&app);
    app.task_entries[telemetry_task_index] = create_telemetry_task(&app);
    for (size_t i = 0; i < NUM_TASKS; i++) {
        if (i != sd_card_task_index) {
            configASSERT(app.task_entries[i].handle != NULL);
        }
    }
    if(app.startup_mode == START_ALL || app.startup_mode == START_NO_IDWG) {
        for (size_t i = 0; i < NUM_TASKS; i++) {
            TaskHandle_t handle = app.task_entries[i].handle;
            if (handle != NULL && strcmp(app.task_entries[i].name, "idwg") != 0) {
                vTaskResume(handle);
            }
        }
        if (app.startup_mode == START_ALL) {
            vTaskResume(app.task_entries[independent_watchdog_task_index].handle);
        }
    }
    else if(app.startup_mode == START_CLI_ONLY) {
        TaskHandle_t cli_handle = app.task_entries[cli_input_task_index].handle;
        vTaskResume(cli_handle);
    }
}

void __serial_print(const char *str) {
    CDC_Transmit_FS((uint8_t *)str, strlen(str));
}

void serial_log(const char *fmt, ...)
{
    if (app.log_level == LOG_NONE) {
        return;
    }

    log_msg_t log;
    TickType_t ticks = xTaskGetTickCount();
    uint32_t ms = (ticks * 1000UL) / configTICK_RATE_HZ;
    uint32_t sec = ms / 1000;
    uint32_t rem = ms % 1000;

    char msg[96];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    snprintf(log.line, sizeof(log.line), "[%lu.%03lu] %s\r\n", sec, rem, msg);
    if (app.log_level == LOG_SERIAL) {
        __serial_print(log.line);
        return;
    }

    if (sd_card_q_handle != NULL &&
        storage_policy_accepts_logs(&app.sd_card.policy) != 0U) {
        BaseType_t status = xQueueSend(sd_card_q_handle, &log, 0);

        if (status != pdPASS) {
            app.sd_card.policy.dropped_records++;
        }
    } else {
        app.sd_card.policy.dropped_records++;
    }
}
