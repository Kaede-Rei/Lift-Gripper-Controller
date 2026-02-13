#include "Encoder.h"

#define ENCODER_PPR       1000    // 编码器每转脉冲数
#define SAMPLING_PERIOD   10      // 采样周期 (ms)

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


