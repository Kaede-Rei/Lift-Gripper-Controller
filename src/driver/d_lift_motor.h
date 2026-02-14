/**
 * @file  d_lift_motor.h
 * @brief 升降台电机驱动 (OOP 封装)
 *        PB0 / PB1 控制启停和方向
 */
#ifndef _d_lift_motor_h_
#define _d_lift_motor_h_

#include "stm32f10x.h"

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef enum {
    LiftDirStop_e = 0,
    LiftDirUp_e,
    LiftDirDown_e
} LiftDir_e;

typedef struct LiftMotor LiftMotor;
struct LiftMotor {
// public:
    /**
     * @brief   初始化电机
     * @param   self 电机对象
     * @retval  None
     */
    void (*init)(LiftMotor* self);
    /**
     * @brief   设置电机方向
     * @param   self 电机对象
     * @param   dir 方向
     * @retval  None
     */
    void (*set_dir)(LiftMotor* self, LiftDir_e dir);
    /**
     * @brief   停止电机
     * @param   self 电机对象
     * @retval  None
     */
    void (*stop)(LiftMotor* self);
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

LiftMotor lift_motor_create(void);

#endif
