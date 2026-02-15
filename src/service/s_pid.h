/**
 * @file    s_pid.h
 * @brief   PID 控制器
 *          支持: 输出限幅, 积分分离, 死区, 微分滤波,
 *                微分先行, 积分抗饱和, 输出变化率限制, 前馈控制
 * @note
 *          -------- 基础用法 --------
 *          PID_t pid = pid_create();
 *          pid.init(&pid, PID_MODE_PID, PID_FEAT_NONE);
 *          pid.set_gains(&pid, 1.0f, 0.1f, 0.01f);
 *          float out = pid.calculate(&pid, target, actual, dt_s);
 *
 *          -------- 高级用法 (配置表) --------
 *          const PID_Cfg_t cfg = {
 *              .mode     = PID_MODE_PID,
 *              .features = PID_FEAT_OUTPUT_LIMIT | PID_FEAT_ANTI_WINDUP | PID_FEAT_DIFF_FILTER,
 *              .kp = 2.0f, .ki = 0.5f, .kd = 0.1f,
 *              .max_out            = 1000.0f,
 *              .integral_separation = 0.0f,
 *              .dead_band          = 0.0f,
 *              .diff_filter_alpha  = 0.3f,
 *              .output_max_rate    = 0.0f,
 *          };
 *          PID_t pid = pid_create();
 *          pid.init_cfg(&pid, &cfg);
 *          float out = pid.calculate(&pid, target, actual, dt_s);
 */
#ifndef _s_pid_h_
#define _s_pid_h_

#include <stdint.h>


// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

// PID 模式 (按位组合)
#define PID_MODE_P      0x04u   // 0b100
#define PID_MODE_I      0x02u   // 0b010
#define PID_MODE_D      0x01u   // 0b001
#define PID_MODE_PI     0x06u   // 0b110
#define PID_MODE_PD     0x05u   // 0b101
#define PID_MODE_PID    0x07u   // 0b111

// PID 功能特性 (按位组合) 
#define PID_FEAT_NONE               0x00u
#define PID_FEAT_OUTPUT_LIMIT       (1u << 0)   // 输出限幅
#define PID_FEAT_INTEGRAL_SEP       (1u << 1)   // 积分分离
#define PID_FEAT_DEADBAND           (1u << 2)   // 死区    
#define PID_FEAT_DIFF_FILTER        (1u << 3)   // 微分滤波
#define PID_FEAT_DIFF_ON_MEAS       (1u << 4)   // 微分先行
#define PID_FEAT_ANTI_WINDUP        (1u << 5)   // 积分抗饱和
#define PID_FEAT_OUTPUT_RATE_LIMIT  (1u << 6)   // 输出变化率限制
#define PID_FEAT_FEEDFORWARD        (1u << 7)   // 前馈控制
#define PID_FEAT_ALL                0xFFu

/**
 * @brief PID 配置结构体 (用于初始化)
 */
typedef struct {
    uint8_t mode;                   // PID 模式, PID_MODE_xxx
    uint8_t features;               // 功能特性, PID_FEAT_xxx 按位或
    float kp;                       // 比例系数
    float ki;                       // 积分系数
    float kd;                       // 微分系数
    float max_out;                  // 最大输出值
    float integral_separation;      // 积分分离阈值
    float dead_band;                // 死区阈值
    float diff_filter_alpha;        // 微分滤波系数 (0~1)
    float output_max_rate;          // 输出最大变化率         
} pid_cfg_t;

/**
 * @brief PID 控制器类
 */
typedef struct PID PID;
struct PID {
// public:
    uint8_t mode_;                  // PID 模式
    uint8_t features_;              // 功能特性

    float kp_;
    float ki_;
    float kd_;

    float max_out_;                 // 最大输出值
    float integral_separation_;     // 积分分离阈值
    float dead_band_;               // 死区阈值
    float diff_filter_alpha_;       // 微分滤波系数
    float output_max_rate_;         // 输出最大变化率
    float ff_value_;                // 前馈值

    float output_;                  // 当前输出
    float integral_;                // 积分累积值
    float prev_err_;                // 上一次误差

    /**
     * @brief   构造函数 (初始化函数指针)
     * @param   pid     PID 实例指针
     * @param   mode    PID 模式 (PID_MODE_xxx)
     * @param   features 功能特性 (PID_FEAT_xxx 按位或)
     */
    void(*init)(PID* pid, uint8_t mode, uint8_t features);
    /**
     * @brief   通过配置表初始化 PID 控制器
     * @param   pid PID 实例指针
     * @param   cfg 配置结构体指针
     */
    void(*init_cfg)(PID* pid, const pid_cfg_t* cfg);
    /**
     * @brief   设置 PID 增益
     * @param   pid PID 实例指针
     * @param   kp  比例系数
     * @param   ki  积分系数
     * @param   kd  微分系数
     */
    void(*set_gains)(PID* pid, float kp, float ki, float kd);
    /**
     * @brief   设置高级参数
     * @param   pid                 PID 实例指针
     * @param   max_out             最大输出值
     * @param   integral_separation 积分分离阈值
     * @param   dead_band           死区阈值
     * @param   diff_filter_alpha   微分滤波系数 (0~1)
     * @param   output_max_rate     输出最大变化率
     */
    void(*set_params)(PID* pid, float max_out, float integral_separation,
        float dead_band, float diff_filter_alpha, float output_max_rate);
    /**
     * @brief   设置前馈值
     * @param   pid      PID 实例指针
     * @param   ff_value 前馈值
     */
    void(*set_feedforward)(PID* pid, float ff_value);
    /**
     * @brief   计算 PID 输出
     * @param   pid    PID 实例指针
     * @param   target 目标值
     * @param   actual 实际值
     * @param   dt_s   时间间隔 (秒); 0 时积分离散累加, 微分项不计算
     * @return  PID 输出值
     */
    float(*calculate)(PID* pid, float target, float actual, float dt_s);
    /**
     * @brief   重置 PID 控制器状态 (不改变参数)
     * @param   pid PID 实例指针
     */
    void(*reset)(PID* pid);

// private:
    float _filtered_diff_;
    float _prev_output_;
    float _prev_measurement_;
};

// ! ========================= 接 口 函 数 声 明 ========================= ! //

PID pid_create(void);

#endif
