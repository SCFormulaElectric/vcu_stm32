#include "app.h"

static volatile app_data_t app;
static volatile MotorControl_t motor;

void create_app(){
    memset(&app, 0, sizeof(app));
    app.startup_mode = ALL;

    memset(&motor, 0, sizeof(motor));
    app.throttle_level = 0;
    app.brake_level = 0;

    // Todo @aut - These are not referencing the right things right now.
    app.can_bus.hcan = &hcan1;
    app.can_bus.tx_header = &tx_header;
    app.can_bus.rx_header = &rx_header;
    
    app.can_bus.tx_mailbox = 0;
    app.can_bus.can_tx_queue = xQueueCreate(8, sizeof(can_message_t));
    app.can_bus.can_rx_queue = xQueueCreate(8, sizeof(can_message_t));
    assert(app.can_bus.can_tx_queue != NULL);
    assert(app.can_bus.can_rx_queue != NULL);
    


    if(app.startup_mode == ALL) {
        assert((app.throttle_task_handle = create_throttle_task(&app)) != NULL);
        assert((app.brake_pedal_plausibility_check_task_handle = create_brake_pedal_plausibility_check_task(&app)) != NULL);
        assert((app.can_receiver_task_handle = create_can_receiver_task(&app)) != NULL);
        assert((app.can_transmitter_task_handle = create_can_transmitter_task(&app)) != NULL);
        assert((app.cli_input_task_handle = create_cli_input_task(&app)) != NULL);
        assert((app.cooling_task_handle = create_cooling_task(&app)) != NULL);
        assert((app.dash_task_handle = create_dash_task(&app)) != NULL);
        assert((app.default_task_task_handle = create_default_task_task(&app)) != NULL);
        assert((app.independent_watchdog_task_handle = create_independent_watchdog_task(&app)) != NULL);
        assert((app.light_controller_task_handle = create_light_controller_task(&app)) != NULL);
        assert((app.motor_controller_task_handle = create_motor_controller_task(&app)) != NULL);
        assert((app.sd_card_task_handle = create_sd_card_task(&app)) != NULL);
        assert((app.state_machine_task_handle = create_state_machine_task(&app)) != NULL);
        assert((app.telemetry_task_handle = create_telemetry_task(&app)) != NULL);
    }
    else if(app.startup_mode == CLI_ONLY) {
        assert((app.cli_input_task_handle = create_cli_input_task(&app)) != NULL);
    }


}

