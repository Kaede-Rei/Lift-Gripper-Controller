/**
 * @file    can.c
 * @brief   CAN HAL 实现 — 配置表驱动
 *          根据 can_cfg_t 自动适配 CAN
 *          默认引脚: PA12-TX  PA11-RX
 */
#include "can.h"

#include "s_delay.h"

// ! ========================= 变 量 声 明 ========================= ! //

typedef struct {
    CAN_TypeDef* periph;
    uint32_t rcc_can;
    uint32_t rcc_gpio;
    GPIO_TypeDef* tx_port;
    uint16_t tx_pin;
    GPIO_TypeDef* rx_port;
    uint16_t rx_pin;
    uint8_t irqn;
} can_hw_t;

static const can_hw_t _hw[CAN_COUNT] = {
    [CAN_1] = {.periph = CAN1,
            .rcc_can = RCC_APB1Periph_CAN1,
            .rcc_gpio = RCC_APB2Periph_GPIOA,
            .tx_port = GPIOA,
            .tx_pin = GPIO_Pin_12,
            .rx_port = GPIOA,
            .rx_pin = GPIO_Pin_11,
            .irqn = USB_LP_CAN1_RX0_IRQn },
};

static can_t* _handles[CAN_COUNT] = { 0 };

static const uint8_t _mode_map[] = {
    [CAN_MODE_NORMAL] = CAN_Mode_Normal,
    [CAN_MODE_LOOPBACK] = CAN_Mode_LoopBack,
    [CAN_MODE_SILENT] = CAN_Mode_Silent,
    [CAN_MODE_SILENT_LOOPBACK] = CAN_Mode_Silent_LoopBack,
};

// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   初始化 CAN (依据配置表)
 * @param   handle 句柄
 * @param   cfg 配置表
 */
void can_init(can_t* handle, const can_cfg_t* cfg) {
    handle->cfg = cfg;
    handle->rx_cb = 0;

    can_id_e id = cfg->id;
    const can_hw_t* hw = &_hw[id];
    _handles[id] = handle;

    /* GPIO */
    RCC_APB2PeriphClockCmd(hw->rcc_gpio, ENABLE);
    RCC_APB1PeriphClockCmd(hw->rcc_can, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = hw->tx_pin;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(hw->tx_port, &gpio);
    gpio.GPIO_Pin = hw->rx_pin;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(hw->rx_port, &gpio);

    /* CAN 基本配置 */
    CAN_InitTypeDef ci;
    ci.CAN_TTCM = DISABLE;
    ci.CAN_ABOM = ENABLE;
    ci.CAN_AWUM = DISABLE;
    ci.CAN_NART = DISABLE;
    ci.CAN_RFLM = DISABLE;
    ci.CAN_TXFP = DISABLE;
    ci.CAN_Mode = _mode_map[cfg->mode];
    ci.CAN_SJW = cfg->sjw;
    ci.CAN_BS1 = cfg->bs1;
    ci.CAN_BS2 = cfg->bs2;
    ci.CAN_Prescaler = cfg->prescaler;
    CAN_Init(hw->periph, &ci);

    /* 滤波器: 全部接收 */
    CAN_FilterInitTypeDef fi;
    fi.CAN_FilterNumber = 0;
    fi.CAN_FilterMode = CAN_FilterMode_IdMask;
    fi.CAN_FilterScale = CAN_FilterScale_32bit;
    fi.CAN_FilterIdHigh = 0x0000;
    fi.CAN_FilterIdLow = 0x0000;
    fi.CAN_FilterMaskIdHigh = 0x0000;
    fi.CAN_FilterMaskIdLow = 0x0000;
    fi.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
    fi.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&fi);

    /* RX0 中断 */
    NVIC_InitTypeDef ni;
    ni.NVIC_IRQChannel = hw->irqn;
    ni.NVIC_IRQChannelPreemptionPriority = cfg->nvic_preempt;
    ni.NVIC_IRQChannelSubPriority = cfg->nvic_sub;
    ni.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&ni);
    CAN_ITConfig(hw->periph, CAN_IT_FMP0, ENABLE);
}

/**
 * @brief   发送 CAN 报文
 * @param   handle 句柄
 * @param   std_id 标准ID
 * @param   data   数据指针
 * @param   len    数据长度 (0~8)
 * @retval  true: 成功, false: 失败
 */
bool can_send(can_t* handle, uint16_t std_id, const uint8_t* data, uint8_t len) {
    if(len > 8) return false;
    const can_hw_t* hw = &_hw[handle->cfg->id];

    CanTxMsg tx;
    tx.StdId = std_id & 0x7FF;
    tx.ExtId = 0;
    tx.IDE = CAN_ID_STD;
    tx.RTR = CAN_RTR_DATA;
    tx.DLC = len;
    for(uint8_t i = 0; i < len; ++i)
        tx.Data[i] = data[i];

    uint8_t mbox = CAN_Transmit(hw->periph, &tx);
    if(mbox == CAN_TxStatus_NoMailBox) return false;

    ms_t start = 0;
    while(CAN_TransmitStatus(hw->periph, mbox) != CAN_TxStatus_Ok) {
        if(s_nb_delay_ms(&start, 50)) return false;
    }

    s_delay_ms(1);
    return true;
}

/**
 * @brief   设置接收回调
 * @param   handle 句柄
 * @param   cb 回调函数
 */
void can_set_rx_cb(can_t* handle, can_rx_cb_t cb) {
    handle->rx_cb = cb;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   CAN1 RX0 中断服务函数
 * @note    由 USB_LP_CAN1_RX0_IRQHandler 调用
 */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    can_t* handle = _handles[CAN_1];
    if(!handle) return;
    if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET) {
        CanRxMsg rx;
        CAN_Receive(CAN1, CAN_FIFO0, &rx);
        if(handle->rx_cb) handle->rx_cb(&rx);
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}
