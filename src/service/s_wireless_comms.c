/**
 * @file    s_wireless_comms.c
 * @brief   无线通信服务实现
 *          升降台升降 + 夹爪开合
 */
#include "s_wireless_comms.h"

#include <stdio.h>

// ! ========================= 变 量 声 明 ========================= ! //

float lift_target_pos_mm = 0.0f;

static usart_t* _usart;
static Relay* _lift_relay;
static Gripper* _gripper;

static uint8_t _rx_buf[USART_RX_BUF_SIZE];
static bool _cmd_start = false;
static bool _cmd_ready = false;
static uint8_t _cmd_idx = 1;

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _parse_cmd(uint8_t* cmd);
static bool _compare_cmd(uint8_t* cmd, const char* target);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

void s_wireless_comms_init(usart_t* usart, Relay* lift_relay, Gripper* gripper) {
    _usart = usart;
    _lift_relay = lift_relay;
    _gripper = gripper;
}

/**
 * @brief   无线通信服务处理函数
 * @param   None
 * @retval  bool - true:成功接收数据并处理, false:无数据或数据不完整或无命令
 */
bool s_wireless_comms_process(void) {
    uint8_t byte;
    if(usart_read_byte(_usart, &byte) == false) return false;

    if(byte == '$') {
        _rx_buf[0] = byte;
        _cmd_start = true;
    }

    if(_cmd_start) {
        while(byte != '#') {
            // 无数据，命令未结束
            if(usart_read_byte(_usart, &byte) == false) return false;

            _rx_buf[_cmd_idx++] = byte;

            // 命令过长，丢弃
            if(_cmd_idx >= USART_RX_BUF_SIZE) {
                _cmd_start = false;
                _cmd_idx = 1;
                return false;
            }
        }
        _rx_buf[_cmd_idx] = '\0';
        _cmd_start = false;
        _cmd_ready = true;
        _cmd_idx = 1;
    }

    // 处理命令
    if(_cmd_ready) {
        _parse_cmd(_rx_buf);
        _cmd_ready = false;
        return true;
    }

    return false;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   解析命令字符串并执行对应操作
 * @param   cmd 待解析的命令字符串
 */
static void _parse_cmd(uint8_t* cmd) {
    float fvalue;

    // 升降台升降命令
    if(_compare_cmd(cmd, "$LIFT_UP#")) {
        _lift_relay->set_dir(_lift_relay, RelayDirA);
    }
    else if(_compare_cmd(cmd, "$LIFT_DOWN#")) {
        _lift_relay->set_dir(_lift_relay, RelayDirB);
    }
    else if(_compare_cmd(cmd, "$LIFT_STOP#")) {
        _lift_relay->stop(_lift_relay);
    }
    else if(sscanf((char*)cmd, "$LIFT_SET:%f#", &fvalue) == 1) {
        lift_target_pos_mm = fvalue;
    }

    // 夹爪开合命令
    else if(_compare_cmd(cmd, "$GRIP_OPEN#")) {
        _gripper->open(_gripper);
    }
    else if(_compare_cmd(cmd, "$GRIP_CLOSE#")) {
        _gripper->close(_gripper);
    }
    else if(sscanf((char*)cmd, "$GRIP_SET:%f#", &fvalue) == 1) {
        _gripper->set_angle(_gripper, fvalue);
    }
}

/**
 * @brief   比较命令字符串
 * @param   cmd 待比较的命令字符串
 * @param   target 目标命令字符串
 * @retval  bool - true:命令匹配, false:命令不匹配
 */
static bool _compare_cmd(uint8_t* cmd, const char* target) {
    while(*target) {
        if(*cmd++ != *target++) return false;
    }

    return (*cmd == '\0');
}
