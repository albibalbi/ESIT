#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

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

#endif
