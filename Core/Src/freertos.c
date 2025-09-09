/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

//Brake Pedal Plausibility Check (BPPS)
osThreadId_t BPPS;
const osThreadAttr_t BPPS_attributes = {
  .name = "brakePlausibilityCheck",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

//APPS Implausibility Check
osThreadId_t APPS;
const osThreadAttr_t APPS_attributes = {
  .name = "throttlePlausibilityCheck",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

//Independent watchdog (IWDG)
osThreadId_t IDWG;
const osThreadAttr_t IDWG_attributes = {
  .name = "watchDog",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

//Motor Controller
osThreadId_t motorController;
const osThreadAttr_t motorController_attributes = {
  .name = "motorController",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

//CAN Receiver
osThreadId_t canRX;
const osThreadAttr_t canRX_attributes = {
  .name = "canRX",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

//Cooling
osThreadId_t cooling;
const osThreadAttr_t canRX_attributes = {
  .name = "cooling",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

//Light Controller
osThreadId_t lightController;
const osThreadAttr_t lightController_attributes = {
  .name = "lightController",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

//CAN transceiver
osThreadId_t canTX;
const osThreadAttr_t canTX_attributes = {
  .name = "canTX",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

//Telemetry
osThreadId_t telemetry;
const osThreadAttr_t telemetry_attributes = {
  .name = "telemtry",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

//State Machine
osThreadId_t stateMachine;
const osThreadAttr_t stateMachine_attributes = {
  .name = "stateMachine",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

//Dash
osThreadId_t dash;
const osThreadAttr_t dash_attributes = {
  .name = "dash",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

//SD Card
osThreadId_t sdCard;
const osThreadAttr_t sdCard_attributes = {
  .name = "sdCard",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

//CLI Input
osThreadId_t commandLine;
const osThreadAttr_t commandLine_attributes = {
  .name = "commandLine",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

