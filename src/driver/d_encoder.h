/**
 * @file  d_encoder.h
 * @brief 编码器驱动 (OOP 封装)
 *        TIM2 编码器模式, PA0=CH1, PA1=CH2
 *        读取脉冲并换算为毫米位移与速度
 */
#ifndef _d_encoder_h_
#define _d_encoder_h_

#include "stm32f10x.h"
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef struct Encoder Encoder;
struct Encoder {
// public:
    /**
     * @brief   初始化编码器
     * @param   self 编码器对象
     * @retval  None
     */
    void(*init)(Encoder* self);
    /**
     * @brief   更新编码器数据
     * @param   self 编码器对象
     * @retval  None
     */
    void(*update)(Encoder* self);
    /**
     * @brief   获取位置
     * @param   self 编码器对象
     * @retval  int32_t 位置(mm)
     */
    int32_t(*get_position)(const Encoder* self);
    /**
     * @brief   获取速度
     * @param   self 编码器对象
     * @retval  int32_t 速度(mm/s)
     */
    int32_t(*get_speed)(const Encoder* self);

// private:
    int32_t _total_pulses_;
    int32_t _position_mm_;
    int32_t _speed_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

Encoder encoder_create(void);

#endif
