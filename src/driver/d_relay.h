/**
 * @file    d_relay.h
 * @brief   继电器驱动
 */
#ifndef _d_relay_h_
#define _d_relay_h_

#include "stm32f10x.h"

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef struct {
    uint32_t rcc_mask;
    uint8_t rcc_bus;       /* 1 = APB1, 2 = APB2 */
    GPIO_TypeDef* port;
    uint16_t pin_a;
    uint16_t pin_b;
} relay_cfg_t;

typedef enum {
    RelayDirStop = 0,
    RelayDirA,
    RelayDirB
} RelayDir_e;

typedef struct Relay Relay;
struct Relay {
// public:
    /**
     * @brief   初始化电机
     * @param   self 电机对象
     * @retval  None
     */
    void (*init)(Relay* self, const relay_cfg_t* cfg);
    /**
     * @brief   设置电机方向
     * @param   self 电机对象
     * @param   dir 方向
     * @retval  None
     */
    void (*set_dir)(Relay* self, RelayDir_e dir);
    /**
     * @brief   停止电机
     * @param   self 电机对象
     * @retval  None
     */
    void (*stop)(Relay* self);

// private:
    const relay_cfg_t* _cfg_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

Relay relay_create(void);

#endif
