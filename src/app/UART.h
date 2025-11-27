#ifndef _UART_H_
#define _UART_H_

#include "stm32f10x.h"

#define uart1_board	115200
#define uart2_board	115200

void UART1_Init(void);
void UART2_Init(void);

void UART1_Process(void);

void UART2_SendString(char* str);
void Control_Gripper(char* command, char* param);

#endif
