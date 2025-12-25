#include "app.h"
#include "tasks.h"
#include "handles.h"

app_data_t app = {0};
static MotorControl_t motor = {0};

static can_bus_t can_bus = {0};
static StaticQueue_t CLI_QUEUE;
static uint8_t CLI_Q_STORAGE[ CLI_QUEUE_LENGTH * CLI_ITEM_SIZE ];

static StaticQueue_t CAN_RX_Q;
static uint8_t CAN_RX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_RX_MESSAGE_SIZE ];
static StaticQueue_t CAN_TX_Q;
static uint8_t CAN_TX_Q_STORAGE[ CAN_QUEUE_LENGTH * CAN_TX_MESSAGE_SIZE ];
static task_entry_t entries[NUM_TASKS] = {0};

void create_app(){
    app.startup_mode = ALL;
    
    car_state_t car_state = CAR_IDLE;
    app.car_state = car_state;

    // MOTOR CONTROLLER STUFF
    app.throttle_level = 0;
    app.brake_level = 0;
    app.motorControl = motor;
    
    // CAN BUS STUFF
    app.can_bus = can_bus;
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

    app.task_entires = entries;

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
    if(app.startup_mode == ALL) {
        for (size_t i = 0; i < NUM_TASKS; i++) {
            TaskHandle_t handle = app.task_entires[i].handle;
            vTaskResume(handle);
        }
    }
    else if(app.startup_mode == CLI_ONLY) {
        TaskHandle_t cli_handle = app.task_entires[cli_input_task_index].handle;
        vTaskResume(cli_handle);
    }
}

