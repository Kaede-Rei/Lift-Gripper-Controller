#include "main.h"
#include <stdlib.h>

extern void UART1_Init(void);
extern uint8_t tim3_flag;

float height_difference = 0.0f; //从串口接收的高度差
int current_position = 0;
int target_position = 0;
int current_speed = 0;
int pwm_output = 0;

uint8_t rcvd_flag = 0;

PositionPID position_pid;  
SpeedPID speed_pid;

int lift = 0;

//测试
void Verify_Calibration(void)
{
    printf("最大脉冲数: %d\r\n", ACTUAL_MAX_PULSE);
    printf("最大行程: %d mm\r\n", ACTUAL_MAX_MM);
    printf("脉冲距离比: %.3f 脉冲/mm\r\n", ACTUAL_PULSE_PER_MM);
    printf("行程范围: %d - %d mm\r\n", MIN_STROKE_MM, MAX_STROKE_MM);
    printf("脉冲范围: %d - %d 脉冲\r\n", MIN_STROKE_PULSE, MAX_STROKE_PULSE);
}

int main(void)
{
    Encoder_Init();
    board_init();
		
//	UART1_Init();
	
	PID_Init(&position_pid, &speed_pid, 0.8f, 0.01f, 0.5f, 3.0f, 10.0f, 0.0f);
	
//	Verify_Calibration();
	
    while (1)
    {
         if(tim3_flag){
            // 读取编码器数据
            Encoder_CalcPositionAndSpeed(&current_position, &current_speed);

						// 根据高度差计算目标位置
						if(rcvd_flag == 1){
								target_position = current_position + (int)height_difference;
								rcvd_flag = 0;
						}
            if (height_difference != 0.0f) { 
								int error = target_position - current_position;
								printf("target_Position, current_position, height_defference, err: %d, %d, %f, %d\r\n", target_position, current_position, height_difference, error);
								
								// 死区检查（5个脉冲）
								if (abs(error) > 0) {
										// 控制电机方向
										if (error > 5) {
												GPIO_ResetBits(GPIOB, GPIO_Pin_1); //上升
												GPIO_SetBits(GPIOB, GPIO_Pin_0);
										}
										else if (error < -5) {
												GPIO_SetBits(GPIOB, GPIO_Pin_1); //下降
												GPIO_ResetBits(GPIOB, GPIO_Pin_0);
										}
								}
								else {
										// 在死区内，停止
										GPIO_SetBits(GPIOB, GPIO_Pin_0);
										GPIO_SetBits(GPIOB, GPIO_Pin_1);
										printf("$LIFTER:OK#\r\n");
										height_difference = 0.0f;
								}							
            } 

			
			// 手动控制（调试）
            if(lift == 1){
                GPIO_ResetBits(GPIOB, GPIO_Pin_1); //上升
                GPIO_SetBits(GPIOB, GPIO_Pin_0);
            }
            else if(lift == -1){
                GPIO_SetBits(GPIOB, GPIO_Pin_1);  //下降
                GPIO_ResetBits(GPIOB, GPIO_Pin_0);
            }
			
			
             tim3_flag = 0;
         }	
    }
}
