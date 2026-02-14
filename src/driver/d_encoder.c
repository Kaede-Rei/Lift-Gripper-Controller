/**
 * @file  d_encoder.c
 * @brief 编码器驱动实现
 *        TIM2 编码器模式, PA0=CH1, PA1=CH2
 *        读取脉冲并换算为毫米位移与速度
 */
#include "d_encoder.h"
#include "main.h"

// ! ========================= 变 量 声 明 ========================= ! //

#define SAMPLING_PERIOD_MS   TICK_PERIOD_MS

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static int _read_raw(void);
static void _init(Encoder* self);
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
static int _read_raw(void) {
    int cnt = (int)TIM2->CNT;
    TIM_SetCounter(TIM2, 0);
    return cnt;
}

/**
 * @brief   初始化编码器
 * @param   self 编码器对象
 * @retval  None
 */
static void _init(Encoder* self) {
    self->_total_pulses_ = 0;
    self->_position_mm_ = 0;
    self->_speed_ = 0;

    /* TIM2 编码器模式 GPIO: PA0(CH1), PA1(CH2) */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio);

    /* TIM2 时基: 16-bit 自由计数 */
    TIM_TimeBaseInitTypeDef tb;
    tb.TIM_Period = 65536 - 1;
    tb.TIM_Prescaler = 0;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    tb.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &tb);

    /* 输入捕获滤波 */
    TIM_ICInitTypeDef ic;
    TIM_ICStructInit(&ic);
    ic.TIM_Channel = TIM_Channel_1;
    ic.TIM_ICFilter = 0x0F;
    TIM_ICInit(TIM2, &ic);
    ic.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM2, &ic);

    /* 编码器接口: 双通道计数 */
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12,
        TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief   更新编码器数据
 * @param   self 编码器对象
 * @retval  None
 */
static void _update(Encoder* self) {
    int raw = _read_raw();

    if(raw < 32767)
        self->_total_pulses_ += raw;
    else
        self->_total_pulses_ -= (65536 - raw);

    self->_position_mm_ = (int32_t)((float)self->_total_pulses_ / ACTUAL_PULSE_PER_MM + 0.5f);
    self->_speed_ = (int32_t)((float)raw / ACTUAL_PULSE_PER_MM
        / (SAMPLING_PERIOD_MS / 1000.0f));
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
