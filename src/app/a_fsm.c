/**
 * @file    a_fsm.c
 * @brief   有限状态机实现
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

#include "a_fsm.h"
#include "a_board.h"

#include <math.h>
#include <stdbool.h>

// ! ========================= 变 量 声 明 ========================= ! //

event_e cur_event = EVENT_NONE;
State* cur_state = &state_idle;

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static State* dispatch_event(State* state, event_e e);
static State* find_lca(State* s1, State* s2);
static void exit_up_to(State* from, State* to);
static void enter_down_to(State* from, State* to);
static void execute_action(State* state);

/**
 * @brief   正常状态
 */
static State* normal_handle_event(event_e e);
static void normal_action(void);
State state_normal = {
    .handle_event = normal_handle_event,
    .action = normal_action,
    .entry = 0,
    .exit = 0,

    .name_ = "normal",
    ._parent_ = 0,
};

/**
 * @brief   空闲状态
 */
static State* idle_handle_event(event_e e);
static void idle_action(void);
State state_idle = {
    .handle_event = idle_handle_event,
    .action = idle_action,
    .entry = 0,
    .exit = 0,

    .name_ = "idle",
    ._parent_ = &state_normal,
};

/**
 * @brief   升降台移动状态
 */
static State* lift_moving_handle_event(event_e e);
static void lift_moving_action(void);
static void lift_moving_entry(void);
static void lift_moving_exit(void);
State state_lift_moving = {
    .handle_event = lift_moving_handle_event,
    .action = lift_moving_action,
    .entry = lift_moving_entry,
    .exit = lift_moving_exit,

    .name_ = "lift_moving",
    ._parent_ = &state_normal,
};

/**
 * @brief   错误状态
 */
static State* error_handle_event(event_e e);
static void error_entry(void);
State state_error = {
    .handle_event = error_handle_event,
    .action = 0,
    .entry = error_entry,
    .exit = 0,

    .name_ = "error",
    ._parent_ = 0,
};

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief   FSM 处理函数
 */
void a_fsm_process(void) {
    // 无事件时执行当前状态的持续动作
    if(cur_event == EVENT_NONE) {
        execute_action(cur_state);
        return;
    }

    // 根据当前事件和状态获取下一个状态
    State* next_state = dispatch_event(cur_state, cur_event);
    if(next_state != cur_state) {
        // 找到最近公共祖先状态
        State* lca = find_lca(cur_state, next_state);

        // 从当前状态退出到最近公共祖先状态, 再从最近公共祖先状态进入到下一个状态
        exit_up_to(cur_state, lca);
        enter_down_to(lca, next_state);

        // 状态转移并清除事件
        cur_state = next_state;
        cur_event = EVENT_NONE;
    }

    // 状态持续动作
    execute_action(cur_state);
}

/**
 * @brief   触发事件
 * @param   e 事件
 */
void a_fsm_trigger_event(event_e e) {
    cur_event = e;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief   FSM 事件分发函数
 * @retval  下一个状态
 * @note    根据当前状态和事件返回下一个状态, 寻找顺序为: 当前状态 -> 父状态 -> 祖父状态 -> ... -> 根状态
 */
static State* dispatch_event(State* state, event_e e) {
    State* s = state;
    // 从当前状态开始向上查找, 直到找到一个状态能够处理该事件或者到达根状态
    while(s) {
        if(s->handle_event) {
            State* next = s->handle_event(e);
            if(next) {
                return next;
            }
        }
        s = s->_parent_;
    }
    // 没有状态处理该事件, 保持当前状态不变
    return state;
}

/**
 * @brief   查找两个状态的最近公共祖先状态
 * @param   s1 状态 1
 * @param   s2 状态 2
 * @retval  最近公共祖先状态
 */
static State* find_lca(State* s1, State* s2) {
    if(!s1 || !s2) return 0;

    int depth1 = 0, depth2 = 0;

    State* p = s1;
    while(p) { depth1++; p = p->_parent_; }
    p = s2;
    while(p) { depth2++; p = p->_parent_; }

    State* deeper = depth1 > depth2 ? s1 : s2;
    State* shallower = depth1 > depth2 ? s2 : s1;
    int diff = (int)fabs(depth1 - depth2);

    while(diff--) { deeper = deeper->_parent_; }
    while(deeper != shallower) {
        deeper = deeper->_parent_;
        shallower = shallower->_parent_;
    }

    return deeper;
}

/**
 * @brief   依次执行从状态 from 到状态 to 的所有退出动作
 * @param   from 初始状态
 * @param   to 最终状态
 */
static void exit_up_to(State* from, State* to) {
    State* s = from;
    while(s && s != to) {
        if(s->exit) {
            s->exit();
        }
        s = s->_parent_;
    }
}

/**
 * @brief   依次从状态 from 到状态 to 执行所有进入动作
 * @param   from 初始状态
 * @param   to 最终状态
 */
static void enter_down_to(State* from, State* to) {
    State* path[FSM_DEPTH];
    int depth = 0;
    State* s = to;

    // 找到从 to 到 from 的路径
    while(s && s != from) {
        path[depth++] = s;
        s = s->_parent_;
    }

    // 依次执行
    while(depth--) {
        s = path[depth];
        if(s->entry) {
            s->entry();
        }
    }
}

/**
 * @brief   执行当前状态及其所有祖先状态的持续动作
 */
static void execute_action(State* state) {
    State* s = state;
    while(s) {
        if(s->action) {
            s->action();
        }
        s = s->_parent_;
    }
}

/**
 * @brief   正常状态事件处理函数
 * @param   e 事件
 * @retval  下一个状态
 */
static State* normal_handle_event(event_e e) {
    switch(e) {
        case EVENT_ERROR:
            return &state_error;
        default:
            return 0;
    }
}

/**
 * @brief   正常状态持续动作函数
 */
static void normal_action(void) {
    s_wireless_comms_process();

    if(tick.flag) {
        tick.flag = 0;
        lift_encoder.update(&lift_encoder);
    }
}

/**
 * @brief   空闲状态事件处理函数
 * @param   e 事件
 * @retval  下一个状态
 */
static State* idle_handle_event(event_e e) {
    switch(e) {
        case EVENT_LIFT_MOVE:
            return &state_lift_moving;
        default:
            return 0;
    }
}

/**
 * @brief   空闲状态持续动作函数
 */
static void idle_action(void) {
    if(fabsf(lift_target_pos_mm - lift_encoder.get_position(&lift_encoder)) > 5.0f) {
        a_fsm_trigger_event(EVENT_LIFT_MOVE);
    }
}

/**
 * @brief   升降台移动状态事件处理函数
 * @param   e 事件
 * @retval  下一个状态
 */
static State* lift_moving_handle_event(event_e e) {
    switch(e) {
        case EVENT_LIFT_STOP:
            return &state_idle;
        default:
            return 0;
    }
}

/**
 * @brief   升降台移动状态进入动作函数
 */
static void lift_moving_entry(void) {
    printf("$LIFT:START#");
}

/**
 * @brief   升降台移动状态退出动作函数
 */
static void lift_moving_exit(void) {
    printf("$LIFT:END#");
}

/**
 * @brief   升降台移动状态动作函数
 */
static void lift_moving_action(void) {
    float target = lift_target_pos_mm;
    float current = lift_encoder.get_position(&lift_encoder);

    if(target - current > 5.0f) {
        lift_relay.set_dir(&lift_relay, RelayDirA);
    }
    else if(target - current < -5.0f) {
        lift_relay.set_dir(&lift_relay, RelayDirB);
    }
    else {
        lift_relay.stop(&lift_relay);
        a_fsm_trigger_event(EVENT_LIFT_STOP);
    }
}

/**
 * @brief   错误状态事件处理函数
 * @param   e 事件
 * @retval  下一个状态
 */
static State* error_handle_event(event_e e) {
    switch(e) {
        case EVENT_OK:
            return &state_idle;
        default:
            return 0;
    }
}

/**
 * @brief   错误状态进入动作函数
 */
static void error_entry(void) {
    lift_relay.stop(&lift_relay);
    gripper.open(&gripper);
    a_fsm_trigger_event(EVENT_OK);
}
