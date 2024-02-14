
#include "state_machine.h"
#include "booster_pack_pin_map.h"
#include "lcd_function.h"
#include "race.h"
#include "gps.h"
#include "UART_function.h"

//control variables
volatile bool timer_a_edge=0;
volatile bool wifi_conn =0;

volatile bool transmitted=0;

void fn_FIXING(){

    LED_set_color(RED);

    lcd_show_state(current_state);

    if (1){
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_FIXING;
    }
}

//waiting runs start (play button)
void fn_IDLE(){
    LED_set_color(GREEN);

    lcd_show_state(current_state);

    transmitted=0;
    if (event==UP_PRESSED){
        event=SW_NONE;
        race_init(&race);
        race_set_start_date_time(&race, gps_get_date(&gps), gps_get_time(&gps));
        current_state = STATE_RUNNING;
    }else{
        current_state = STATE_IDLE;
    }
}

void fn_RUNNING(){
    LED_set_color(BLUE);

    lcd_show_state(current_state);

    if (event==UP_PRESSED){
        event=SW_NONE;
        current_state = STATE_PAUSE;
    }else if(event==RESTART_PRESSED){
        current_state = STATE_IDLE;
    }else{
        if (gps_data_valid(&gps) && gps_get_fix(&gps)){
            RACE_set_run_values(&race,&gps);
            if (timer_a_edge){
                timer_a_edge = !timer_a_edge;
                race_set_positions(&race, gps_get_positions(gps));
            }
        }
        current_state = STATE_RUNNING;
    }
}

void fn_PAUSE(){
    LED_set_color(PURPLE);

    lcd_show_state(current_state);

    if (event==UP_PRESSED){
        event=SW_NONE;
        race_set_pause_duration(&race, gps_get_time(&gps));
        current_state = STATE_RUNNING;
    }else if (event==DOWN_PRESSED){
        event=SW_NONE;
        current_state = STATE_ARRIVED;
    }else if(event==RESTART_PRESSED){
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_PAUSE;
    }
}

void fn_ARRIVED(){


    LED_set_color(YELLOW);
    lcd_show_state(current_state);

    if (!transmitted  && wifi_conn){
        transmitted =1;
        UART_disableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

        UART_transmit_all_data_race(EUSCI_A0_BASE, &race);
        UART_transmit_all_data_race(EUSCI_A2_BASE, &race);

        UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

        race_init(&race);
        UART_transmit_string(EUSCI_A0_BASE,"\r\n");
    }if(event==DOWN_PRESSED){
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_ARRIVED;
     }

}


void LED_set_color(Color color){
    switch(color){
        case OFF:
            GPIO_setOutputLowOnPin(RGB_LED_GREEN); GPIO_setOutputLowOnPin(RGB_LED_RED);GPIO_setOutputLowOnPin(RGB_LED_BLUE);
            break;
        case GREEN:
            GPIO_setOutputHighOnPin(RGB_LED_GREEN); GPIO_setOutputLowOnPin(RGB_LED_RED);GPIO_setOutputLowOnPin(RGB_LED_BLUE);
            break;
        case RED:
            GPIO_setOutputHighOnPin(RGB_LED_RED); GPIO_setOutputLowOnPin(RGB_LED_GREEN);GPIO_setOutputLowOnPin(RGB_LED_BLUE);
            break;
        case BLUE:
            GPIO_setOutputHighOnPin(RGB_LED_BLUE); GPIO_setOutputLowOnPin(RGB_LED_RED);GPIO_setOutputLowOnPin(RGB_LED_GREEN);
            break;
        case PURPLE:
            GPIO_setOutputHighOnPin(RGB_LED_BLUE); GPIO_setOutputHighOnPin(RGB_LED_RED);GPIO_setOutputLowOnPin(RGB_LED_GREEN);
            break;
        case YELLOW:
            GPIO_setOutputHighOnPin(RGB_LED_RED); GPIO_setOutputHighOnPin(RGB_LED_GREEN);GPIO_setOutputLowOnPin(RGB_LED_BLUE);
            break;
        case WHITE:
            GPIO_setOutputHighOnPin(RGB_LED_BLUE); GPIO_setOutputHighOnPin(RGB_LED_RED);GPIO_setOutputHighOnPin(RGB_LED_GREEN);
            break;
    }
}



void TA1_0_IRQHandler(void)
{
    //chek wifi connection
    if (GPIO_getInputPinValue (WIFI_CONN)){
        wifi_conn = 1;
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }else{
        wifi_conn = 0;
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
    //timer edge for positions sampling
    timer_a_edge = 1;
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}
