#include "motor_control.h"
#include "canbus.h"
#define BPPS_PRIO 13
#define APPS_PRIO 13
#define IWT_PRIO  13
#define MCT_PRIO  10
#define CAN_receiver_task_PRIO 10
#define Cooling_PRIO 10
#define LC_PRIO 10
#define can_transmitter_task_PRIO 10
#define telemetry_task_PRIO 6
#define state_machine_PRIO 6
#define dash_PRIO 6
#define sd_card_PRIO 6
#define cli_input_PRIO 6
#define motor_control_interval 25 //10-50 milisecond is required.
#define MAX_TORQUE 300 

enum StartUpMode {
    ALL,
    CLI_ONLY
};

typedef enum {
    S0,
	S1,
	S2
} system_state_t;

extern volatile system_state_t extern_curr_state;

typedef struct{
	// Task handles
	TaskHandle_t throttle_task_handle;
	TaskHandle_t brake_pedal_plausibility_check_task_handle;
	TaskHandle_t can_receiver_task_handle;
	TaskHandle_t can_transmitter_task_handle;
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
    StartUpMode  startup_mode;

	car_data_t car_data;
	canbus_t     canbus;
	QueueHandle_t can_tx_queue;
	QueueHandle_t can_rx_queue;

	MotorControl_t motorControl;

	uint16_t throttle_level;
	uint16_t brake_level;

} app_data;

typedef struct {
	canbus_t canbus;
	MotorControl_t motor_control;
} car_data_t

void create_app();
uint16_t getThrottle();