#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

//main fsm STATES
typedef enum {
    STATE_FIXING,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSE,
    STATE_ARRIVED,
    NUM_STATES
}State_t;

//main fsm structure
typedef struct{
    State_t state;
    void (*state_function)();
} StateMachine_t;


//functions associated to the main fsm structure
void fn_FIXING();
void fn_IDLE();
void fn_RUNNING();
void fn_PAUSE();
void fn_ARRIVED();


extern StateMachine_t fsm[];
extern State_t current_state;

#endif
