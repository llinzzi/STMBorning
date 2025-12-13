/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdarg.h>
#include <stdio.h>

// 串口接收缓冲区
#define UART_RX_BUFFER_SIZE 128
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t uart_rx_index = 0;
static uint8_t uart_rx_byte;

// 舵机参数存储（使用头文件中定义的类型）
ServoConfig_t servo_configs[2] = {
  {0, 180},  // 舵机1 (PA7): 默认0度到0度, 目标0度到180度
  {0, 90}    // 舵机2 (PB1): 默认0度到0度, 目标0度到90度
};

// 外部变量声明
extern volatile uint8_t current_angle;
extern volatile uint8_t current_angle2;
extern volatile uint8_t motion_stage;
extern volatile uint32_t last_update_tick;
extern volatile uint32_t last_print_tick;

// 外部函数声明
extern void Servo_SetAngle(uint8_t angle);
extern void Servo2_SetAngle(uint8_t angle);
extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim3;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  // 启动串口接收中断
  HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9 [PA11]     ------> USART1_TX
    PA10 [PA12]     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */
  // 启用USART1中断
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9 [PA11]     ------> USART1_TX
    PA10 [PA12]     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  串口格式化输出
  */
void UART_Printf(const char *format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/**
  * @brief  打印舵机状态
  */
void UART_PrintServoStatus(void)
{
  uint32_t current_tick = HAL_GetTick();
  uint32_t elapsed = (current_tick - last_update_tick);
  float speed = (elapsed > 0) ? (1000.0f / elapsed) : 0;  // 度/秒
  
  UART_Printf("\r\n===== 舵机状态 =====\r\n");
  UART_Printf("舵机1 (PA7):\r\n");
  UART_Printf("  启动角度: %d度\r\n", servo_configs[0].start_angle);
  UART_Printf("  目标角度: %d度\r\n", servo_configs[0].target_angle);
  UART_Printf("  当前角度: %d度\r\n", current_angle);
  UART_Printf("  速度: %.1f 度/秒\r\n", speed);
  
  UART_Printf("\r\n舵机2 (PB1):\r\n");
  UART_Printf("  启动角度: %d度\r\n", servo_configs[1].start_angle);
  UART_Printf("  目标角度: %d度\r\n", servo_configs[1].target_angle);
  UART_Printf("  当前角度: %d度\r\n", current_angle2);
  UART_Printf("  速度: %.1f 度/秒\r\n", speed);
  
  UART_Printf("\r\n当前阶段: %d\r\n", motion_stage);
  UART_Printf("==================\r\n\r\n");
}

/**
  * @brief  处理串口命令
  * 命令格式:
  *   status - 查看舵机状态
  *   set <servo> <start> <target> - 设置舵机参数 (servo: 1或2, start/target: 0-180)
  *   run <servo> <start> <target> - 设置并立即执行
  *   help - 显示帮助信息
  */
void UART_ProcessCommand(void)
{
  char cmd[20];
  int servo_id, start_angle, target_angle;
  
  // 解析命令
  if (strncmp((char*)uart_rx_buffer, "status", 6) == 0) {
    UART_PrintServoStatus();
  }
  else if (sscanf((char*)uart_rx_buffer, "set %d %d %d", &servo_id, &start_angle, &target_angle) == 3) {
    if (servo_id >= 1 && servo_id <= 2 && start_angle >= 0 && start_angle <= 180 && target_angle >= 0 && target_angle <= 180) {
      servo_configs[servo_id - 1].start_angle = start_angle;
      servo_configs[servo_id - 1].target_angle = target_angle;
      UART_Printf("舵机%d 设置成功: 启动=%d度, 目标=%d度\r\n", servo_id, start_angle, target_angle);
    } else {
      UART_Printf("错误: 参数超出范围! (舵机:1-2, 角度:0-180)\r\n");
    }
  }
  else if (sscanf((char*)uart_rx_buffer, "run %d %d %d", &servo_id, &start_angle, &target_angle) == 3) {
    if (servo_id >= 1 && servo_id <= 2 && start_angle >= 0 && start_angle <= 180 && target_angle >= 0 && target_angle <= 180) {
      servo_configs[servo_id - 1].start_angle = start_angle;
      servo_configs[servo_id - 1].target_angle = target_angle;
      UART_Printf("舵机%d 开始运动: %d度 -> %d度\r\n", servo_id, start_angle, target_angle);
      
      // 立即执行
      if (motion_stage == 0) {  // 只有空闲时才执行
        if (servo_id == 1) {
          current_angle = start_angle;
          Servo_SetAngle(current_angle);
          HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
          // 设置为单独运动模式 (跳过音乐，直接运动)
          motion_stage = 2;  // PA7运动阶段
        } else {
          current_angle2 = start_angle;
          Servo2_SetAngle(current_angle2);
          HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
          motion_stage = 3;  // PB1运动阶段
        }
      } else {
        UART_Printf("错误: 舵机正在运动中，请稍后再试!\r\n");
      }
    } else {
      UART_Printf("错误: 参数超出范围! (舵机:1-2, 角度:0-180)\r\n");
    }
  }
  else if (strncmp((char*)uart_rx_buffer, "help", 4) == 0) {
    UART_Printf("\r\n===== 串口命令帮助 =====\r\n");
    UART_Printf("命令格式:\r\n");
    UART_Printf("  status                      - 查看舵机状态\r\n");
    UART_Printf("  set <servo> <start> <target> - 设置舵机参数\r\n");
    UART_Printf("  run <servo> <start> <target> - 设置并立即执行\r\n");
    UART_Printf("  help                        - 显示帮助信息\r\n");
    UART_Printf("\r\n参数说明:\r\n");
    UART_Printf("  servo: 舵机编号 (1=PA7, 2=PB1)\r\n");
    UART_Printf("  start: 启动角度 (0-180)\r\n");
    UART_Printf("  target: 目标角度 (0-180)\r\n");
    UART_Printf("\r\n示例:\r\n");
    UART_Printf("  set 1 0 90     - 设置舵机1从0度到9０度\r\n");
    UART_Printf("  run 2 45 135   - 舵机2从45度立即移动到135度\r\n");
    UART_Printf("====================\r\n\r\n");
  }
  else {
    UART_Printf("未知命令: %s\r\n", uart_rx_buffer);
    UART_Printf("输入 'help' 查看帮助信息\r\n");
  }
}

/**
  * @brief  串口接收完成回调
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    if (uart_rx_byte == '\r' || uart_rx_byte == '\n') {
      if (uart_rx_index > 0) {
        uart_rx_buffer[uart_rx_index] = '\0';
        UART_ProcessCommand();
        uart_rx_index = 0;
      }
    } else if (uart_rx_index < UART_RX_BUFFER_SIZE - 1) {
      uart_rx_buffer[uart_rx_index++] = uart_rx_byte;
    }
    // 继续接收下一个字节
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
  }
}

/* USER CODE END 1 */
