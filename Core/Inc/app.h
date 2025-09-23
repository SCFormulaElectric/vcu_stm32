#define BPPS_PRIO 13
#define APPS_PRIO 13
#define IWT_PRIO  13
#define MCT_PRIO  10
#define CAN_receiver_task_PRIO 10
#define Cooling_PRIO 10
#define LC_PRIO10
#define CAN_transceiver_task_PRIO 10
#define telemetry_task_PRIO 6
#define state_machine_PRIO 6
#define dash_PRIO 6
#define sd_card_PRIO 6
#define cli_input_PRIO 6

#define motor_control_FREQ 40 //10-50 milisecond is required. 1000/40 = 25
#define MAX_TORQUE 300 
enum StartUpMode {
    ALL,
    CLI_ONLY
};

typedef struct {
	//Temperature 1
	uint16_t INV_Module_A_Temp;     	// 0-1
	uint16_t INV_Module_B_Temp;			// 2-3
	uint16_t INV_Module_C_Temp;			// 4-5
	uint16_t INV_GDB_Temp;				// 6-7
	//Temperature 2
	uint16_t INV_Control_Board_Temp;	// 0-1
	uint16_t INV_RTD1_Temperature;		// 2-3
	uint16_t INV_RTD2_Temperature;		// 4-5
	uint16_t INV_Hot_Spot_Temp_Motor;	// 6-7
	//Temperature 3
	uint16_t INV_Coolant_Temp;			// 0-1
	uint16_t INV_Hot_Spot_Temp_Inverter;// 2-3
	uint16_t INV_Motor_Temp;			// 4-5
	uint16_t INV_Torque_Shudder;		// 6-7
} MC_temp_t


typedef z
typedef struct {
	bool enabled;
    bool fault;
	uint8_t  opState; // speed, torque etc?
    uint16_t torqueCommand;
    uint16_t lastTorqueCommand;
    uint16_t voltage;
    uint16_t motor_speed;
    uint16_t actual_torque;
    uint16_t inverter_state;
	MC_temp_t temp;
	//etc
} MotorControl_t;

typedef struct {
    CAN_HandleTypeDef *hcan;
    CAN_TxHeaderTypeDef *tx_header;
    CAN_TxHeaderTypeDef *rx_header;
    uint8_t  rx_packet[8];
    uint8_t  tx_packet[8];
    uint32_t tx_mailbox;
} canbus_t;

typedef struct {
    uint32_t tx_id;
    uint8_t  tx_packet[8];
} can_message_t;

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
    StartUpMode  startup_mode = ALL;

	canbus_t     canbus;
	QueueHandle_t can_tx_queue;
	QueueHandle_t can_rx_queue;

	MotorControl_t motorControl;
} app_data;

void create_app();

uint16_t getThrottle();