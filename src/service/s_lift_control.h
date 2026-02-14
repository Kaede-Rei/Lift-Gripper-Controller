/**
 * @file  s_lift_control.h
 * @brief 升降台控制服务 (OOP 封装)
 *        整合编码器反馈与电机控制
 */
#ifndef _s_lift_control_h_
#define _s_lift_control_h_

#include "d_encoder.h"
#include "d_lift_motor.h"
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef struct LiftControl LiftControl;
struct LiftControl {
// public:
    /**
     * @brief   初始化控制模块
     * @param   self 升降机控制器对象
     * @param   enc 编码器对象
     * @param   motor 电机对象
     * @retval  None
     */
    void (*init)(LiftControl* self, Encoder* enc, LiftMotor* motor);
    /**
     * @brief   设定目标高度差
     * @param   self 升降机控制器对象
     * @param   delta_mm 高度差(mm)
     * @retval  None
     */
    void (*set_height)(LiftControl* self, float delta_mm);
    /**
     * @brief   手动控制命令
     * @param   self 升降机控制器对象
     * @param   dir 方向
     * @retval  None
     */
    void (*manual)(LiftControl* self, LiftDir_e dir);
    /**
     * @brief   控制循环更新
     * @param   self 升降机控制器对象
     * @retval  None
     */
    void (*update)(LiftControl* self);

// private:
    Encoder* _encoder_;
    LiftMotor* _motor_;
    int32_t _target_pos_;
    float _height_diff_;
    uint8_t _rcvd_flag_;
    LiftDir_e _manual_dir_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

LiftControl lift_ctrl_create(void);

#endif
