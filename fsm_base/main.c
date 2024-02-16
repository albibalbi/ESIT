#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "race.h"
#include "booster_pack_pin_map.h"
#include "state_machine.h"

typedef enum Color{
    OFF, GREEN, RED, BLUE, PURPLE, YELLOW, WHITE
}Color;

void GPIO_enable_pullup_all_buttons();      //enable button LEFT, RIGHT, UP, DOWN
void GPIO_enable_interrupt_all_buttons();   //enable interrupt for all buttonss
void GPIO_enable_leds();                    //enable all leds
void LED_set_color(Color color);            //set the color for RGB LED

void UART0_config(eUSCI_UART_ConfigV1 uartConfig);  //config UART instance 0 and enable interrupt. Serial monitor UART
void UART2_config(eUSCI_UART_ConfigV1 uartConfig);  //config UART instance 2 and enable interrupt. GPS UART
void UART0_transmit_gps_data();                    //transmit gps data via serial monitor (testing function)
void UART_transmit_string(uint32_t moduleInstance, const char* str);    //send string via UART
void UART_transmit_all_data_race(uint32_t moduleInstance, Race* race);  //send all data collected via UART

void INTERRUPT_enable(); //enable interrupt PORT1, EUSCI_2, TIMER_A

void RACE_set_run_values(); //set all run values (speed, distance, run duration, high difference)

//UART to use serial monitor (9600 baudrate 3Mhz clock)
const eUSCI_UART_ConfigV1 uartConfig0 =
    {
            EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
            19,                                      // BRDIV = 19
            8,                                       // UCxBRF = 8
            85,                                      // UCxBRS = 85
            EUSCI_A_UART_NO_PARITY,                  // No Parity
            EUSCI_A_UART_LSB_FIRST,                  // MSB First
            EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
            EUSCI_A_UART_MODE,                       // UART mode
            EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
            EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
    };

//UART to use GPS and ESP32 (9600 baudrate 3Mhz clock)
const eUSCI_UART_ConfigV1 uartConfig1 =
    {
            EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
            19,                                      // BRDIV = 19
            8,                                       // UCxBRF = 8
            85,                                      // UCxBRS = 85
            EUSCI_A_UART_NO_PARITY,                  // No Parity
            EUSCI_A_UART_LSB_FIRST,                  // MSB First
            EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
            EUSCI_A_UART_MODE,                       // UART mode
            EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
            EUSCI_A_UART_8_BIT_LEN                  // 8 bit data length
    };

//Configuration TIMER_A a 2 secons (0.5 Hz)
const Timer_A_UpModeConfig upConfig =
{
        TIMER_A_CLOCKSOURCE_ACLK,               // ACLK Clock Source 32.768kHz
        TIMER_A_CLOCKSOURCE_DIVIDER_64,         // ACLK/64 = 32768/64 = 512
        0x400,                                 // 0x1400 = 0.1Hz / 0x400 = 0.5Hz
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};


StateMachine_t fsm[] = {
  {STATE_FIXING, fn_FIXING},
  {STATE_IDLE, fn_IDLE},
  {STATE_RUNNING, fn_RUNNING},
  {STATE_PAUSE, fn_PAUSE},
  {STATE_ARRIVED, fn_ARRIVED}
};

State_t current_state = STATE_FIXING;

Gps gps;
Race race;

//control variables
volatile bool timer_a_edge = 0;
volatile bool wifi_conn = 0;

volatile bool up=0;
volatile bool down=0;
volatile bool restart=0;
volatile bool transmitted=0;

int main(void){

    WDT_A_holdTimer();

    gps_init(&gps);

    GPIO_enable_pullup_all_buttons();
    GPIO_enable_interrupt_all_buttons();
    GPIO_enable_leds();
    GPIO_setAsInputPinWithPullUpResistor(WIFI_CONN);
    UART0_config(uartConfig0);
    UART2_config(uartConfig1);

    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    INTERRUPT_enable();


    while(1){
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }else{
            /* serious error */
        }

    }
}

void fn_FIXING(){
    LED_set_color(RED);

    if (1){
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_FIXING;
    }
}

//waiting runs start (play button)
void fn_IDLE(){
    LED_set_color(GREEN);
    transmitted=0;
    if (up){
        up = !up;
        race_init(&race);
        race_set_start_date_time(&race, gps_get_date(&gps), gps_get_time(&gps));
        current_state = STATE_RUNNING;
    }else{
        current_state = STATE_IDLE;
    }
}

void fn_RUNNING(){
    LED_set_color(BLUE);

    if (up){
        up = !up;
        current_state = STATE_PAUSE;
    }else if(restart){
        restart = !restart;
        current_state = STATE_IDLE;
    }else{
        if (gps_data_valid(&gps) && gps_get_fix(&gps)){
            RACE_set_run_values();
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

    if (up){
        up = !up;
        race_set_pause_duration(&race, gps_get_time(&gps));
        current_state = STATE_RUNNING;
    }else if (down){
        down = !down;
        current_state = STATE_ARRIVED;
    }else if(restart){
        restart = !restart;
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_PAUSE;
    }
}

void fn_ARRIVED(){


    LED_set_color(WHITE);

    if (!transmitted  && wifi_conn){
        transmitted =1;
        UART_disableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

        UART_transmit_all_data_race(EUSCI_A0_BASE, &race);
        UART_transmit_all_data_race(EUSCI_A2_BASE, &race);

        UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

        race_init(&race);
        UART_transmit_string(EUSCI_A0_BASE,"\r\n");
    }if(down){
        down = !down;
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_ARRIVED;
     }



}

void GPIO_enable_pullup_all_buttons(){
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_LEFT);  //sinistra
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_RIGHT);  //destra
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_UP);  //giù
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_DOWN);  //su
}

void GPIO_enable_interrupt_all_buttons(){
    GPIO_enableInterrupt(BUTTON_LEFT);
    GPIO_interruptEdgeSelect (BUTTON_LEFT, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_enableInterrupt(BUTTON_RIGHT);
    GPIO_interruptEdgeSelect (BUTTON_RIGHT, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_enableInterrupt(BUTTON_UP);
    GPIO_interruptEdgeSelect (BUTTON_UP, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_enableInterrupt(BUTTON_DOWN);
    GPIO_interruptEdgeSelect (BUTTON_DOWN, GPIO_LOW_TO_HIGH_TRANSITION);
}

void GPIO_enable_leds(){
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(RGB_LED_RED); GPIO_setOutputLowOnPin(RGB_LED_RED);
    GPIO_setAsOutputPin(RGB_LED_GREEN); GPIO_setOutputLowOnPin(RGB_LED_GREEN);
    GPIO_setAsOutputPin(RGB_LED_BLUE); GPIO_setOutputLowOnPin(RGB_LED_BLUE);
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

void UART0_config(eUSCI_UART_ConfigV1 uartConfig){
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    UART_enableModule(EUSCI_A0_BASE);
}
void UART2_config(eUSCI_UART_ConfigV1 uartConfig){
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A2_BASE, &uartConfig);
    UART_enableModule(EUSCI_A2_BASE);
    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
}

void UART_transmit_string(uint32_t moduleInstance, const char* str){
    int i;
    for(i=0; i<strlen(str); i++){
        UART_transmitData(moduleInstance, str[i]);
    }
}

void UART_transmit_all_data_race(uint32_t moduleInstance, Race* race){
        char* data_title = race_make_data_title(race);
        UART_transmit_string(moduleInstance,data_title);
        int i;
        for (i=0 ; i<race->positions_index; i++){
                UART_transmit_string(moduleInstance, race->positions[i].longitude);UART_transmit_string(moduleInstance,";");
                UART_transmit_string(moduleInstance, race->positions[i].latitude);UART_transmit_string(moduleInstance,";");
                UART_transmit_string(moduleInstance, race->positions[i].altitude);UART_transmit_string(moduleInstance,";");
            }
        UART_transmit_string(moduleInstance,"\r\n");
        free(data_title);
    }

void UART0_transmit_gps_data(){
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "speed: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_instant_speed(&race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "distance: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_distance(&race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "duration time: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_run_time(&race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "high difference: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_heigh_difference(&race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "avg speed: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_avg_speed(&race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
}

void INTERRUPT_enable(){
    Interrupt_enableInterrupt(INT_EUSCIA2);     //UART RX interrupt enable
    Interrupt_enableInterrupt(INT_PORT1);   //BUTTON LEFT and RIGHT interrupt enable
    Interrupt_enableInterrupt(INT_PORT3);   //BUTTON DOWN
    Interrupt_enableInterrupt(INT_PORT5);   //BUTTNO UP
    Interrupt_enableInterrupt(INT_TA1_0);       //TIMER_1 interrupt enable
    Interrupt_enableMaster();
}


void RACE_set_run_values(){
    race_set_instant_speed(&race, gps_get_instant_speed(&gps));
    race_set_distance(&race, gps_get_last_distance(&gps));
    race_set_run_duration(&race,gps_get_time(&gps));
    race_set_heigh_difference(&race, gps_get_last_altitude(&gps));
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



void PORT5_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);

    while(!(P5->IN & BIT1));

    if(status & GPIO_PIN1 && (current_state == STATE_IDLE ||  current_state == STATE_RUNNING ||  current_state == STATE_PAUSE)){
        up = 1;
    }
}

void PORT3_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);
    GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    while(!(P3->IN & BIT5));

    if(status & GPIO_PIN5 && (current_state == STATE_PAUSE ||  current_state == STATE_ARRIVED )){
            down = 1;
    }

}


void PORT1_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);


    if(status & GPIO_PIN4 && (current_state == STATE_RUNNING || current_state == STATE_PAUSE)){
          up=0;
          down=0;
          restart = 1;
    }

}


void EUSCIA2_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char ch = UART_receiveData(EUSCI_A2_BASE);
        gps_encode(&gps, ch);
        if (gps_data_valid(&gps)){
            if (gps_get_fix(&gps)){
                UART0_transmit_gps_data();
            }else{
                UART_transmit_string(EUSCI_A0_BASE, "signal not fixed\r\n");
            }
        }else{
        }
        UART_clearInterruptFlag(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    }
    Interrupt_disableSleepOnIsrExit();
}
