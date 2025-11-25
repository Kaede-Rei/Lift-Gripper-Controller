#include "main.h"
#include <stdlib.h>

extern int target_position;
uint8_t cmd_flag = 0;
uint8_t buffer[3] = {0};
extern int lift;

#define RX_BUFFER_SIZE 50
char rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;
uint8_t frame_received = 0;
extern float height_difference;  
extern uint8_t rcvd_flag;

void UART1_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
	
	USART_DeInit(USART1);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(USART1, ENABLE); 
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t command = USART_ReceiveData(USART1);

        if (command == 0x01) {
            lift = 1;
            printf("Lift UP\n");
        }
        else if (command == 0x02) {
            lift = -1;
            printf("Lift DOWN\n");
        }
        else if (command == 0x00) {
            lift = 0;
            printf("Lift STOP\n");
        }

        if (command == '$') {
            rx_index = 0;
            memset(rx_buffer, 0, RX_BUFFER_SIZE);
        }

        if (rx_index < RX_BUFFER_SIZE - 1)
            rx_buffer[rx_index++] = command;

        if (command == '#') {
            frame_received = 1;
        }

        if (frame_received) {
            frame_received = 0;

            if (strncmp((char*)rx_buffer, "$LIFTER:", 8) == 0) {
                char *start = (char*)rx_buffer + 8;
                char *end = strchr(start, '#');
                if (end) *end = '\0';

                float value = atof(start);
                height_difference = value; 
								rcvd_flag = 1;
                printf("Recv LIFTER ¦¤h = %.2f\r\n", height_difference);
            }

            memset(rx_buffer, 0, RX_BUFFER_SIZE);
            rx_index = 0;
        }

        if (cmd_flag == 0 && command == 0x0F) {
            cmd_flag = 1;
        }
        else if (cmd_flag == 1) {
            buffer[0] = command;
            cmd_flag = 2;
        }
        else if (cmd_flag == 2) {
            buffer[1] = command;
            cmd_flag = 3;
        }
        else if (cmd_flag == 3) {
            buffer[2] = command;
            cmd_flag = 4;
        }
        else if (cmd_flag == 4 && command == 0x1F) {
            target_position = (int)(buffer[0] * 100 + buffer[1] * 10 + buffer[2]);
            printf("Target pos = %d\n", target_position);
            cmd_flag = 0;
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

#pragma import(__use_no_semihosting)             

struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       

void _sys_exit(int x) 
{ 
	x = x; 
}

int fputc(int ch, FILE *f)
{    
	while((USART1->SR&0X40)==0){};
		USART1->DR = (u8) ch;
	return ch;
}
