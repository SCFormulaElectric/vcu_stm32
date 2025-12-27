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
static task_entry_t entries[NUM_TASKS] = {0};
static EventGroupHandle_t wd_event_group;
static sd_card_t sd_card = {0};
void create_app(){
    // STARTUP CONFIGURATIONS
    app.startup_mode = START_ALL;
    app.log_level = LOG_ALL;

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
    char filename[32];
    sd_card.log_number = find_next_log_index();
    // Only proceed if the MCU owns the SD card
    if (sd_card_owner == MCU_SD_CARD) {
        f_mount(&sd_card.file_system, "", 1);
        snprintf(sd_card.filename, sizeof(sd_card.filename), "log_%lu.txt", sd_card.log_number);
        FRESULT file_result = f_open(&sd_card.file, sd_card.filename, FA_WRITE | FA_CREATE_NEW);
        configASSERT(file_result == FR_OK);
        sd_card.file_created = 1;
    }
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

void serial_print(const char *fmt, ...) {
    if (app.log_level == LOG_ALL){
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
    
        CDC_Transmit_FS((uint8_t*)buf, strlen(buf));
    }
}

uint32_t find_next_log_index(void) {
    FILINFO fno;
    char filename[32];
    uint32_t index = 1;
    while (1)
    {
        snprintf(filename, sizeof(filename), "log_%lu.txt", index);
        if (f_stat(filename, &fno) != FR_OK)
        {
            return index;
        }
        index++;
    }
}