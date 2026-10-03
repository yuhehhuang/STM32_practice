/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "aht20.h"
#include "stdio.h"
#include <string.h>
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
#include <stdbool.h>

#include "oled.h"
volatile bool is_overheat = false;
/* USER CODE END Variables */
/* Definitions for LED_Task */
osThreadId_t LED_TaskHandle;
const osThreadAttr_t LED_Task_attributes = {
  .name = "LED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for OLED_Task */
osThreadId_t OLED_TaskHandle;
const osThreadAttr_t OLED_Task_attributes = {
  .name = "OLED_Task",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Tem_Task */
osThreadId_t Tem_TaskHandle;
const osThreadAttr_t Tem_Task_attributes = {
  .name = "Tem_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for OLED_Queue */
osMessageQueueId_t OLED_QueueHandle;
const osMessageQueueAttr_t OLED_Queue_attributes = {
  .name = "OLED_Queue"
};
/* Definitions for I2C_Mutex */
osMutexId_t I2C_MutexHandle;
const osMutexAttr_t I2C_Mutex_attributes = {
  .name = "I2C_Mutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartLEDTask(void *argument);
void StartOLEDTask(void *argument);
void StartTemTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of I2C_Mutex */
  I2C_MutexHandle = osMutexNew(&I2C_Mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of OLED_Queue */
  OLED_QueueHandle = osMessageQueueNew (16, sizeof(WeatherData), &OLED_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of LED_Task */
  LED_TaskHandle = osThreadNew(StartLEDTask, NULL, &LED_Task_attributes);

  /* creation of OLED_Task */
  OLED_TaskHandle = osThreadNew(StartOLEDTask, NULL, &OLED_Task_attributes);

  /* creation of Tem_Task */
  Tem_TaskHandle = osThreadNew(StartTemTask, NULL, &Tem_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartLEDTask */
/**
  * @brief  Function implementing the LED_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartLEDTask */
void StartLEDTask(void *argument)
{
  /* USER CODE BEGIN StartLEDTask */
  //初始化:確保LED一開始都是關閉的
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  /* Infinite loop */
  for(;;)
  {
    if (is_overheat)
    {
      //溫度異常，關閉綠燈，紅燈toggle(100ms)
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
      HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
      osDelay(100);
    }
    else
    {
      //正常狀態，關閉紅燈，綠燈慢慢閃爍(500ms)
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
      osDelay(500);
    }
  }
  /* USER CODE END StartLEDTask */
}

/* USER CODE BEGIN Header_StartOLEDTask */
/**
* @brief Function implementing the OLED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartOLEDTask */
void StartOLEDTask(void *argument)
{
  /* USER CODE BEGIN StartOLEDTask */
  WeatherData receiveData;
  char tempStr[20];
  char humStr[20];

  osSemaphoreAcquire(I2C_MutexHandle,osWaitForever);
  OLED_Init();
  osSemaphoreRelease(I2C_MutexHandle);


  /* Infinite loop */
  for(;;)
  {
    if (osMessageQueueGet(OLED_QueueHandle,&receiveData,NULL,osWaitForever) == osOK)
    {
      sprintf(tempStr,"Temp: %.1f C",receiveData.temperature);
      sprintf(humStr,"Humid: %.1f %%",receiveData.humidity);

      osSemaphoreAcquire(I2C_MutexHandle,osWaitForever);

      OLED_NewFrame();//開始畫新圖

      OLED_PrintString(0,0,tempStr,&font16x16,OLED_COLOR_NORMAL);
      OLED_PrintString(0,18,humStr,&font16x16,OLED_COLOR_NORMAL);
      OLED_ShowFrame();

      osSemaphoreRelease(I2C_MutexHandle);
    }
  }
  /* USER CODE END StartOLEDTask */
}

/* USER CODE BEGIN Header_StartTemTask */
/**
* @brief Function implementing the Tem_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTemTask */
void StartTemTask(void *argument)
{
  /* USER CODE BEGIN StartTemTask */
  AHT20_Init();

  WeatherData data;
  /* Infinite loop */
  for(;;)
  {
    AHT20_Read(&data.temperature,&data.humidity);

    if (data.temperature > 27.5f) {
      is_overheat = true;
    } else {
      is_overheat = false;
    }


    osMessageQueuePut(OLED_QueueHandle, &data, 0, 0);
    osDelay(2000);
  }
  /* USER CODE END StartTemTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

