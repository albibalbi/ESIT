#ifndef __STATE_MACHINE_RUN_H__
#define __STATE_MACHINE_RUN_H__

typedef enum {
    STATE_DESELECT,
    STATE_RIGHT,
    STATE_LEFT,
    NUM_STATES_RUN
}State_t_run;

typedef struct{
    State_t_run state_run;
    void (*state_function_run)();
} StateMachine_t_run;

void fn_DESELECT();
void fn_RIGHT();
void fn_LEFT();

#endif
