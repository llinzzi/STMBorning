/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim14;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM17_Init(void);
void MX_TIM3_Init(void);
void MX_TIM14_Init(void);

/* USER CODE BEGIN Prototypes */
void Servo_SetAngle(uint8_t angle);
void Servo2_SetAngle(uint8_t angle);  // PB1舵机控制
void Buzzer_PlayAngryMusic(void);     // 播放生气的音乐
void Buzzer_PlayHappyMusic(void);     // 播放愉快的音乐
void Buzzer_SetFrequency(uint16_t freq);  // 设置蜂鸣器频率
void Buzzer_Stop(void);               // 停止蜂鸣器
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /*__ TIM_H__ */
