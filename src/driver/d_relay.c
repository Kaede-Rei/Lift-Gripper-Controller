/**
 * @file    d_relay.c
 * @brief   继电器驱动实现
 */
#include "d_relay.h"

// ! ========================= 变 量 声 明 ========================= ! //



// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _init(Relay* self, const relay_cfg_t* cfg);
static void _set_dir(Relay* self, RelayDir_e dir);
static void _stop(Relay* self);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 Relay 对象
 * @param   None
 * @retval  Relay 对象
 */
Relay relay_create(void) {
    Relay obj;
    obj.init = _init;
    obj.set_dir = _set_dir;
    obj.stop = _stop;
    return obj;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   初始化电机
 * @param   self 电机对象
 * @retval  None
 */
static void _init(Relay* self, const relay_cfg_t* cfg) {
    switch(cfg->rcc_bus) {
        case 1:
            RCC_APB1PeriphClockCmd(cfg->rcc_mask, ENABLE);
            break;
        case 2:
            RCC_APB2PeriphClockCmd(cfg->rcc_mask, ENABLE);
            break;
        default:
            break;
    }

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = cfg->pin_a | cfg->pin_b;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(cfg->port, &gpio);

    /* 默认停止 */
    GPIO_ResetBits(cfg->port, cfg->pin_a);
    GPIO_ResetBits(cfg->port, cfg->pin_b);

    self->_cfg_ = cfg;
}

/**
 * @brief   设置电机方向
 * @param   self 电机对象
 * @param   dir 方向
 * @retval  None
 */
static void _set_dir(Relay* self, RelayDir_e dir) {
    switch(dir) {
        case RelayDirA:
            GPIO_SetBits(self->_cfg_->port, self->_cfg_->pin_a);
            GPIO_ResetBits(self->_cfg_->port, self->_cfg_->pin_b);
            break;
        case RelayDirB:
            GPIO_ResetBits(self->_cfg_->port, self->_cfg_->pin_a);
            GPIO_SetBits(self->_cfg_->port, self->_cfg_->pin_b);
            break;
        case RelayDirStop:
        default:
            GPIO_ResetBits(self->_cfg_->port, self->_cfg_->pin_a);
            GPIO_ResetBits(self->_cfg_->port, self->_cfg_->pin_b);
            break;
    }
}

/**
 * @brief   停止电机
 * @param   self 电机对象
 * @retval  None
 */
static void _stop(Relay* self) {
    GPIO_ResetBits(self->_cfg_->port, self->_cfg_->pin_a);
    GPIO_ResetBits(self->_cfg_->port, self->_cfg_->pin_b);
}
