#include "Encoder.h"

#define ENCODER_PPR       1000    // 编码器每转脉冲数
#define SAMPLING_PERIOD   10      // 采样周期 (ms)


void Encoder_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitTypeDef TIM_TimeInitStructure;
    TIM_TimeInitStructure.TIM_Period = 65536 - 1;
    TIM_TimeInitStructure.TIM_Prescaler = 1 - 1;
    TIM_TimeInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeInitStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICFilter = 0xF;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);

    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    TIM_Cmd(TIM2, ENABLE);
}

int Encoder_Read(void)
{
    int cnt = TIM2->CNT;
    TIM_SetCounter(TIM2, 0);
    return cnt;
}

// 计算位置和速度
void Encoder_CalcPositionAndSpeed(int* position,int* speed)
{
	static int32_t total_pulses = 0;
    int temp = Encoder_Read();
	
    if(temp < 32767){
        total_pulses += temp;
    }
    else{
        total_pulses -= (65536 - temp); 
    }

	// 计算当前位置（mm）
	*position = (int)((float)total_pulses / ACTUAL_PULSE_PER_MM + 0.5f);
    
    // 计算当前速度（mm/s）
	*speed = (temp / ACTUAL_PULSE_PER_MM) / (SAMPLING_PERIOD / 1000.0f);
}


