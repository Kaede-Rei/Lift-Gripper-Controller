/**
 * @file  main.h
 * @brief 项目全局配置头文件 — 引脚定义、参数宏
 */
#ifndef _main_h_
#define _main_h_

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ========================= 通信参数 ========================= */
#define USART1_BAUD             115200
#define USART2_BAUD             115200

/* ========================= 定时器节拍 ========================= */
#define TICK_PERIOD_MS          10

#endif
