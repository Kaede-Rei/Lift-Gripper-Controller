/**
 * @file  a_board.c
 * @brief 板级初始化实现
 *        按 HAL → DRVL → SRVL 顺序创建并初始化所有模块
 */
#include "a_board.h"
#include "main.h"

// ! ========================= 变 量 声 明 ========================= ! //

static const can_cfg_t can_cfg = {
    .id = CAN_1,
    .mode = CAN_MODE_NORMAL,
    .sjw = CAN_SJW_1tq,
    .bs1 = CAN_BS1_7tq,
    .bs2 = CAN_BS2_1tq,
    .prescaler = 4,           /* 36 MHz / (4 * (1+7+1)) = 1 Mbps */
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

static const usart_cfg_t usart2_cfg = {
    .id = USART_2,
    .baudrate = USART2_BAUD,
    .enable_rx_irq = 0,
    .nvic_preempt = 0,
    .nvic_sub = 0,
};

static const tim_cfg_t tick_cfg = {
    .id = TIM_3,
    .prescaler = 720 - 1,
    .period = 1000 - 1,    /* 72 MHz / 720 / 1000 = 10 ms */
    .nvic_preempt = 1,
    .nvic_sub = 1,
};

can_t can;
usart_t usart1;
usart_t usart2;
tim_t tick;

Encoder encoder;
LiftMotor lift_motor;
Gripper gripper;
Comms comms;
LiftControl lift_ctrl;

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
    encoder = encoder_create();
    lift_motor = lift_motor_create();
    gripper = gripper_create();
    comms = comms_create();
    lift_ctrl = lift_ctrl_create();

    /* HAL 初始化 (配置表驱动) */
    can_init(&can, &can_cfg);
    usart_init(&usart1, &usart1_cfg);
    usart_init(&usart2, &usart2_cfg);
    tim_init(&tick, &tick_cfg);

    /* 驱动初始化 */
    encoder.init(&encoder);
    lift_motor.init(&lift_motor);
    gripper.init(&gripper, &usart2);

    /* 服务初始化 */
    s_delay_init(systick_get_ms, systick_is_timeout, dwt_get_us, dwt_is_timeout);
    comms.init(&comms, &usart1, &lift_ctrl, &gripper);
    lift_ctrl.init(&lift_ctrl, &encoder, &lift_motor);

    s_delay_ms(1000);
    printf("Board initialized!\r\n");
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

