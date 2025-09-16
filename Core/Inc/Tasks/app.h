#define BPPS_PRIO 13
#define APPS_PRIO 13
#define IWT_PRIO  13
#define MCT_PRIO  10
#define CAN_RT_PRIORITY 10
#define Cooling_PRIO 10
#define LC_PRIORITY 10
#define CAN_transceiver_task_PRIORITY 10
#define telemetry_task_PRIORITY 6
#define state_machine_PRIORITY 6
#define dash_PRIORITY 6
#define sd_card_PRIORI5Y 6
#define cli_input_PRIORIT5 6

enum StartUpMode {
    ALL,
    CLI_ONLY
};
typedef struct{
	// Task handles
	TaskHandle_t apps_implausibility_check_task_handle;
	TaskHandle_t brake_pedal_plausibility_check_task_handle;
	TaskHandle_t can_receiver_task_handle;
	TaskHandle_t can_transceiver_task_handle;
	TaskHandle_t cli_input_task_handle;
	TaskHandle_t cooling_task_handle;
	TaskHandle_t dash_task_handle;
	TaskHandle_t default_task_task_handle;
	TaskHandle_t independent_watchdog_task_handle;
	TaskHandle_t light_controller_task_handle;
	TaskHandle_t motor_controller_task_handle;
	TaskHandle_t sd_card_task_handle;
	TaskHandle_t state_machine_task_handle;
	TaskHandle_t telemetry_task_handle;
    StartUpMode startup_mode = ALL;
    
} app_data;

void create_app();