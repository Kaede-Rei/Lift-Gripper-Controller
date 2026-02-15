/**
 * @file    can.h
 * @brief   CAN HAL — 配置表驱动
 *          支持 CAN1 (STM32F103, PA11-RX  PA12-TX)
 */
#ifndef _can_h_
#define _can_h_

#include "stm32f10x.h"
#include <stdbool.h>
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef void(*can_rx_cb_t)(CanRxMsg* msg);

/**
 * @brief CAN ID 枚举
 */
typedef enum {
    CAN_1 = 0,
    CAN_COUNT
} can_id_e;

/**
 * @brief CAN 工作模式
 */
typedef enum {
    CAN_MODE_NORMAL = 0,
    CAN_MODE_LOOPBACK,
    CAN_MODE_SILENT,
    CAN_MODE_SILENT_LOOPBACK
} can_mode_e;

/**
 * @brief CAN 配置表
 */
typedef struct {
    can_id_e id;                // CAN ID
    CAN_TypeDef* periph;        // CAN 外设
    can_mode_e mode;            // 工作模式
    uint8_t sjw;                // CAN_SJW_xtq
    uint8_t bs1;                // CAN_BS1_xtq
    uint8_t bs2;                // CAN_BS2_xtq
    uint16_t prescaler;         // 分频系数
    uint8_t nvic_preempt;       // 抢占优先级
    uint8_t nvic_sub;           // 子优先级
} can_cfg_t;

/**
 * @brief CAN 运行时句柄
 */
typedef struct {
    const can_cfg_t* cfg;
    can_rx_cb_t rx_cb;
} can_t;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void can_init(can_t* handle, const can_cfg_t* cfg);
bool can_send(can_t* handle, uint16_t std_id, const uint8_t* data, uint8_t len);
void can_set_rx_cb(can_t* handle, can_rx_cb_t cb);

#endif
