/**
 * @file    s_wireless_comms.h
 * @brief   无线通信服务接口
 *          升降台升降 + 夹爪开合
 */
#ifndef _s_wireless_comms_h_
#define _s_wireless_comms_h_

#include "usart.h"
#include "d_relay.h"
#include "d_gripper.h"
#include "d_encoder.h"

#include <stdbool.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

extern float lift_target_pos_mm;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void s_wireless_comms_init(usart_t* usart, Relay* lift_relay, Gripper* gripper);
bool s_wireless_comms_process(void);

#endif
