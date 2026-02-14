/**
 * @file  s_comms.h
 * @brief 无线串口通信协议服务
 *        通过 USART 接收工控机指令, 控制升降台与夹爪
 */
#ifndef _s_comms_h_
#define _s_comms_h_

#include "usart.h"
#include "s_lift_control.h"
#include "d_gripper.h"
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

#define COMMS_CMD_BUF_SIZE  64

typedef struct Comms Comms;
struct Comms {
// public:
    /**
     * @brief   初始化通信模块
     * @param   self 通信对象
     * @param   uart 串口句柄
     * @param   lift 升降机对象
     * @param   grip 夹爪对象
     * @retval  None
     */
    void (*init)(Comms* self, usart_t* uart, LiftControl* lift, Gripper* grip);
    /**
     * @brief   通信处理循环
     * @param   self 通信对象
     * @retval  None
     */
    void (*process)(Comms* self);

// private:
    usart_t* _uart_;
    LiftControl* _lift_;
    Gripper* _grip_;
    char _cmd_buf_[COMMS_CMD_BUF_SIZE];
    uint8_t _cmd_idx_;
    uint8_t _parsing_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

Comms comms_create(void);

#endif
