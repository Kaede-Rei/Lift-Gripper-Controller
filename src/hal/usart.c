/**
 * @file    usart.c
 * @brief   USART HAL 实现 — 配置表驱动
 *          根据 usart_cfg_t 自动适配 USART1 ~ USART3
 *          默认引脚:
 *              USART1: PA9-TX   PA10-RX
 *              USART2: PA2-TX   PA3-RX
 *              USART3: PB10-TX  PB11-RX
 */
#include "usart.h"
#include <stdio.h>

// ! ========================= 变 量 声 明 ========================= ! //

typedef struct {
    USART_TypeDef* periph;
    uint32_t rcc_periph;
    uint8_t rcc_bus;      /* 1 = APB1, 2 = APB2 */
    uint32_t rcc_gpio;
    GPIO_TypeDef* tx_port;
    uint16_t tx_pin;
    GPIO_TypeDef* rx_port;
    uint16_t rx_pin;
    uint8_t irqn;
} usart_hw_t;

static const usart_hw_t _hw[USART_COUNT] = {
    [USART_1] = {.periph = USART1,
                .rcc_periph = RCC_APB2Periph_USART1,
                .rcc_bus = 2,
                .rcc_gpio = RCC_APB2Periph_GPIOA,
                .tx_port = GPIOA,
                .tx_pin = GPIO_Pin_9,
                .rx_port = GPIOA,
                .rx_pin = GPIO_Pin_10,
                .irqn = USART1_IRQn },
    [USART_2] = {.periph = USART2,
                .rcc_periph = RCC_APB1Periph_USART2,
                .rcc_bus = 1,
                .rcc_gpio = RCC_APB2Periph_GPIOA,
                .tx_port = GPIOA,
                .tx_pin = GPIO_Pin_2,
                .rx_port = GPIOA,
                .rx_pin = GPIO_Pin_3,
                .irqn = USART2_IRQn },
    [USART_3] = {.periph = USART3,
                .rcc_periph = RCC_APB1Periph_USART3,
                .rcc_bus = 1,
                .rcc_gpio = RCC_APB2Periph_GPIOB,
                .tx_port = GPIOB,
                .tx_pin = GPIO_Pin_10,
                .rx_port = GPIOB,
                .rx_pin = GPIO_Pin_11,
                .irqn = USART3_IRQn },
};

static usart_t* _handles[USART_COUNT] = { 0 };

// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   初始化 USART (依据配置表)
 * @param   handle 句柄
 * @param   cfg 配置表
 */
void usart_init(usart_t* handle, const usart_cfg_t* cfg) {
    handle->cfg = cfg;
    handle->rx_head = 0;
    handle->rx_tail = 0;

    usart_id_e id = cfg->id;
    const usart_hw_t* hw = &_hw[id];
    _handles[id] = handle;

    /* GPIO 时钟 (始终需要 APB2) */
    RCC_APB2PeriphClockCmd(hw->rcc_gpio, ENABLE);

    /* USART 外设时钟 */
    if(hw->rcc_bus == 2)
        RCC_APB2PeriphClockCmd(hw->rcc_periph, ENABLE);
    else
        RCC_APB1PeriphClockCmd(hw->rcc_periph, ENABLE);

    USART_DeInit(hw->periph);

    /* GPIO */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = hw->tx_pin;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(hw->tx_port, &gpio);
    gpio.GPIO_Pin = hw->rx_pin;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(hw->rx_port, &gpio);

    /* USART */
    USART_InitTypeDef ui;
    ui.USART_BaudRate = cfg->baudrate;
    ui.USART_WordLength = USART_WordLength_8b;
    ui.USART_StopBits = USART_StopBits_1;
    ui.USART_Parity = USART_Parity_No;
    ui.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    ui.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(hw->periph, &ui);

    /* RX 中断 */
    if(cfg->enable_rx_irq) {
        NVIC_InitTypeDef ni;
        ni.NVIC_IRQChannel = hw->irqn;
        ni.NVIC_IRQChannelPreemptionPriority = cfg->nvic_preempt;
        ni.NVIC_IRQChannelSubPriority = cfg->nvic_sub;
        ni.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&ni);
        USART_ITConfig(hw->periph, USART_IT_RXNE, ENABLE);
    }

    USART_Cmd(hw->periph, ENABLE);
}

/**
 * @brief   发送单字节
 * @param   handle 句柄
 * @param   byte 字节数据
 */
void usart_send_byte(usart_t* handle, uint8_t byte) {
    const usart_hw_t* hw = &_hw[handle->cfg->id];
    while(USART_GetFlagStatus(hw->periph, USART_FLAG_TC) == RESET);
    USART_SendData(hw->periph, byte);
}

/**
 * @brief   发送字符串
 * @param   handle 句柄
 * @param   str 字符串
 */
void usart_send_string(usart_t* handle, const char* str) {
    while(*str)
        usart_send_byte(handle, (uint8_t)*str++);
}

/**
 * @brief   读取单字节 (从环形缓冲区)
 * @param   handle 句柄
 * @param   out 输出缓冲区
 * @retval  bool - true:成功, false:缓冲区空
 */
bool usart_read_byte(usart_t* handle, uint8_t* out) {
    if(handle->rx_head == handle->rx_tail) return false;
    *out = handle->rx_buf[handle->rx_tail];
    handle->rx_tail = (handle->rx_tail + 1) % USART_RX_BUF_SIZE;
    return true;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   USART 中断服务函数
 * @note    由 USART1_IRQHandler、USART2_IRQHandler、USART3_IRQHandler 调用
 */
static void _usart_irq(usart_id_e id) {
    usart_t* handle = _handles[id];
    if(!handle) return;
    const usart_hw_t* hw = &_hw[id];
    if(USART_GetITStatus(hw->periph, USART_IT_RXNE) != RESET) {
        uint8_t data = (uint8_t)USART_ReceiveData(hw->periph);
        uint16_t next = (handle->rx_head + 1) % USART_RX_BUF_SIZE;
        // 如果缓冲区未满，则存储数据；否则丢弃数据
        if(next != handle->rx_tail) {
            handle->rx_buf[handle->rx_head] = data;
            handle->rx_head = next;
        }
        USART_ClearITPendingBit(hw->periph, USART_IT_RXNE);
    }
}

void USART1_IRQHandler(void) { _usart_irq(USART_1); }
void USART2_IRQHandler(void) { _usart_irq(USART_2); }
void USART3_IRQHandler(void) { _usart_irq(USART_3); }

#pragma import(__use_no_semihosting)

struct __FILE { int handle; };
FILE __stdout;

void _sys_exit(int x) { x = x; }

int fputc(int ch, FILE* f) {
    (void)f;
    while((USART1->SR & 0x40) == 0);
    USART1->DR = (uint8_t)ch;
    return ch;
}
