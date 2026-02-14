/**
 * @file  timer.c
 * @brief 通用定时器 HAL 实现 — 配置表驱动
 *        根据 tim_cfg_t 自动适配 TIM1 ~ TIM4
 */
#include "timer.h"

// ! ========================= 变 量 声 明 ========================= ! //

typedef struct {
    TIM_TypeDef* periph;
    uint32_t rcc_mask;
    uint8_t rcc_bus;       /* 1 = APB1, 2 = APB2 */
    uint8_t irqn;
} tim_hw_t;

static const tim_hw_t _hw[TIM_COUNT] = {
    [TIM_1] = {.periph = TIM1,
            .rcc_mask = RCC_APB2Periph_TIM1,
            .rcc_bus = 2,
            .irqn = TIM1_UP_IRQn },
    [TIM_2] = {.periph = TIM2,
            .rcc_mask = RCC_APB1Periph_TIM2,
            .rcc_bus = 1,
            .irqn = TIM2_IRQn },
    [TIM_3] = {.periph = TIM3,
            .rcc_mask = RCC_APB1Periph_TIM3,
            .rcc_bus = 1,
            .irqn = TIM3_IRQn },
    [TIM_4] = {.periph = TIM4,
            .rcc_mask = RCC_APB1Periph_TIM4,
            .rcc_bus = 1,
            .irqn = TIM4_IRQn },
};

static tim_t* _handles[TIM_COUNT] = { 0 };

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _gpio_init(GPIO_TypeDef* port, uint16_t pin, uint32_t rcc_mask,
    uint8_t rcc_bus, GPIOMode_TypeDef mode);


// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   初始化定时器 (依据配置表)
 * @param   handle 句柄
 * @param   cfg 配置表
 */
void tim_init(tim_t* handle, const tim_cfg_t* cfg) {
    if(!cfg) return;

    tim_id_e id = cfg->id;
    const tim_hw_t* hw = &_hw[id];

    if(handle) {
        handle->cfg = cfg;
        handle->flag = 0;
        handle->callback = 0;
        _handles[id] = handle;
    }
    else {
        _handles[id] = 0;
    }

    if(hw->rcc_bus == 2)
        RCC_APB2PeriphClockCmd(hw->rcc_mask, ENABLE);
    else
        RCC_APB1PeriphClockCmd(hw->rcc_mask, ENABLE);

    TIM_InternalClockConfig(hw->periph);

    TIM_TimeBaseInitTypeDef tb;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    tb.TIM_Period = cfg->period;
    tb.TIM_Prescaler = cfg->prescaler;
    tb.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(hw->periph, &tb);

    if(cfg->enable_irq && handle) {
        TIM_ClearFlag(hw->periph, TIM_FLAG_Update);
        TIM_ITConfig(hw->periph, TIM_IT_Update, ENABLE);

        NVIC_InitTypeDef ni;
        ni.NVIC_IRQChannel = hw->irqn;
        ni.NVIC_IRQChannelCmd = ENABLE;
        ni.NVIC_IRQChannelPreemptionPriority = cfg->nvic_preempt;
        ni.NVIC_IRQChannelSubPriority = cfg->nvic_sub;
        NVIC_Init(&ni);
    }
    else {
        TIM_ITConfig(hw->periph, TIM_IT_Update, DISABLE);
    }

    switch(cfg->mode) {
        case TIM_MODE_BASE:
            break;
        case TIM_MODE_ENCODER: {
            const tim_encoder_cfg_t* ecfg = &cfg->cfg.encoder;
            _gpio_init(ecfg->ch1_port, ecfg->ch1_pin, ecfg->ch1_gpio_rcc_mask,
                ecfg->ch1_gpio_rcc_bus, ecfg->gpio_mode);
            _gpio_init(ecfg->ch2_port, ecfg->ch2_pin, ecfg->ch2_gpio_rcc_mask,
                ecfg->ch2_gpio_rcc_bus, ecfg->gpio_mode);

            TIM_ICInitTypeDef ic;
            TIM_ICStructInit(&ic);
            ic.TIM_ICFilter = ecfg->ic_filter;
            ic.TIM_Channel = TIM_Channel_1;
            ic.TIM_ICPolarity = ecfg->ic_polarity_ch1;
            TIM_ICInit(hw->periph, &ic);

            ic.TIM_Channel = TIM_Channel_2;
            ic.TIM_ICPolarity = ecfg->ic_polarity_ch2;
            TIM_ICInit(hw->periph, &ic);

            TIM_EncoderInterfaceConfig(hw->periph, ecfg->encoder_mode,
                ecfg->ic_polarity_ch1, ecfg->ic_polarity_ch2);
            TIM_SetCounter(hw->periph, 0);
            break;
        }
        case TIM_MODE_OC_PWM: {
            const tim_oc_pwm_cfg_t* pcfg = &cfg->cfg.oc_pwm;
            _gpio_init(pcfg->port, pcfg->pin, pcfg->gpio_rcc_mask,
                pcfg->gpio_rcc_bus, pcfg->gpio_mode);

            TIM_OCInitTypeDef oc;
            TIM_OCStructInit(&oc);
            oc.TIM_OCMode = pcfg->oc_mode;
            oc.TIM_OutputState = pcfg->output_state;
            oc.TIM_Pulse = pcfg->pulse;
            oc.TIM_OCPolarity = pcfg->oc_polarity;

            switch(pcfg->channel) {
                case TIM_Channel_1: TIM_OC1Init(hw->periph, &oc); break;
                case TIM_Channel_2: TIM_OC2Init(hw->periph, &oc); break;
                case TIM_Channel_3: TIM_OC3Init(hw->periph, &oc); break;
                case TIM_Channel_4: TIM_OC4Init(hw->periph, &oc); break;
                default: break;
            }

            if(pcfg->preload) {
                switch(pcfg->channel) {
                    case TIM_Channel_1: TIM_OC1PreloadConfig(hw->periph, TIM_OCPreload_Enable); break;
                    case TIM_Channel_2: TIM_OC2PreloadConfig(hw->periph, TIM_OCPreload_Enable); break;
                    case TIM_Channel_3: TIM_OC3PreloadConfig(hw->periph, TIM_OCPreload_Enable); break;
                    case TIM_Channel_4: TIM_OC4PreloadConfig(hw->periph, TIM_OCPreload_Enable); break;
                    default: break;
                }
            }

            TIM_ARRPreloadConfig(hw->periph, ENABLE);
            break;
        }
        case TIM_MODE_IC: {
            const tim_ic_cfg_t* icfg = &cfg->cfg.ic;
            _gpio_init(icfg->port, icfg->pin, icfg->gpio_rcc_mask,
                icfg->gpio_rcc_bus, icfg->gpio_mode);

            TIM_ICInitTypeDef ic;
            TIM_ICStructInit(&ic);
            ic.TIM_Channel = icfg->channel;
            ic.TIM_ICPolarity = icfg->ic_polarity;
            ic.TIM_ICSelection = icfg->ic_selection;
            ic.TIM_ICPrescaler = icfg->ic_prescaler;
            ic.TIM_ICFilter = icfg->ic_filter;
            TIM_ICInit(hw->periph, &ic);
            break;
        }
        default:
            break;
    }

    TIM_Cmd(hw->periph, ENABLE);
}

/**
 * @brief   设置定时器中断回调
 * @param   handle 句柄
 * @param   cb 回调函数
 */
void tim_set_callback(tim_t* handle, tim_cb_t cb) {
    handle->callback = cb;
}

/**
 * @brief   GPIO 初始化
 * @param   port GPIO 端口
 * @param   pin GPIO 引脚
 * @param   rcc_mask RCC 时钟掩码
 * @param   rcc_bus RCC 时钟总线 (1=APB1, 2=APB2)
 * @param   mode GPIO 模式
 * @note    由定时器初始化函数调用
 */
static void _gpio_init(GPIO_TypeDef* port, uint16_t pin, uint32_t rcc_mask,
    uint8_t rcc_bus, GPIOMode_TypeDef mode) {
    if(rcc_bus == 2)
        RCC_APB2PeriphClockCmd(rcc_mask, ENABLE);
    else
        RCC_APB1PeriphClockCmd(rcc_mask, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = pin;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = mode;
    GPIO_Init(port, &gpio);
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   定时器中断服务函数
 * @param   id 定时器 ID
 * @note    由 TIM1_UP_IRQHandler、TIM2_IRQHandler、TIM3_IRQHandler、TIM4_IRQHandler 调用
 */
static void _tim_irq(tim_id_e id) {
    tim_t* handle = _handles[id];
    if(!handle) return;
    const tim_hw_t* hw = &_hw[id];
    if(TIM_GetFlagStatus(hw->periph, TIM_FLAG_Update) == SET) {
        handle->flag = 1;
        if(handle->callback) handle->callback();
        TIM_ClearITPendingBit(hw->periph, TIM_IT_Update);
    }
}

void TIM1_UP_IRQHandler(void) { _tim_irq(TIM_1); }
void TIM2_IRQHandler(void) { _tim_irq(TIM_2); }
void TIM3_IRQHandler(void) { _tim_irq(TIM_3); }
void TIM4_IRQHandler(void) { _tim_irq(TIM_4); }
