/**
 * @file  main.c
 * @brief 程序入口
 */
#include "a_board.h"

int main(void) {
    a_board_init();

    while(1) {
        a_fsm_process();
    }
}
