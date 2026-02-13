#ifndef _Encoder_h_
#define _Encoder_h_

#include "main.h"


#define ACTUAL_MAX_PULSE       15518     // ʵ�ʲ��������������
#define ACTUAL_MAX_MM          2609      // ����г�1000mm
#define ACTUAL_PULSE_PER_MM    37.48f   // ʵ������/mm�� 

#define MIN_STROKE 0         
#define MAX_STROKE 1000  

#define MIN_STROKE_PULSE       (int)(MIN_STROKE_MM * ACTUAL_PULSE_PER_MM)    // 776����
#define MAX_STROKE_PULSE       (int)(MAX_STROKE_MM * ACTUAL_PULSE_PER_MM)    // 15518����

extern int current_position;  
extern int target_position;  

void Encoder_Init(void);
int Encoder_Read(void);
void Encoder_CalcPositionAndSpeed(int* position,int* speed);
float CalcHeightDifference(void);


#endif
