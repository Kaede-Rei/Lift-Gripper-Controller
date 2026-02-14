/**
 * @file  d_lift_motor.c
 * @brief 升降台电机驱动实现
 *        PB0 / PB1 控制:
 *          Stop: PB0=1, PB1=1
 *          Up:   PB0=1, PB1=0
 *          Down: PB0=0, PB1=1
 */
#include "d_lift_motor.h"

// ! ========================= 变 量 声 明 ========================= ! //



// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void _init(LiftMotor* self);
static void _set_dir(LiftMotor* self, LiftDir_e dir);
static void _stop(LiftMotor* self);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 LiftMotor 对象
 * @param   None
 * @retval  LiftMotor 对象
 */
LiftMotor lift_motor_create(void) {
    LiftMotor obj;
    obj.init = _init;
    obj.set_dir = _set_dir;
    obj.stop = _stop;
    return obj;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   初始化电机
 * @param   self 电机对象
 * @retval  None
 */
static void _init(LiftMotor* self) {
    (void)self;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpio);

    /* 默认停止 */
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

/**
 * @brief   设置电机方向
 * @param   self 电机对象
 * @param   dir 方向
 * @retval  None
 */
static void _set_dir(LiftMotor* self, LiftDir_e dir) {
    (void)self;
    switch(dir) {
        case LiftDirUp_e:
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
            GPIO_ResetBits(GPIOB, GPIO_Pin_1);
            break;
        case LiftDirDown_e:
            GPIO_ResetBits(GPIOB, GPIO_Pin_0);
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            break;
        case LiftDirStop_e:
        default:
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            break;
    }
}

/**
 * @brief   停止电机
 * @param   self 电机对象
 * @retval  None
 */
static void _stop(LiftMotor* self) {
    (void)self;
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
}
