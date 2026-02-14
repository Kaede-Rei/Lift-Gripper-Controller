/**
 * @file  s_log.c
 * @brief 日志输出服务实现
 */
#include "s_log.h"

// ! ========================= 变 量 声 明 ========================= ! //

#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_RESET   "\x1b[0m"

// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   输出波形数据
 * @param   count 数据个数
 * @param   ... float* 类型的指针
 * @retval  None
 */
void s_log_wave(int count, ...) {
    printf("[WAVE] : ");
    va_list args;
    va_start(args, count);
    for(int i = 0; i < count; ++i) {
        float* ptr = (float*)va_arg(args, void*);
        if(i > 0) printf(",");
        printf("%f", *ptr);
    }
    va_end(args);
    printf("\r\n");
}

/**
 * @brief   输出 INFO 日志
 * @param   fmt 格式化字符串
 * @param   ... 参数
 * @retval  None
 */
void s_log_info(const char* fmt, ...) {
#if (LOG_LEVEL >= LOG_LEVEL_INFO)
    printf(ANSI_BLUE "[INFO] ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(ANSI_RESET "\r\n");
#else
    (void)fmt;
#endif
}

/**
 * @brief   输出 WARN 日志
 * @param   fmt 格式化字符串
 * @param   ... 参数
 * @retval  None
 */
void s_log_warn(const char* fmt, ...) {
#if (LOG_LEVEL >= LOG_LEVEL_WARN)
    printf(ANSI_YELLOW "[WARN] ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(ANSI_RESET "\r\n");
#else
    (void)fmt;
#endif
}

/**
 * @brief   输出 ERROR 日志
 * @param   fmt 格式化字符串
 * @param   ... 参数
 * @retval  None
 */
void s_log_error(const char* fmt, ...) {
#if (LOG_LEVEL >= LOG_LEVEL_ERROR)
    printf(ANSI_RED "[ERROR] ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(ANSI_RESET "\r\n");
#else
    (void)fmt;
#endif
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //


