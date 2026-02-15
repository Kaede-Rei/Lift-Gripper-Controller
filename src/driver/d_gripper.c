/**
 * @file    d_gripper.c
 * @brief   二指夹爪驱动实现
 */
#include "d_gripper.h"
#include <stdio.h>
#include <string.h>

// ! ========================= 变 量 声 明 ========================= ! //

#define GRIPPER_OPEN_ANGLE      3.14f
#define GRIPPER_CLOSE_ANGLE     -1.93f
#define GRIPPER_MOVE_TIME_S     0.5f

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _init(Gripper* self, can_t* can, uint16_t motor_id);
static void _enable(Gripper* self);
static void _disable(Gripper* self);
static void _open(Gripper* self);
static void _close(Gripper* self);
static void _set_angle(Gripper* self, float angle);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 Gripper 对象
 * @param   None
 * @retval  Gripper 对象
 */
Gripper gripper_create(void) {
    Gripper obj;
    obj._can_ = 0;
    obj.init = _init;
    obj.enable = _enable;
    obj.disable = _disable;
    obj.open = _open;
    obj.close = _close;
    obj.set_angle = _set_angle;

    return obj;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   初始化夹爪
 * @param   self 夹爪对象
 * @param   can CAN对象
 * @retval  None
 */
static void _init(Gripper* self, can_t* can, uint16_t motor_id) {
    self->_can_ = can;
    self->_motor_id_ = motor_id + 0x100;
}

/**
 * @brief   使能夹爪
 * @param   self 夹爪对象
 */
static void _enable(Gripper* self) {
    // 发送使能指令
    uint8_t data[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC };
    can_send(self->_can_, self->_motor_id_, data, 8);

    // 切换为位置速度模式
    uint16_t id_l = self->_motor_id_ & 0x00FF;
    data[0] = (uint8_t)id_l;
    data[1] = 0x00;
    data[2] = 0x55;
    data[3] = 10;
    data[4] = 2;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    can_send(self->_can_, self->_motor_id_, data, 8);
}

/**
 * @brief   失能夹爪
 * @param   self 夹爪对象
 */
static void _disable(Gripper* self) {
    uint8_t data[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD };
    can_send(self->_can_, self->_motor_id_, data, 8);
}

/**
 * @brief   打开夹爪
 * @param   self 夹爪对象
 * @retval  None
 */
static void _open(Gripper* self) {
    _set_angle(self, GRIPPER_OPEN_ANGLE);
}

/**
 * @brief   关闭夹爪
 * @param   self 夹爪对象
 * @retval  None
 */
static void _close(Gripper* self) {
    _set_angle(self, GRIPPER_CLOSE_ANGLE);
}

/**
 * @brief   设置夹爪角度
 * @param   self 夹爪对象
 * @param   angle 角度
 * @retval  None
 */
static void _set_angle(Gripper* self, float angle) {
    uint8_t data[8];
    angle = (angle < GRIPPER_CLOSE_ANGLE) ? GRIPPER_CLOSE_ANGLE : ((angle > GRIPPER_OPEN_ANGLE) ? GRIPPER_OPEN_ANGLE : angle);
    float speed = (GRIPPER_MOVE_TIME_S > 0) ? (GRIPPER_OPEN_ANGLE - GRIPPER_CLOSE_ANGLE) / GRIPPER_MOVE_TIME_S : 10.0f;
    uint8_t* angle_bytes = (uint8_t*)&angle;
    uint8_t* speed_bytes = (uint8_t*)&speed;

    data[0] = *(angle_bytes);
    data[1] = *(angle_bytes + 1);
    data[2] = *(angle_bytes + 2);
    data[3] = *(angle_bytes + 3);
    data[4] = *(speed_bytes);
    data[5] = *(speed_bytes + 1);
    data[6] = *(speed_bytes + 2);
    data[7] = *(speed_bytes + 3);

    can_send(self->_can_, self->_motor_id_, data, 8);
}
