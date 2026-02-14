/**
 * @file  s_pid.c
 * @brief PID 控制器实现
 */
#include "s_pid.h"

// ! ========================= 变 量 声 明 ========================= ! //



// ! ========================= 私 有 函 数 声 明 ========================= ! //

#define PID_ABS(x)              ((x) >= 0.0f ? (x) : -(x))
#define PID_CLAMP(v, lo, hi)    ((v) > (hi) ? (hi) : ((v) < (lo) ? (lo) : (v)))

static void _init(PID* pid, uint8_t mode, uint8_t features);
static void _init_cfg(PID* pid, const pid_cfg_t* cfg);
static void _set_gains(PID* pid, float kp, float ki, float kd);
static void _set_params(PID* pid, float max_out, float integral_separation,
    float dead_band, float diff_filter_alpha, float output_max_rate);
static void _set_feedforward(PID* pid, float ff_value);
static float _calculate(PID* pid, float target, float actual, float dt_s);
static void _reset(PID* pid);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   创建 PID 实例
 * @return  PID 实例
 */
PID pid_create(void) {
    PID pid;

    pid.init = _init;
    pid.init_cfg = _init_cfg;
    pid.set_gains = _set_gains;
    pid.set_params = _set_params;
    pid.set_feedforward = _set_feedforward;
    pid.calculate = _calculate;
    pid.reset = _reset;

    return pid;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   初始化 PID 控制器
 * @param   pid      PID 实例指针
 * @param   mode     PID 模式 (PID_MODE_xxx)
 * @param   features 功能特性 (PID_FEAT_xxx 按位或)
 */
static void _init(PID* pid, uint8_t mode, uint8_t features) {
    pid->mode_ = mode;
    pid->features_ = features;

    pid->kp_ = 0.0f;  pid->ki_ = 0.0f;  pid->kd_ = 0.0f;

    pid->max_out_ = 0.0f;
    pid->integral_separation_ = 0.0f;
    pid->dead_band_ = 0.0f;
    pid->diff_filter_alpha_ = 0.0f;
    pid->output_max_rate_ = 0.0f;
    pid->ff_value_ = 0.0f;

    pid->output_ = 0.0f;
    pid->integral_ = 0.0f;
    pid->prev_err_ = 0.0f;

    pid->_filtered_diff_ = 0.0f;
    pid->_prev_output_ = 0.0f;
    pid->_prev_measurement_ = 0.0f;
}

/**
 * @brief   通过配置表初始化 PID 控制器
 * @param   pid PID 实例指针
 * @param   cfg 配置结构体指针
 */
static void _init_cfg(PID* pid, const pid_cfg_t* cfg) {
    _init(pid, cfg->mode, cfg->features);
    pid->kp_ = cfg->kp;
    pid->ki_ = cfg->ki;
    pid->kd_ = cfg->kd;
    pid->max_out_ = cfg->max_out;
    pid->integral_separation_ = cfg->integral_separation;
    pid->dead_band_ = cfg->dead_band;
    pid->diff_filter_alpha_ = cfg->diff_filter_alpha;
    pid->output_max_rate_ = cfg->output_max_rate;
}

/**
 * @brief   设置 PID 增益
 */
static void _set_gains(PID* pid, float kp, float ki, float kd) {
    pid->kp_ = kp;
    pid->ki_ = ki;
    pid->kd_ = kd;
}

/**
 * @brief   设置高级参数
 */
static void _set_params(PID* pid, float max_out, float integral_separation,
    float dead_band, float diff_filter_alpha, float output_max_rate) {
    pid->max_out_ = max_out;
    pid->integral_separation_ = integral_separation;
    pid->dead_band_ = dead_band;
    pid->diff_filter_alpha_ = diff_filter_alpha;
    pid->output_max_rate_ = output_max_rate;
}

/**
 * @brief   设置前馈值
 */
static void _set_feedforward(PID* pid, float ff_value) {
    pid->ff_value_ = ff_value;
}

/**
 * @brief   计算 PID 输出
 * @param   pid    PID 实例指针
 * @param   target 目标值
 * @param   actual 实际值
 * @param   dt_s   时间间隔 (秒); 0 时积分离散累加, 微分项不计算
 * @return  PID 输出值
 */
float _calculate(PID* pid, float target, float actual, float dt_s) {
    float err = target - actual;
    uint8_t feat = pid->features_;
    uint8_t mode = pid->mode_;

    /* 死区 */
    if((feat & PID_FEAT_DEADBAND) && PID_ABS(err) < pid->dead_band_) {
        err = 0.0f;
    }

    float out = 0.0f;

    /* 比例项 */
    if(mode & PID_MODE_P) {
        out += pid->kp_ * err;
    }

    /* 积分项 */
    if(mode & PID_MODE_I) {
        uint8_t allow_integral = 1;
        uint8_t allow_separation = 0;

        /* 积分抗饱和 : 条件积分法 (输出饱和且误差同向时禁止积分) */
        if(feat & PID_FEAT_ANTI_WINDUP) {
            if(pid->_prev_output_ >= pid->max_out_ && err > 0.0f) allow_integral = 0;
            if(pid->_prev_output_ <= -pid->max_out_ && err < 0.0f) allow_integral = 0;
        }

        if(allow_integral) {
            pid->integral_ += (dt_s > 0.0f) ? (err * dt_s) : err;
        }

        /* 积分分离 (误差过大时不叠加积分输出) */
        if(feat & PID_FEAT_INTEGRAL_SEP) {
            if(PID_ABS(err) > pid->integral_separation_) {
                allow_separation = 1;
            }
        }

        if(!allow_separation) {
            out += pid->ki_ * pid->integral_;
        }
    }

    /* 微分项 */
    if(mode & PID_MODE_D) {
        float diff;

        /* 微分先行: 基于测量值变化率, 避免目标突变时 D 项跳变 */
        if(feat & PID_FEAT_DIFF_ON_MEAS) {
            diff = (dt_s > 0.0f) ? (-(actual - pid->_prev_measurement_) / dt_s) : 0.0f;
            pid->_prev_measurement_ = actual;
        }
        else {
            diff = (dt_s > 0.0f) ? ((err - pid->prev_err_) / dt_s) : 0.0f;
            pid->prev_err_ = err;
        }

        /* 微分滤波: 一阶低通 */
        if(feat & PID_FEAT_DIFF_FILTER) {
            diff = pid->diff_filter_alpha_ * diff
                + (1.0f - pid->diff_filter_alpha_) * pid->_filtered_diff_;
            pid->_filtered_diff_ = diff;
        }

        out += pid->kd_ * diff;
    }

    /* 前馈 */
    if(feat & PID_FEAT_FEEDFORWARD) {
        out += pid->ff_value_;
    }

    /* 保存未限幅输出, 用于反计算法抗饱和 */
    float total_output = out;

    /* 输出限幅 */
    if(feat & PID_FEAT_OUTPUT_LIMIT) {
        out = PID_CLAMP(out, -pid->max_out_, pid->max_out_);
    }

    /* 输出变化率限制 */
    if(feat & PID_FEAT_OUTPUT_RATE_LIMIT) {
        if(dt_s > 0.0f) {
            float max_change = pid->output_max_rate_ * dt_s;
            float delta = out - pid->_prev_output_;
            if(PID_ABS(delta) > max_change) {
                out = pid->_prev_output_ + (delta > 0.0f ? max_change : -max_change);
            }
        }
    }

    /* 积分抗饱和 : 反计算法 (back-calculation) */
    if((mode & PID_MODE_I) && (feat & PID_FEAT_ANTI_WINDUP) && (feat & PID_FEAT_OUTPUT_LIMIT)) {
        float output_diff = total_output - out;
        if(PID_ABS(pid->kp_) > 1e-6f && PID_ABS(pid->ki_) > 1e-6f) {
            float Kb = pid->ki_ / pid->kp_;  /* Kb = 1/Tt = Ki/Kp */
            pid->integral_ -= output_diff * Kb * dt_s;
        }
    }

    pid->output_ = out;
    pid->_prev_output_ = out;

    return out;
}

/**
 * @brief   重置 PID 控制器状态 (不改变参数)
 */
void _reset(PID* pid) {
    pid->output_ = 0.0f;
    pid->integral_ = 0.0f;
    pid->prev_err_ = 0.0f;
    pid->_filtered_diff_ = 0.0f;
    pid->_prev_output_ = 0.0f;
    pid->_prev_measurement_ = 0.0f;
}
