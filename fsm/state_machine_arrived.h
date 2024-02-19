#ifndef __STATE_MACHINE_ARRIVED_H__
#define __STATE_MACHINE_ARRIVED_H__

//ARRIVED fsm  STATES
typedef enum {
    STATE_PAGE0,
    STATE_PAGE1,
    STATE_PAGE2,
    STATE_PAGE3,
    STATE_PAGE4,
    NUM_STATES_ARRIVED
}State_t_arrived;

//ARRIVED fsm associated structure
typedef struct{
    State_t_arrived state_arrived;
    void (*state_function_arrived)();
} StateMachine_t_arrived;


//functions associated to the ARRIVED fsm structure
void fn_PAGE0();
void fn_PAGE1();
void fn_PAGE2();
void fn_PAGE3();
void fn_PAGE4();

extern State_t_arrived current_state_arrived;

#endif
