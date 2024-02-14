#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__
#include <stdbool.h>

typedef enum {
    STATE_FIXING,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSE,
    STATE_ARRIVED,
    NUM_STATES
}State_t;

typedef struct{
    State_t state;
    void (*state_function)();
} StateMachine_t;

void fn_FIXING();
void fn_IDLE();
void fn_RUNNING();
void fn_PAUSE();
void fn_ARRIVED();

State_t current_state;
StateMachine_t fsm[];


typedef enum {
    SW_NONE,
    UP_PRESSED,
    DOWN_PRESSED,
    RESTART_PRESSED
} Event_t;

Event_t event;

typedef enum Color{
    OFF, GREEN, RED, BLUE, PURPLE, YELLOW, WHITE
}Color;
void LED_set_color(Color color);


void TA1_0_IRQHandler(void);



#endif
