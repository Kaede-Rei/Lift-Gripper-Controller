#include "UART.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern int target_position;
extern int lift;
extern float height_difference;
extern uint8_t rcvd_flag;

#define RING_BUFFER_SIZE 128
static uint8_t rx_ring_buffer[RING_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static char str_cmd_buffer[64];
static uint8_t str_cmd_index = 0;
static uint8_t is_parsing_string = 0;

static uint8_t legacy_cmd_step = 0;
static uint8_t legacy_data_buf[3] = { 0 };

void UART2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    // TX (PA2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX (PA3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = uart2_board;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

void UART2_SendString(char* str)
{
    while(*str){
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, *str++);
    }
}

void Control_Gripper(char* command, char* param)
{
    char cmd_buffer[32];

    if(strcmp(command, "OPEN") == 0){
        sprintf(cmd_buffer, "#000P0500T1000!");
        UART2_SendString(cmd_buffer);
    }
    else if(strcmp(command, "CLOSE") == 0){
        sprintf(cmd_buffer, "#000P2500T1000!");
        UART2_SendString(cmd_buffer);
    }
    else if(strcmp(command, "POS") == 0 && param != NULL){
        sprintf(cmd_buffer, "#000P%sT1000!", param);
        UART2_SendString(cmd_buffer);
    }
}

void UART1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    USART_DeInit(USART1);
    GPIO_InitTypeDef GPIO_InitStructure;
    // TX PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = uart1_board;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
        uint8_t data = USART_ReceiveData(USART1);

        uint16_t next_head = (rx_head + 1) % RING_BUFFER_SIZE;

        if(next_head != rx_tail){
            rx_ring_buffer[rx_head] = data;
            rx_head = next_head;
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

static void Process_String_Command(char* frame)
{
    if(strncmp(frame, "LIFTER:", 7) == 0){
        float value = atof(frame + 7);
        height_difference = value;
        rcvd_flag = 1;
    }
    else if(strncmp(frame, "GRIPPER:", 8) == 0){
        char* cmd_start = frame + 8;
        char* param_start = strchr(cmd_start, ':');

        if(param_start){
            *param_start = '\0'; // 切断字符串
            param_start++;       // 指向参数
            Control_Gripper(cmd_start, param_start);
        }
        else{
            Control_Gripper(cmd_start, NULL);
        }
    }
}

void UART1_Process(void)
{
    uint8_t data;

    while(rx_head != rx_tail){
        data = rx_ring_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % RING_BUFFER_SIZE;

        if(!is_parsing_string && legacy_cmd_step == 0){
            if(data == 0x01){ lift = 1; }
            else if(data == 0x02){ lift = -1; }
            else if(data == 0x00){ lift = 0; }
        }

        if(data == '$'){
            is_parsing_string = 1;
            str_cmd_index = 0;
            memset(str_cmd_buffer, 0, sizeof(str_cmd_buffer));
            continue;
        }

        if(is_parsing_string){
            if(data == '#'){
                str_cmd_buffer[str_cmd_index] = '\0'; // 结束符
                Process_String_Command(str_cmd_buffer);
                is_parsing_string = 0; // 结束解析
            }
            else{
                if(str_cmd_index < sizeof(str_cmd_buffer) - 1){
                    str_cmd_buffer[str_cmd_index++] = data;
                }
                else{
                    is_parsing_string = 0;
                }
            }
            continue;
        }

        if(data == 0x0F && legacy_cmd_step == 0){
            legacy_cmd_step = 1;
        }
        else if(legacy_cmd_step >= 1 && legacy_cmd_step <= 3){
            legacy_data_buf[legacy_cmd_step - 1] = data;
            legacy_cmd_step++;
        }
        else if(legacy_cmd_step == 4 && data == 0x1F){
            target_position = (int)(legacy_data_buf[0] * 100 + legacy_data_buf[1] * 10 + legacy_data_buf[2]);
            legacy_cmd_step = 0;
        }
        else if(legacy_cmd_step != 0){
            legacy_cmd_step = 0;
        }
    }
}

#pragma import(__use_no_semihosting)             
struct __FILE{ int handle; };
FILE __stdout;
void _sys_exit(int x) { x = x; }

int fputc(int ch, FILE* f)
{
    while((USART1->SR & 0X40) == 0){ };
    USART1->DR = (u8)ch;
    return ch;
}
