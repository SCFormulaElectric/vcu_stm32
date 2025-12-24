#include "app.h"
#include "tasks.h"
#include "handles.h"

static app_data_t app;
static MotorControl_t motor;

static StaticQueue_t CLI_QUEUE;
static uint8_t CLI_Q_STORAGE[ CLI_QUEUE_LENGTH * CLI_ITEM_SIZE ];

static StaticQueue_t CAN_RX_Q;
static uint8_t CAN_RX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_MESSAGE_SIZE ];
static StaticQueue_t CAN_TX_Q;
static uint8_t CAN_TX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_MESSAGE_SIZE ];

const size_t CLI_TASK_COUNT = sizeof(cli_tasks) / sizeof(cli_tasks[0]);
void create_app(){
    memset(&app, 0, sizeof(app));
    app.startup_mode = ALL;

    // MOTOR CONTROLLER STUFF
    memset(&motor, 0, sizeof(motor));
    app.throttle_level = 0;
    app.brake_level = 0;

    // CAN BUS STUFF
    // Todo @aut - These are not referencing the right things right now.
    app.can_bus.hcan = &hcan1;
    app.can_bus.tx_header = &tx_header;
    app.can_bus.rx_header = &rx_header;
    
    app.can_bus.tx_mailbox = 0;
    QueueHandle_t can_rx_q_handle;
    QueueHandle_t can_tx_q_handle;
    can_rx_q_handle = xQueueCreateStatic( CAN_QUEUE_LENGTH,
                                CAN_MESSAGE_SIZE,
                                CAN_RX_Q_STORAGE,
                                &CAN_RX_Q );
    can_tx_q_handle = xQueueCreateStatic( CAN_QUEUE_LENGTH,
                                CAN_MESSAGE_SIZE,
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
    task_entry_t entries[NUM_TASKS];
    memset(&entries, 0, sizeof(entries));
    app.task_entires = entries;

    if(app.startup_mode == ALL) {
        assert((app.task_entires[throttle_task_index] = create_throttle_task(&app)) != NULL);
        assert((app.task_entires[brake_pedal_plausibility_check_task_index] = create_brake_pedal_plausibility_check_task(&app)) != NULL);
        assert((app.task_entires[can_receiver_task_index] = create_can_receiver_task(&app)) != NULL);
        assert((app.task_entires[can_transmitter_task_index] = create_can_transmitter_task(&app)) != NULL);
        assert((app.task_entires[cli_input_task_index] = create_cli_input_task(&app)) != NULL);
        assert((app.task_entires[cooling_task_index] = create_cooling_task(&app)) != NULL);
        assert((app.task_entires[dash_task_index] = create_dash_task(&app)) != NULL);
        assert((app.task_entires[default_task_task_index] = create_default_task_task(&app)) != NULL);
        assert((app.task_entires[independent_watchdog_task_index] = create_independent_watchdog_task(&app)) != NULL);
        assert((app.task_entires[light_controller_task_index] = create_light_controller_task(&app)) != NULL);
        assert((app.task_entires[motor_controller_task_index] = create_motor_controller_task(&app)) != NULL);
        assert((app.task_entires[sd_card_task_index] = create_sd_card_task(&app)) != NULL);
        assert((app.task_entires[state_machine_task_index] = create_state_machine_task(&app)) != NULL);
        assert((app.task_entires[telemetry_task_index] = create_telemetry_task(&app)) != NULL);
    }
    else if(app.startup_mode == CLI_ONLY) {
        assert((app.task_entires[cli_input_task_index] = create_cli_input_task(&app)) != NULL);
    }
}

