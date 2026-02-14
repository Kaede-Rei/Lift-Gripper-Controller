/**
 * @file  s_log.h
 * @brief 日志输出服务
 */
#ifndef _s_log_h_
#define _s_log_h_

#include <stdio.h>
#include <stdarg.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

#define LOG_LEVEL_ERROR  1
#define LOG_LEVEL_WARN   2
#define LOG_LEVEL_INFO   3

// 日志等级
#ifndef LOG_LEVEL
#define LOG_LEVEL  LOG_LEVEL_INFO
#endif

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void s_log_wave(int count, ...);
void s_log_info(const char* fmt, ...);
void s_log_warn(const char* fmt, ...);
void s_log_error(const char* fmt, ...);

#endif
