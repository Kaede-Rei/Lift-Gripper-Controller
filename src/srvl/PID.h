#ifndef _PID_H_
#define _PID_H_

#include "main.h"


typedef struct _PositionPID {
    float Kp;         
    float Ki;         
    float Kd;         
} PositionPID;

typedef struct _SpeedPID {
    float Kp;         
    float Ki;         
    float Kd;         
} SpeedPID;

void PID_Init(PositionPID *position_pid, SpeedPID *speed_pid, float Kp_pos, float Ki_pos, float Kd_pos, float Kp_speed, float Ki_speed, float Kd_speed) ;

int Position_PID(PositionPID pid, float position, float target_position);
int Speed_PID(SpeedPID pid, float expect_speed, float actual_speed);
int Cascade_PID_Control(PositionPID *pos_pid, SpeedPID *speed_pid, 
                       int current_pos, int target_pos, int current_spd);


#endif
