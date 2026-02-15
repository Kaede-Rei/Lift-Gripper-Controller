/**
 * @file    a_fsm.h
 * @brief   有限状态机
 * @note    本实现对应于 UML 状态机图中的以下状态层次：
 *
 * [UML State Machine Diagram]
 * |
 * ├──  NormalState (state_normal)
 * |    |
 * |    ├── IdleState (state_idle)
 * |    └── LiftMovingState (state_lift_moving)
 * |
 * └──  ErrorState (state_error)
 */

#ifndef _a_fsm_h_
#define _a_fsm_h_

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

// 状态机深度
#define FSM_DEPTH 5

/**
 * @brief   事件枚举
 */
typedef enum {
    EVENT_NONE = 0,
    EVENT_OK,
    EVENT_ERROR,
    EVENT_LIFT_MOVE,
    EVENT_LIFT_STOP,
    EVENT_MAX
} event_e;

/**
 * @brief   状态结构体
 */
typedef struct State State;
struct State {
// public:
    const char* name_;

    /**
     * @brief   事件处理函数
     * @param   event 事件
     */
    State* (*handle_event)(event_e e);
    /**
     * @brief   状态持续动作函数
     */
    void(*action)(void);
    /**
     * @brief   状态进入动作函数
     */
    void(*entry)(void);
    /**
     * @brief   状态退出动作函数
     */
    void(*exit)(void);

// private:
    State* _parent_;
};

// 当前状态和事件
extern event_e cur_event;
extern State* cur_state;

/**
 * @brief   FSM 状态声明
 * @note
 *  - 正常状态
 *      - 空闲状态
 *      - 升降台移动状态
 *  - 错误状态
 */
extern State state_normal;
extern State state_idle, state_lift_moving;
extern State state_error;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void a_fsm_process(void);
void a_fsm_trigger_event(event_e e);

#endif
