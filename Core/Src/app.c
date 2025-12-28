#include "app.h"
#include "Tasks/Task_Helper/tasks.h"
#include "Tasks/Task_Helper/handles.h"

app_data_t app = {0};

static StaticQueue_t CLI_QUEUE;
static uint8_t CLI_Q_STORAGE[ CLI_QUEUE_LENGTH * CLI_ITEM_SIZE ];

static StaticQueue_t CAN_RX_Q;
static uint8_t CAN_RX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_RX_MESSAGE_SIZE ];
static StaticQueue_t CAN_TX_Q;
static uint8_t CAN_TX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_TX_MESSAGE_SIZE ];

static uint8_t LOG_Q_STORAGE[ LOG_QUEUE_LENGTH * LOG_MSG_SIZE ];
static StaticQueue_t SD_CARD_Q;
static QueueHandle_t sd_card_q_handle;
sd_card_t sd_card = {0};

static task_entry_t entries[NUM_TASKS] = {0};
static EventGroupHandle_t wd_event_group;
void create_app(){
    // STARTUP CONFIGURATIONS
    app.startup_mode = START_ALL;
    app.log_level = LOG_SD_CARD;

    // IDWG
    wd_event_group = xEventGroupCreate();
    configASSERT(wd_event_group);
    app.idwg_group = wd_event_group;

    // MOTOR CONTROLLER STUFF
    app.car_state = CAR_IDLE;
    app.throttle_level = 0;
    app.brake_level = 0;
    app.motorControl = (MotorControl_t){0};
    
    // CAN BUS STUFF
    app.can_bus = (can_bus_t){0};
    app.can_bus.hcan = &hcan1;
    QueueHandle_t can_rx_q_handle;
    QueueHandle_t can_tx_q_handle;
    can_rx_q_handle = xQueueCreateStatic( CAN_QUEUE_LENGTH,
                                CAN_RX_MESSAGE_SIZE,
                                CAN_RX_Q_STORAGE,
                                &CAN_RX_Q );
    can_tx_q_handle = xQueueCreateStatic( CAN_QUEUE_LENGTH,
                                CAN_TX_MESSAGE_SIZE,
                                CAN_TX_Q_STORAGE,
                                &CAN_TX_Q );
    configASSERT(can_rx_q_handle);
    configASSERT(can_tx_q_handle);
    app.can_bus.can_rx_queue = can_rx_q_handle;
    app.can_bus.can_tx_queue = can_tx_q_handle;

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
    configASSERT(sd_card_q_handle != NULL);
    sd_card.sd_card_q = sd_card_q_handle;
    app.sd_card = sd_card;


    app.task_entries = entries;
    configASSERT((app.task_entries[throttle_task_index] = create_throttle_task(&app).handle) != NULL);
    configASSERT((app.task_entries[brake_pedal_plausibility_check_task_index] = create_brake_pedal_plausibility_check_task(&app).handle) != NULL);
    configASSERT((app.task_entries[can_receiver_task_index] = create_can_receiver_task(&app).handle) != NULL);
    configASSERT((app.task_entries[can_transmitter_task_index] = create_can_transmitter_task(&app).handle) != NULL);
    configASSERT((app.task_entries[cli_input_task_index] = create_cli_input_task(&app).handle) != NULL);
    configASSERT((app.task_entries[cooling_task_index] = create_cooling_task(&app).handle) != NULL);
    configASSERT((app.task_entries[dash_task_index] = create_dash_task(&app).handle) != NULL);
    configASSERT((app.task_entries[default_task_task_index] = create_default_task_task(&app).handle) != NULL);
    configASSERT((app.task_entries[independent_watchdog_task_index] = create_independent_watchdog_task(&app).handle) != NULL);
    configASSERT((app.task_entries[light_controller_task_index] = create_light_controller_task(&app).handle) != NULL);
    configASSERT((app.task_entries[motor_controller_task_index] = create_motor_controller_task(&app).handle) != NULL);
    configASSERT((app.task_entries[sd_card_task_index] = create_sd_card_task(&app).handle) != NULL);
    configASSERT((app.task_entries[state_machine_task_index] = create_state_machine_task(&app).handle) != NULL);
    configASSERT((app.task_entries[telemetry_task_index] = create_telemetry_task(&app).handle) != NULL);
    if(app.startup_mode == START_ALL || app.startup_mode == START_NO_IDWG) {
        for (size_t i = 0; i < NUM_TASKS; i++) {
            TaskHandle_t handle = app.task_entries[i].handle;
            if (handle != NULL && app.task_entries[i].name != "idwg") {
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
    __serial_print(log.line);

    if (app.log_level == LOG_SERIAL) {
        return;
    }

    if (sd_card_q_handle != NULL) {
        BaseType_t status = xQueueSend(sd_card_q_handle, &log, 0);

        if (status != pdPASS) {
            __serial_print("Queue full for sd_card");
        }
    }
}