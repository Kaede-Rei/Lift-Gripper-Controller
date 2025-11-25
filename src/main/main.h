#ifndef _main_h_
#define _main_h_

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "UART.h"
#include "Delay.h"
#include "Encoder.h"
#include "PID.h"
#include "TIM.h"

// 硬件参数
#define MOTOR_VOLTAGE 24        // 电源电压24V
#define ENCODER_PPR 1000        // 霍尔传感器每行程脉冲数

// 校准参数
#define ACTUAL_MAX_PULSE       15518     // 实际测量的最大脉冲数（1000mm时）
#define ACTUAL_MAX_MM          1000      // 最大行程1000mm
#define ACTUAL_PULSE_PER_MM    15.518f   // 实际脉冲/mm比 (15518/1000)

#define MIN_STROKE_MM          50        // 最小行程50mm
#define MAX_STROKE_MM          1000      // 最大行程1000mm

#define MIN_STROKE_PULSE       (int)(MIN_STROKE_MM * ACTUAL_PULSE_PER_MM)    // 776脉冲
#define MAX_STROKE_PULSE       (int)(MAX_STROKE_MM * ACTUAL_PULSE_PER_MM)    // 15518脉冲

// 控制参数
#define DESIRED_SPEED 20        // 期望速度20mm/s（对应规格书S5/S6档）
// GPIO定义
#define MOTOR_UP_PIN    GPIO_Pin_0
#define MOTOR_DOWN_PIN  GPIO_Pin_1
#define MOTOR_GPIO_PORT GPIOB

#endif
