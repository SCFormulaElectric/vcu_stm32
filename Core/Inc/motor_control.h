#include <stdint.h>

#define CAN_Command_Message_Lost_Fault 0x00000800;

typedef struct {
    uint8_t apps_fault;
    uint8_t bpps_fault;
} external_motor_fault_t;

typedef struct {
    volatile uint16_t torqueCommand;
    volatile uint16_t lastTorqueCommand;
    volatile uint16_t voltage;
    volatile uint16_t motor_speed;
    volatile uint16_t actual_torque;
    volatile uint16_t inverter_state;
	volatile external_motor_fault_t input_faults;

	//Motor Control CAN Broadcast Messages
	MC_temp_t temp;
	analog_input_voltage_t analog_inputs;
	digital_input_status_t digital_inputs;
	motor_position_information_t position_info;
	current_information_t current_info;
	voltage_information_t voltage_info;
	flux_information_t flux_info;
	internal_voltages_t internal_voltages;
	internal_states_t internal_states;
	fault_codes_t fault_codes;
	torque_timer_info_t torque_timer_info;
	modulation_flux_info_t modulation_flux_info;
	firmware_info_t firmware_info;
	diagnostic_data_t diagnostic_data;
	high_speed_msg_t high_speed_msg;
	torque_capability_t torque_capability;

	//Parameter Response
	parameter_response_t param_response;
} MotorControl_t;

//0x0A0-0x0A2
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
} MC_temp_t;

//0x0A3
typedef struct {
	//voltages
	uint16_t INV_Analog_Input_1; // 10 bits 0-9
	uint16_t INV_Analog_Input_2; // 10-19 
	uint16_t INV_Analog_Input_3; // etc
	uint16_t INV_Analog_Input_4;
	uint16_t INV_Analog_Input_5;
	uint16_t INV_Analog_Input_6;
} analog_input_voltage_t;

//0x0A4
typedef struct {
	uint_8 INV_Digital_Input_1; // Status of Digital Input #1, Forward switch
	uint_8 INV_Digital_Input_2; // Status of Digital Input #2, Reverse switch
	uint_8 INV_Digital_Input_3; // Status of Digital Input #3, Brake switch
	uint_8 INV_Digital_Input_4; // Status of Digital Input #4, Regen Disable switch
	uint_8 INV_Digital_Input_5; // Status of Digital Input #5, Ignition switch
	uint_8 INV_Digital_Input_6; // Status of Digital Input #6, Start switch
	uint_8 INV_Digital_Input_7; // Status of Digital Input #7, Valet Mode
	uint_8 INV_Digital_Input_8; // Status of Digital Input #8
} digital_input_status_t;

//0x0A5
typedef struct {
	uint16_t INV_Motor_Angle_Electrical; // Angle The electrical angle of the motor as read by the encoder or resolver.
	uint16_t INV_Motor_Speed; // Angular Velocity The measured speed of the motor.
	uint16_t INV_Electrical_Output_Frequency; // Frequency The actual electrical frequency of the inverter.
	uint16_t INV_Delta_Resolver_Filtered // Angle This is used in calibration of resolver angle adjustment. The range of this parameter is ±180°. Values between 180° and 360° are shown as negative angle. For example, 270° is equal to -90°, and 190° is equal to -170°
} motor_position_information_t;

//0x0A6
typedef struct {
	uint16_t INV_Phase_A_Current; // Current The measured value of Phase A Current.
	uint16_t INV_Phase_B_Current; // Current The measured value of Phase B Current.
	uint16_t INV_Phase_C_Current; // Current The measured value of Phase C Current.
	uint16_t INV_DC_Bus_Current; // Current The calculated DC Bus current.
} current_information_t;

//0x0A7
typedef struct {
	uint16_t INV_DC_Bus_Voltage; // High Voltage The actual measured value of the DC bus voltage.
	uint16_t INV_Output_Voltage; // High Voltage The calculated value of the output voltage, in peak line-neutral volts.
	uint16_t INV_VAB_Vd_Voltage; // High Voltage Measured value of the voltage between Phase A and Phase B (VAB) when the inverter is disabled. Vd voltage when the inverter is enabled.
	uint16_t INV_VBC_Vq_Voltage; // High Voltage Measured value of the voltage between Phase B and Phase C (VBC) when the inverter is disabled. Vq voltage when the inverter is enabled
} voltage_information_t;

//0x0A8
typedef struct {
	uint16_t INV_Vd_ff; // Flux D-axis voltage feed-forward.
	uint16_t INV_Vq_ff; // Flux Q-axis voltage feed-forward
	uint16_t INV_Id; // Current D-axis current feedback
	uint16_t INV_Iq; // Current Q-axis current feedback
} flux_information_t;

//0x0A9
typedef struct {
	uint16_t INV_Ref_Voltage_1_5; //Low Voltage 1.5V Reference voltage.
	uint16_t INV_Ref_Voltage_2_5; //Low Voltage 2.5V Reference voltage.
	uint16_t INV_Ref_Voltage_5_0; //Low Voltage 5.0V Reference voltage.
	uint16_t INV_Ref_Voltage_12_0; //Low Voltage 12V Reference voltage.
} internal_voltages_t;

//0x0AA
typedef struct {
	uint8_t INV_VSM_State; 	//Byte 0
	//0: VSM Start State 
	//1: Pre-charge Init State  
	//2: Pre-charge Active State  
	//3: Pre-charge Complete State  
	//4: VSM Wait State
	//5: VSM Ready State
	//6: Motor Running State
	//7: Blink Fault Code State
	//14: Shutdown in Process. In key switch mode 1, user has turned the key switch to off position.
	//15: Recycle Power State. User must recycle power when the unit is in this state.

	uint8_t INV_PWM_Frequency; 	//Byte 1 The PWM frequency may change depending on operating conditions.
	uint8_t INV_Inverter_State; //Byte 2
	//0: Power on State
	//1: Stop State
	//2: Open Loop State
	//3: Closed Loop State
	//4: Wait State
	//5– 7: Internal States
	//8: Idle Run State
	//9: Idle Stop State
	//10– 12: Internal States

	uint8_t Relay States;		//Byte 3
	//Bit 0: INV_Relay_1_Status (1=active)
	//Bit 1: Relay 2 Status
	//Bit 2: Relay 3 Status
	//Bit 3: Relay 4 Status
	//Bit 4: Relay 5 Status
	//Bit 5: Relay 6 Status

	uint_8 INV_Inverter_Run_Mode; //Byte 4 bit 0, 0 = Torque Mode 1 = Speed Mode
	uint_8 INV_Self_Sensing_Assist_Enable; //Byte 4 bit 1, 0 = Disabled, 1 = Enabled
	uint8_t INV_ASC_State; //Byte 4 bit 2-4, Current Active Short Circuit State:
	//0: ASC_Disabled
	//1: ASC_Enabled
	//2: ASC_Pending
	//3: ASC_Delay
	//4: ASC_Active
	//5: ASC_Complete
	//6: ASC_Blocked
	//7: ASC_Suppressed

	uint8_t INV_Inverter_Discharge_State; //Byte 4 bit 5-7, Current Inverter Active Discharge State:
	//0: Disabled
	//1: Enabled, waiting
	//2: Speed Check
	//3: Active
	//4: Completed

	uint_8 INV_Inverter_Command_Mode; // Byte 5 bit 0, 0 = CAN Mode, 1 = VSM Mode
	uint8_t INV_Rolling_Counter;    // Byte 5 bits 4-7, The value of the currently expected Rolling Counter value.
	
	//Byte 6
	uint_8 INV_Inverter_Enable_State; // 0 = disabled, 1 = enabled
	uint_8 INV_Burst_Model_Mode;      // 0 = Stall, 1 = High Speed
	uint_8 INV_BMS_Limiting_Regen_Torque; // 0 = Not Limited, 1 = Limited
	uint_8 INV_Limit_Motor_Temp_Derate;   // 0 = Not Limited, 1 = Limited
	uint_8 INV_Limit_Hot_Spot_Motor;      // 0 = Not Limited, 1 = Limited
	uint_8 INV_Key_Switch_Start_Status;   // 0 = Not Active, 1 = Active
	uint_8 INV_Inverter_Enable_Lockout //0 = Inverter can be enabled, 1 = Inverter cannot be enabled

	// Byte 7
	uint_8 INV_BMS_Active;                  // 7-Bit 1: 0 = BMS Message is not being received, 1 = BMS Message is being received
	uint_8 INV_BMS_Limiting_Motor_Torque;   // 7-Bit 2: Indicates if motoring torque is being limited
	uint_8 INV_Limit_Max_Speed;             // 7-Bit 3: Indicates if torque is being limited due to the motor speed exceeding the maximum
	uint_8 INV_Limit_Hot_Spot_Inverter;     // 7-Bit 4: Indicates if the current is limited due to the inverter hot spot temperature regulator
	uint_8 INV_Low_Speed_Limiting;          // 7-Bit 5: Indicates if the current is limited due to low speed current limiting
	uint_8 INV_Limit_Coolant_Derating;      // 7-Bit 6: Indicates if the current is limited due to excessive coolant temperature
	uint_8 INV_Limit_Stall_Burst_Model;     // 7-Bit 7: Indicates if the current is being limited due to the stall burst model
} internal_states_t;

// 0x0AB: Fault Codes
typedef struct {
	uint16_t INV_Post_Fault_Lo; // Bytes 0-1: Each bit represents a POST fault
	uint16_t INV_Post_Fault_Hi; // Bytes 2-3: Each bit represents a POST fault
	uint16_t INV_Run_Fault_Lo;  // Bytes 4-5: Each bit represents a Run fault
	uint16_t INV_Run_Fault_Hi;  // Bytes 6-7: Each bit represents a Run fault
} fault_codes_t;

// 0x0AC: Torque & Timer Information
typedef struct {
	uint16_t INV_Commanded_Torque;   // Bytes 0-1: The commanded torque
	uint16_t INV_Torque_Feedback;    // Bytes 2-3: The estimated motor torque
	uint32_t INV_Power_On_Timer;     // Bytes 4-7: Power-on timer (Counts x 0.003 sec)
} torque_timer_info_t;

// 0x0AD: Modulation Index & Flux Weakening Output Information
typedef struct {
	uint16_t Modulation_Index;            // Bytes 0-1: Modulation index (divide by 100 for actual value)
	uint16_t Flux_Weakening_Output;       // Bytes 2-3: Flux weakening output current/voltage
	uint16_t Id_Command_Current_1;        // Bytes 4-5: Commanded D-axis current
	uint16_t Id_Command_Current_2;        // Bytes 6-7: Commanded D-axis current (duplicate field as per table)
} modulation_flux_info_t;

// 0x0AE: Firmware Information
typedef struct {
	uint16_t EEPROM_Version_Project_Code; // Bytes 0-1: EEPROM version/project code
	uint16_t Software_Version;            // Bytes 2-3: Software version (major/minor)
	uint16_t Date_Code_MMDD;              // Bytes 4-5: Date code (mmdd)
	uint16_t Date_Code_YYYY;              // Bytes 6-7: Date code (yyyy)
} firmware_info_t;

// 0x0AF: Diagnostic Data (details not specified)
typedef struct {
	uint8_t data[8]; // Placeholder for diagnostic data, see section 6.8 for details
} diagnostic_data_t;

// 0x0B0: High Speed Message
typedef struct {
	uint16_t Torque_Command;      // Bytes 0-1: Commanded torque
	uint16_t Torque_Feedback;     // Bytes 2-3: Estimated motor torque
	uint16_t Motor_Speed;         // Bytes 4-5: Measured motor speed
	uint16_t DC_Bus_Voltage;      // Bytes 6-7: Measured DC bus voltage
} high_speed_msg_t;

// 0x0B1: Torque Capability
typedef struct {
	uint16_t Motor_Torque_Available; // Bytes 0-1: Available motoring torque
	uint16_t Regen_Torque_Available; // Bytes 2-3: Available regen torque
	uint16_t reserved1;              // Bytes 4-5: NA
	uint16_t reserved2;              // Bytes 6-7: NA
} torque_capability_t;

//0x0C2
typedef struct {
	uint16_t Parameter_Address;
	uint8_t Write_Success;
	uint16_t Data;
} parameter_response_t;

uint8_t is_fault(const fault_codes_t *faults);
