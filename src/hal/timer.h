/**
 * @file    timer.h
 * @brief   通用定时器 HAL — 配置表驱动
 *          支持 TIM1 ~ TIM4 周期中断
 */
#ifndef _timer_h_
#define _timer_h_

#include "stm32f10x.h"
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef void (*tim_cb_t)(void);

/**
 * @brief 定时器 ID 枚举
 */
typedef enum {
    TIM_1 = 0,
    TIM_2,
    TIM_3,
    TIM_4,
    TIM_COUNT
} tim_id_e;

/**
 * @brief 定时器模式
 */
typedef enum {
    TIM_MODE_BASE = 0,
    TIM_MODE_OC_PWM,
    TIM_MODE_IC,
    TIM_MODE_ENCODER,
} tim_mode_e;

/**
 * @brief PWM 配置
 */
typedef struct {
    uint16_t channel;           // TIM_Channel_x
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t gpio_rcc_mask;
    uint8_t gpio_rcc_bus;       // 1=APB1, 2=APB2
    GPIOMode_TypeDef gpio_mode;
    uint16_t oc_mode;           // TIM_OCMode_PWM1/2
    uint16_t oc_polarity;       // TIM_OCPolarity_x
    uint16_t pulse;             // 占空比
    uint8_t output_state;       // TIM_OutputState_Enable/Disable
    uint8_t preload;            // 1=使能预装载
} tim_oc_pwm_cfg_t;

/**
 * @brief 输入捕获配置
 */
typedef struct {
    uint16_t channel;           // TIM_Channel_x
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t gpio_rcc_mask;
    uint8_t gpio_rcc_bus;       // 1=APB1, 2=APB2
    GPIOMode_TypeDef gpio_mode;
    uint16_t ic_polarity;       // TIM_ICPolarity_x
    uint16_t ic_selection;      // TIM_ICSelection_x
    uint16_t ic_prescaler;      // TIM_ICPSC_x
    uint16_t ic_filter;         // 滤波
} tim_ic_cfg_t;

/**
 * @brief 编码器配置
 */
typedef struct {
    GPIO_TypeDef* ch1_port;
    uint16_t ch1_pin;
    uint32_t ch1_gpio_rcc_mask;
    uint8_t ch1_gpio_rcc_bus;   // 1=APB1, 2=APB2
    GPIO_TypeDef* ch2_port;
    uint16_t ch2_pin;
    uint32_t ch2_gpio_rcc_mask;
    uint8_t ch2_gpio_rcc_bus;   // 1=APB1, 2=APB2
    GPIOMode_TypeDef gpio_mode;
    uint8_t ic_filter;          // 输入捕获滤波
    uint16_t ic_polarity_ch1;   // 通道1 极性
    uint16_t ic_polarity_ch2;   // 通道2 极性
    uint16_t encoder_mode;      // 编码器模式
} tim_encoder_cfg_t;

/**
 * @brief 定时器配置表
 */
typedef struct {
    tim_id_e id;            // 定时器 ID
    TIM_TypeDef* periph;    // 定时器外设
    tim_mode_e mode;        // 定时器模式
    uint16_t prescaler;     // 预分频系数
    uint16_t period;        // 自动重装值
    uint8_t enable_irq;     // 1=使能更新中断
    uint8_t nvic_preempt;   // 抢占优先级
    uint8_t nvic_sub;       // 子优先级
    union {
        tim_oc_pwm_cfg_t oc_pwm;
        tim_ic_cfg_t ic;
        tim_encoder_cfg_t encoder;
    } cfg;
} tim_cfg_t;

/**
 * @brief 定时器运行时句柄
 */
typedef struct {
    const tim_cfg_t* cfg;   // 指向配置表
    volatile uint8_t flag;  // 中断标志
    tim_cb_t callback;      // 中断回调
} tim_t;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void tim_init(tim_t* handle, const tim_cfg_t* cfg);
void tim_set_callback(tim_t* handle, tim_cb_t cb);

#endif
