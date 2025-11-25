#include "PID.h"
#include "Encoder.h"

void PID_Init(PositionPID *position_pid, SpeedPID *speed_pid, float Kp_pos, float Ki_pos, float Kd_pos, float Kp_speed, float Ki_speed, float Kd_speed) {
    // 初始化位置环PID参数
    position_pid->Kp = Kp_pos;
    position_pid->Ki = Ki_pos;
    position_pid->Kd = Kd_pos;

    // 初始化速度环PI参数
    speed_pid->Kp = Kp_speed;
    speed_pid->Ki = Ki_speed;
    speed_pid->Kd = Kd_speed;
}

// 位置环PID控制器
int Position_PID(PositionPID pid, float current_position, float target_position) {
    static float Bias, Pwm, Integral_bias, Last_Bias;

    // 计算偏差
    Bias = target_position - current_position;
    // 求出偏差的积分
    Integral_bias += Bias;
    // 积分限幅
    if (Integral_bias > 400) 
		Integral_bias = 400;
    if (Integral_bias < -400) 
		Integral_bias = -400;

    // 位置式PID控制器
    Pwm = pid.Kp * Bias + pid.Ki * Integral_bias + pid.Kd * (Bias - Last_Bias);
    // 保存上一次偏差
    Last_Bias = Bias;

    return Pwm;
}

// 速度环PI控制器
int Speed_PID(SpeedPID pid, float expect_speed, float actual_speed)
{
    int Err, Err_LowOut;
    static int Err_Last, Integral;
    static float a = 0.7;
	
    Err = expect_speed - actual_speed;
    Err_LowOut = a*Err + (1-a)*Err_Last;
    Err_Last = Err_LowOut;
    Integral += Err_LowOut;
    Integral = Integral>+2000 ? +2000 : (Integral<-2000 ? -2000 : Integral);

    return pid.Kp*Err_LowOut + pid.Ki*Integral;
}

// 串级PID控制
int Cascade_PID_Control(PositionPID *pos_pid, SpeedPID *speed_pid, 
                       int current_pos, int target_pos, int current_spd)
{
    // 位置环输出作为速度环的期望输入
    int expect_speed = Position_PID(*pos_pid, current_pos, target_pos);
    
    // 速度环输出作为最终PWM
    int pwm_output = Speed_PID(*speed_pid, expect_speed, current_spd);
    
    return pwm_output;
}
