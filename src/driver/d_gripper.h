/**
 * @file  d_gripper.h
 * @brief 夹爪驱动
 *        通过 USART 向舵机发送 PWM 指令
 */
#ifndef _d_gripper_h_
#define _d_gripper_h_

#include "stm32f10x.h"
#include "usart.h"
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef struct Gripper Gripper;
struct Gripper {
// public:
    /**
     * @brief   初始化夹爪
     * @param   self 夹爪对象
     * @param   uart 串口句柄
     * @retval  None
     */
    void(*init)(Gripper* self, usart_t* uart);
    /**
     * @brief   打开夹爪
     * @param   self 夹爪对象
     * @retval  None
     */
    void(*open)(Gripper* self);
    /**
     * @brief   关闭夹爪
     * @param   self 夹爪对象
     * @retval  None
     */
    void(*close)(Gripper* self);
    /**
     * @brief   设置夹爪位置
     * @param   self 夹爪对象
     * @param   pulse 脉宽
     * @retval  None
     */
    void(*set_pos)(Gripper* self, uint16_t pulse);

// private:
    usart_t* _uart_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

Gripper gripper_create(void);

#endif
