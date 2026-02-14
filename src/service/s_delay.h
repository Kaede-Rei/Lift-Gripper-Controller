/**
 * @file  s_delay.h
 * @brief 延时服务 (阻塞 & 非阻塞)
 */
#ifndef _s_delay_h_
#define _s_delay_h_

#include <stdbool.h>
#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef uint32_t ms_t;
typedef uint32_t us_t;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void s_delay_init(ms_t(*get_ms)(void), bool(*ms_timeout)(ms_t start, ms_t timeout_ms), us_t(*get_us)(void), bool(*us_timeout)(us_t start, us_t timeout_us));
void s_delay_us(us_t us);
void s_delay_ms(ms_t ms);
void s_delay_s(ms_t s);
bool s_nb_delay_us(us_t* start, us_t interval_us);
bool s_nb_delay_ms(ms_t* start, ms_t interval_ms);

#endif
