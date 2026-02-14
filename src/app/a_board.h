/**
 * @file  a_board.h
 * @brief 板级初始化 — 汇总所有模块, 声明全局实例
 */
#ifndef _a_board_h_
#define _a_board_h_

#include "can.h"
#include "usart.h"
#include "timer.h"
#include "systick.h"
#include "dwt.h"
#include "d_encoder.h"
#include "d_lift_motor.h"
#include "d_gripper.h"
#include "s_comms.h"
#include "s_lift_control.h"
#include "s_delay.h"
#include "s_log.h"
#include "s_pid.h"

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

extern can_t can;
extern usart_t usart1;
extern usart_t usart2;
extern tim_t tick;

extern Encoder encoder;
extern LiftMotor lift_motor;
extern Gripper gripper;
extern Comms comms;
extern LiftControl lift_ctrl;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void a_board_init(void);

#endif
