#include "main.h"
#include "UART.h"
#include <stdlib.h>
#include <stdio.h>

extern uint8_t tim3_flag;

float height_difference = 0.0f;
int current_position = 0;
int target_position = 0;
int current_speed = 0;

uint8_t rcvd_flag = 0;

PositionPID position_pid;
SpeedPID speed_pid;

int lift = 0;

static void Motor_Stop(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

static void Motor_Up(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
}

static void Motor_Down(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

int main(void)
{
    board_init();

    PID_Init(&position_pid, &speed_pid, 0.8f, 0.01f, 0.5f, 3.0f, 10.0f, 0.0f);

    static uint8_t print_divider = 0;

    while(1){
        if(tim3_flag){
            UART1_Process();
            tim3_flag = 0;

            Encoder_CalcPositionAndSpeed(&current_position, &current_speed);

            if(rcvd_flag == 1){
                target_position = current_position + (int)height_difference;
                rcvd_flag = 0;
            }

            if(lift != 0){
                if(lift == 1) Motor_Up();
                else Motor_Down();
            }
            else if(height_difference != 0.0f){
                int error = target_position - current_position;

                if(abs(error) > 5){
                    if(error > 0) Motor_Up();
                    else Motor_Down();
                }
                else{
                    Motor_Stop();
                    printf("$LIFTER:OK#\r\n");
                    height_difference = 0.0f;
                }

                if(++print_divider >= 50){
                    printf("T:%d, C:%d, Err:%d\r\n", target_position, current_position, error);
                    print_divider = 0;
                }
            }
            else{
                Motor_Stop();
            }
        }
    }
}
