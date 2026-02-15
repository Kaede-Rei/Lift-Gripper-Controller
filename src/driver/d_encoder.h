/**
 * @file    d_encoder.h
 * @brief   编码器驱动
 */
#ifndef _d_encoder_h_
#define _d_encoder_h_

#include "timer.h"

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
    void(*init)(Encoder* self, const tim_cfg_t* cfg, int period_ms, int32_t pulses_per_mm);
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
    float(*get_position)(const Encoder* self);
    /**
     * @brief   获取速度
     * @param   self 编码器对象
     * @retval  int32_t 速度(mm/s)
     */
    float(*get_speed)(const Encoder* self);

// private:
    tim_cfg_t _tim_cfg_;
    int _period_ms_;

    int32_t _total_pulses_;
    int32_t _pulses_per_mm_;
    float _position_mm_;
    float _speed_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

Encoder encoder_create(void);

#endif
