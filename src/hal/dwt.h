/**
 * @file    dwt.h
 * @brief   DWT 微秒级计时器
 */
#ifndef _dwt_h_
#define _dwt_h_

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

#define CPU_FREQ_MHZ  72

#ifndef DWT_BASE
#define DWT_BASE  (0xE0001000UL)
#define DWT       ((DWT_Type *)DWT_BASE)
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t CYCCNT;
    volatile uint32_t CPICNT;
    volatile uint32_t EXCCNT;
    volatile uint32_t SLEEPCNT;
    volatile uint32_t LSUCNT;
    volatile uint32_t FOLDCNT;
    volatile uint32_t PCSR;
} DWT_Type;
#endif

#ifndef DWT_CTRL_CYCCNTENA_Msk
#define DWT_CTRL_CYCCNTENA_Msk  (1UL << 0)
#endif

#ifndef CoreDebug_DEMCR_TRCENA_Msk
#define CoreDebug_DEMCR_TRCENA_Msk  (1UL << 24)
#endif

typedef uint32_t us_t;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void dwt_init(void);
us_t dwt_get_us(void);
bool dwt_is_timeout(us_t start, us_t timeout_us);

#endif
