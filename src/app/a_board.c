/**
 * @file    a_board.c
 * @brief   板级初始化实现
 *          按 HAL → DRVL → SRVL 顺序创建并初始化所有模块
 */
#include "a_board.h"

// ! ========================= 变 量 声 明 ========================= ! //

#define USART1_BAUD             115200
#define USART2_BAUD             115200
#define TICK_PERIOD_MS          10

// 实际每毫米的脉冲数 (经测量校准)
#define ACTUAL_PULSE_PER_MM     15.518f

static const relay_cfg_t relay_cfg = {
    .rcc_mask = RCC_APB2Periph_GPIOB,
    .rcc_bus = 2,
    .port = GPIOB,
    .pin_a = GPIO_Pin_0,
    .pin_b = GPIO_Pin_1,
};

static const can_cfg_t can_cfg = {
    .id = CAN_1,
    .periph = CAN1,
    .mode = CAN_MODE_NORMAL,
    .sjw = CAN_SJW_1tq,
    .bs1 = CAN_BS1_7tq,
    .bs2 = CAN_BS2_1tq,
    .prescaler = 4,           // 36 MHz / (4 * (1+7+1)) = 1 Mbps
    .nvic_preempt = 1,
    .nvic_sub = 0,
};

static const usart_cfg_t usart1_cfg = {
    .id = USART_1,
    .baudrate = USART1_BAUD,
    .enable_rx_irq = 1,
    .nvic_preempt = 3,
    .nvic_sub = 3,
};

static const tim_cfg_t tim_cfg_table[TIM_COUNT] = {
    [TIM_2] = {
        .id = TIM_2,
        .periph = TIM2,
        .mode = TIM_MODE_ENCODER,
        .prescaler = 0,
        .period = 65536 - 1,
        .enable_irq = 0,
        .nvic_preempt = 0,
        .nvic_sub = 0,
        .cfg.encoder = {
            .ch1_port = GPIOA,
            .ch1_pin = GPIO_Pin_0,
            .ch1_gpio_rcc_mask = RCC_APB2Periph_GPIOA,
            .ch1_gpio_rcc_bus = 2,
            .ch2_port = GPIOA,
            .ch2_pin = GPIO_Pin_1,
            .ch2_gpio_rcc_mask = RCC_APB2Periph_GPIOA,
            .ch2_gpio_rcc_bus = 2,
            .gpio_mode = GPIO_Mode_IPU,
            .ic_filter = 0x0F,
            .ic_polarity_ch1 = TIM_ICPolarity_Rising,
            .ic_polarity_ch2 = TIM_ICPolarity_Rising,
            .encoder_mode = TIM_EncoderMode_TI12,
        },
    },
    [TIM_3] = {
        .id = TIM_3,
        .periph = TIM3,
        .mode = TIM_MODE_BASE,
        .prescaler = 720 - 1,
        .period = 1000 - 1,    // 72 MHz / 720 / 1000 = 10 ms
        .enable_irq = 1,
        .nvic_preempt = 1,
        .nvic_sub = 1,
    },
};

can_t can;
usart_t usart1;
usart_t usart2;
tim_t tick;

Encoder lift_encoder;
Relay lift_relay;
Gripper gripper;

// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   板级初始化函数
 * @param   None
 * @retval  None
 */
void a_board_init(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 底层时基 */
    systick_init();
    dwt_init();

    /* 创建对象 */
    lift_encoder = encoder_create();
    lift_relay = relay_create();
    gripper = gripper_create();

    /* HAL 初始化 */
    can_init(&can, &can_cfg);
    usart_init(&usart1, &usart1_cfg);
    tim_init(&tick, &tim_cfg_table[TIM_3]);

    /* 驱动初始化 */
    lift_encoder.init(&lift_encoder, &tim_cfg_table[TIM_2], 10, ACTUAL_PULSE_PER_MM);
    lift_relay.init(&lift_relay, &relay_cfg);
    gripper.init(&gripper, &can, 0x01);

    /* 服务初始化 */
    s_delay_init(systick_get_ms, systick_is_timeout, dwt_get_us, dwt_is_timeout);
    s_wireless_comms_init(&usart1, &lift_relay, &gripper);

    s_delay_ms(1000);
    printf("Board initialized!\r\n");
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

