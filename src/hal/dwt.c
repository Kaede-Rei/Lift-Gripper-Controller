/**
 * @file    dwt.c
 * @brief   DWT 微秒级计时器实现
 */
#include "dwt.h"

// ! ========================= 变 量 声 明 ========================= ! //



// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   初始化 DWT
 * @param   None
 * @retval  None
 */
void dwt_init(void) {
    if(!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
}

/**
 * @brief   获取系统运行微秒数
 * @param   None
 * @retval  us_t 微秒数
 */
us_t dwt_get_us(void) {
    return DWT->CYCCNT / CPU_FREQ_MHZ;
}

/**
 * @brief   检查是否超时 (微秒)
 * @param   start 起始时间
 * @param   timeout_us 超时时间
 * @retval  bool true:超时, false:未超时
 */
bool dwt_is_timeout(us_t start, us_t timeout_us) {
    us_t now = dwt_get_us();
    if(now >= start)
        return (now - start) >= timeout_us ? true : false;
    else
        return ((0xFFFFFFFFU - start) + now + 1) >= timeout_us ? true : false;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //


