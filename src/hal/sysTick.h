/**
 * @file    systick.h
 * @brief   SysTick 1 ms 心跳计时
 */
#ifndef _sysTick_h_
#define _sysTick_h_

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef uint32_t ms_t;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void systick_init(void);
ms_t systick_get_ms(void);
ms_t systick_get_s(void);
bool systick_is_timeout(ms_t start, ms_t timeout_ms);

#endif
