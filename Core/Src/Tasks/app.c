#include "app.h"

// Task includes
#include "Tasks/apps_implausibility_check_task.h"
#include "Tasks/brake_pedal_plausibility_check_task.h"
#include "Tasks/can_receiver_task.h"
#include "Tasks/can_transceiver_task.h"
#include "Tasks/cli_input_task.h"
#include "Tasks/cooling_task.h"
#include "Tasks/dash_task.h"
#include "Tasks/default_task_task.h"
#include "Tasks/independent_watchdog_task.h"
#include "Tasks/light_controller_task.h"
#include "Tasks/motor_controller_task.h"
#include "Tasks/sd_card_task.h"
#include "Tasks/state_machine_task.h"
#include "Tasks/telemetry_task.h"


app_data app;

void create_app(){
    if(app.startup_mode = ALL) {
        assert((app.apps_implausibility_check_task_handle = create_apps_implausibility_check_task()) != NULL);
        assert((app.brake_pedal_plausibility_check_task_handle = create_brake_pedal_plausibility_check_task()) != NULL);
        assert((app.can_receiver_task_handle = create_can_receiver_task()) != NULL);
        assert((app.can_transceiver_task_handle = create_can_transceiver_task()) != NULL);
        assert((app.cli_input_task_handle = create_cli_input_task()) != NULL);
        assert((app.cooling_task_handle = create_cooling_task()) != NULL);
        assert((app.dash_task_handle = create_dash_task()) != NULL);
        assert((app.default_task_task_handle = create_default_task_task()) != NULL);
        assert((app.independent_watchdog_task_handle = create_independent_watchdog_task()) != NULL);
        assert((app.light_controller_task_handle = create_light_controller_task()) != NULL);
        assert((app.motor_controller_task_handle = create_motor_controller_task()) != NULL);
        assert((app.sd_card_task_handle = create_sd_card_task()) != NULL);
        assert((app.state_machine_task_handle = create_state_machine_task()) != NULL);
        assert((app.telemetry_task_handle = create_telemetry_task()) != NULL);
    }
    else if(app.startup_mode = CLI_ONLY) {
	// Create and assert each task handle
        assert((app.cli_input_task_handle = create_cli_input_task()) != NULL);
    }
}
