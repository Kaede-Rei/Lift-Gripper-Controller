/**
 * @file  d_encoder.c
 * @brief 编码器驱动实现
 *        TIM2 编码器模式, PA0=CH1, PA1=CH2
 *        读取脉冲并换算为毫米位移与速度
 */
#include "d_encoder.h"
#include "timer.h"

// ! ========================= 变 量 声 明 ========================= ! //

// 实际每毫米的脉冲数 (经测量校准)
#define ACTUAL_PULSE_PER_MM     15.518f

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static int _read_raw(const tim_cfg_t* cfg);
static void _init(Encoder* self, const tim_cfg_t* cfg, int period_ms);
static void _update(Encoder* self);
static int32_t _get_position(const Encoder* self);
static int32_t _get_speed(const Encoder* self);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 Encoder 对象
 * @param   None
 * @retval  Encoder 对象
 */
Encoder encoder_create(void) {
    Encoder obj;
    obj._total_pulses_ = 0;
    obj._position_mm_ = 0;
    obj._speed_ = 0;
    obj.init = _init;
    obj.update = _update;
    obj.get_position = _get_position;
    obj.get_speed = _get_speed;
    return obj;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   读取原始计数值
 * @param   None
 * @retval  int 计数值
 */
static int _read_raw(const tim_cfg_t* cfg) {
    int cnt = (int)TIM_GetCounter(cfg->periph);
    TIM_SetCounter(cfg->periph, 0);
    return cnt;
}

/**
 * @brief   初始化编码器
 * @param   self 编码器对象
 * @retval  None
 */
static void _init(Encoder* self, const tim_cfg_t* cfg, int period_ms) {
    self->_total_pulses_ = 0;
    self->_position_mm_ = 0;
    self->_speed_ = 0;
    self->_tim_cfg_ = *cfg;
    self->_period_ms_ = period_ms;

    tim_init(0, cfg);
}

/**
 * @brief   更新编码器数据
 * @param   self 编码器对象
 * @retval  None
 */
static void _update(Encoder* self) {
    int raw = _read_raw(&self->_tim_cfg_);

    if(raw < 32767)
        self->_total_pulses_ += raw;
    else
        self->_total_pulses_ -= (65536 - raw);

    self->_position_mm_ = (int32_t)((float)self->_total_pulses_ / ACTUAL_PULSE_PER_MM + 0.5f);
    self->_speed_ = (int32_t)((float)raw / ACTUAL_PULSE_PER_MM
        / (self->_period_ms_ / 1000.0f));
}

/**
 * @brief   获取位置
 * @param   self 编码器对象
 * @retval  int32_t 位置(mm)
 */
static int32_t _get_position(const Encoder* self) {
    return self->_position_mm_;
}

/**
 * @brief   获取速度
 * @param   self 编码器对象
 * @retval  int32_t 速度(mm/s)
 */
static int32_t _get_speed(const Encoder* self) {
    return self->_speed_;
}
