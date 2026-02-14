/**
 * @file  s_comms.c
 * @brief 无线串口通信协议实现
 *
 * 协议格式:
 *   字符串帧: $LIFTER:value#   $GRIPPER:OPEN#  $GRIPPER:CLOSE#  $GRIPPER:POS:xxxx#
 *   字节指令: 0x01=升  0x02=降  0x00=停
 */
#include "s_comms.h"
#include <string.h>
#include <stdlib.h>

// ! ========================= 变 量 声 明 ========================= ! //



// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _process_string_cmd(Comms* self, char* frame);
static void _init(Comms* self, usart_t* uart, LiftControl* lift, Gripper* grip);
static void _process(Comms* self);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 Comms 对象
 * @param   None
 * @retval  Comms 对象
 */
Comms comms_create(void) {
    Comms obj;
    obj._uart_ = 0;
    obj._lift_ = 0;
    obj._grip_ = 0;
    obj._cmd_idx_ = 0;
    obj._parsing_ = 0;
    obj.init = _init;
    obj.process = _process;
    return obj;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   处理字符串指令
 * @param   self 通信对象
 * @param   frame 指令帧
 * @retval  None
 */
static void _process_string_cmd(Comms* self, char* frame) {
    if(strncmp(frame, "LIFTER:", 7) == 0) {
        float val = (float)atof(frame + 7);
        self->_lift_->set_height(self->_lift_, val);
    }
    else if(strncmp(frame, "GRIPPER:", 8) == 0) {
        char* cmd = frame + 8;
        char* param = strchr(cmd, ':');
        if(param) {
            *param = '\0';
            param++;
        }

        if(strcmp(cmd, "OPEN") == 0)
            self->_grip_->open(self->_grip_);
        else if(strcmp(cmd, "CLOSE") == 0)
            self->_grip_->close(self->_grip_);
        else if(strcmp(cmd, "POS") == 0 && param)
            self->_grip_->set_pos(self->_grip_, (uint16_t)atoi(param));
    }
}

/**
 * @brief   初始化通信模块
 * @param   self 通信对象
 * @param   uart 串口对象
 * @param   lift 升降机对象
 * @param   grip 夹爪对象
 * @retval  None
 */
static void _init(Comms* self, usart_t* uart, LiftControl* lift, Gripper* grip) {
    self->_uart_ = uart;
    self->_lift_ = lift;
    self->_grip_ = grip;
    self->_cmd_idx_ = 0;
    self->_parsing_ = 0;
}

/**
 * @brief   通信处理循环
 * @param   self 通信对象
 * @retval  None
 */
static void _process(Comms* self) {
    uint8_t byte;

    while(usart_read_byte(self->_uart_, &byte)) {

        /* 单字节电机指令 (仅在非字符串解析状态下有效) */
        if(!self->_parsing_) {
            if(byte == 0x01) {
                self->_lift_->manual(self->_lift_, LiftDirUp_e);
                continue;
            }
            else if(byte == 0x02) {
                self->_lift_->manual(self->_lift_, LiftDirDown_e);
                continue;
            }
            else if(byte == 0x00) {
                self->_lift_->manual(self->_lift_, LiftDirStop_e);
                continue;
            }
        }

        /* 字符串帧起始 */
        if(byte == '$') {
            self->_parsing_ = 1;
            self->_cmd_idx_ = 0;
            memset(self->_cmd_buf_, 0, sizeof(self->_cmd_buf_));
            continue;
        }

        /* 字符串帧解析中 */
        if(self->_parsing_) {
            if(byte == '#') {
                self->_cmd_buf_[self->_cmd_idx_] = '\0';
                _process_string_cmd(self, self->_cmd_buf_);
                self->_parsing_ = 0;
            }
            else {
                if(self->_cmd_idx_ < COMMS_CMD_BUF_SIZE - 1)
                    self->_cmd_buf_[self->_cmd_idx_++] = (char)byte;
                else
                    self->_parsing_ = 0;     /* 溢出, 丢弃 */
            }
        }
    }
}
