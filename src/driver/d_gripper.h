/**
 * @file    d_gripper.h
 * @brief   二指夹爪驱动
 */
#ifndef _d_gripper_h_
#define _d_gripper_h_

#include "stm32f10x.h"
#include "can.h"

#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef struct Gripper Gripper;
struct Gripper {
// public:
    /**
     * @brief   初始化夹爪
     * @param   self 夹爪对象
     * @param   can_cfg CAN配置对象
     * @param   motor_id 电机ID
     * @retval  None
     */
    void(*init)(Gripper* self, can_t* can, uint16_t motor_id);
    /**
     * @brief   使能夹爪
     * @param   self 夹爪对象
     * @retval  None
     */
    void(*enable)(Gripper* self);
    /**
     * @brief   失能夹爪
     * @param   self 夹爪对象
     */
    void(*disable)(Gripper* self);
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
     * @brief   设置夹爪角度
     * @param   self 夹爪对象
     * @param   angle 角度
     * @retval  None
     */
    void(*set_angle)(Gripper* self, float angle);

// private:
    can_t* _can_;
    uint16_t _motor_id_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

Gripper gripper_create(void);

#endif
