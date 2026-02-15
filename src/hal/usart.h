/**
 * @file    usart.h
 * @brief   USART HAL — 配置表驱动
 *          支持 USART1 ~ USART3 (默认引脚, 无重映射)
 */
#ifndef _usart_h_
#define _usart_h_

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

/// @brief USART RX 环形缓冲区大小
#define USART_RX_BUF_SIZE  128

/**
 * @brief USART ID 枚举
 */
typedef enum {
    USART_1 = 0,
    USART_2,
    USART_3,
    USART_COUNT
} usart_id_e;

/**
 * @brief USART 配置表
 */
typedef struct {
    usart_id_e id;          // USART ID
    uint32_t baudrate;      // 波特率
    uint8_t enable_rx_irq;  // 是否启用 RX 中断
    uint8_t nvic_preempt;   // 抢占优先级
    uint8_t nvic_sub;       // 子优先级
} usart_cfg_t;

/**
 * @brief USART 运行时句柄
 */
typedef struct {
    const usart_cfg_t* cfg;
    uint8_t  rx_buf[USART_RX_BUF_SIZE];
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
} usart_t;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void usart_init(usart_t* handle, const usart_cfg_t* cfg);
void usart_send_byte(usart_t* handle, uint8_t byte);
void usart_send_string(usart_t* handle, const char* str);
bool usart_read_byte(usart_t* handle, uint8_t* out);

#endif
