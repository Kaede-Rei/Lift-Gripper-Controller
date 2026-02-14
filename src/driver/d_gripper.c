/**
 * @file  d_gripper.c
 * @brief 夹爪驱动实现
 *        舵机指令格式: #000Pxxxxtyyyy!
 */
#include "d_gripper.h"
#include <stdio.h>
#include <string.h>

// ! ========================= 变 量 声 明 ========================= ! //

#define GRIPPER_OPEN_POS    500
#define GRIPPER_CLOSE_POS   2500
#define GRIPPER_MOVE_TIME   1000

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _send_servo_cmd(Gripper* self, uint16_t pulse, uint16_t time_ms);
static void _init(Gripper* self, usart_t* uart);
static void _open(Gripper* self);
static void _close(Gripper* self);
static void _set_pos(Gripper* self, uint16_t pulse);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 Gripper 对象
 * @param   None
 * @retval  Gripper 对象
 */
Gripper gripper_create(void) {
    Gripper obj;
    obj._uart_ = 0;
    obj.init = _init;
    obj.open = _open;
    obj.close = _close;
    obj.set_pos = _set_pos;
    return obj;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   发送舵机指令
 * @param   self 夹爪对象
 * @param   pulse 脉宽
 * @param   time_ms 时间
 * @retval  None
 */
static void _send_servo_cmd(Gripper* self, uint16_t pulse, uint16_t time_ms) {
    char buf[32];
    sprintf(buf, "#000P%04dT%04d!", pulse, time_ms);
    usart_send_string(self->_uart_, buf);
}

/**
 * @brief   初始化夹爪
 * @param   self 夹爪对象
 * @param   uart 串口对象
 * @retval  None
 */
static void _init(Gripper* self, usart_t* uart) {
    self->_uart_ = uart;
}

/**
 * @brief   打开夹爪
 * @param   self 夹爪对象
 * @retval  None
 */
static void _open(Gripper* self) {
    _send_servo_cmd(self, GRIPPER_OPEN_POS, GRIPPER_MOVE_TIME);
}

/**
 * @brief   关闭夹爪
 * @param   self 夹爪对象
 * @retval  None
 */
static void _close(Gripper* self) {
    _send_servo_cmd(self, GRIPPER_CLOSE_POS, GRIPPER_MOVE_TIME);
}

/**
 * @brief   设置夹爪位置
 * @param   self 夹爪对象
 * @param   pulse 脉宽
 * @retval  None
 */
static void _set_pos(Gripper* self, uint16_t pulse) {
    _send_servo_cmd(self, pulse, GRIPPER_MOVE_TIME);
}
