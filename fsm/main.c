#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "race.h"
#include "booster_pack_pin_map.h"
#include "state_machine.h"
#include "lcd_function.h"
#include "UART_function.h"
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"



void GPIO_enable_pullup_all_buttons();      //enable button LEFT, RIGHT, UP, DOWN
void GPIO_enable_interrupt_all_buttons();   //enable interrupt for all buttonss
void GPIO_enable_leds();                    //enable all leds


void INTERRUPT_enable(); //enable interrupt PORT1, EUSCI_2, TIMER_A

void RACE_set_run_values(); //set all run values (speed, distance, run duration, high difference)


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


void _hwInit()
{
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    _graphicsInit();
    //_adcInit();
}



int main(void){

    WDT_A_holdTimer();
    _hwInit();
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


    fsm[0].state=STATE_FIXING;
    fsm[0].state_function=fn_FIXING;
    fsm[1].state=STATE_IDLE;
    fsm[1].state_function=fn_IDLE;
    fsm[2].state=STATE_RUNNING;
    fsm[2].state_function=fn_RUNNING;
    fsm[3].state=STATE_PAUSE;
    fsm[3].state_function=fn_PAUSE;
    fsm[4].state=STATE_ARRIVED;
    fsm[4].state_function=fn_ARRIVED;

    current_state = STATE_FIXING;

    event = SW_NONE;

    while(1){
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }else{
            /* serious error */
        }

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




void INTERRUPT_enable(){
    Interrupt_enableInterrupt(INT_EUSCIA2);     //UART RX interrupt enable
    Interrupt_enableInterrupt(INT_PORT1);   //BUTTON LEFT and RIGHT interrupt enable
    Interrupt_enableInterrupt(INT_PORT3);   //BUTTON DOWN
    Interrupt_enableInterrupt(INT_PORT5);   //BUTTNO UP
    Interrupt_enableInterrupt(INT_TA1_0);       //TIMER_1 interrupt enable
    Interrupt_enableMaster();
}




void PORT1_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);


    if(status & GPIO_PIN4 && (current_state == STATE_RUNNING || current_state == STATE_PAUSE)){
          event=RESTART_PRESSED;
    }

}


void PORT5_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);

    while(!(P5->IN & BIT1));

    if(status & GPIO_PIN1 && (current_state == STATE_IDLE ||  current_state == STATE_RUNNING ||  current_state == STATE_PAUSE)){
        event=UP_PRESSED;

    }
}

void PORT3_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);
    GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    while(!(P3->IN & BIT5));

    if(status & GPIO_PIN5 && (current_state == STATE_PAUSE ||  current_state == STATE_ARRIVED )){
            event=DOWN_PRESSED;
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
                UART0_transmit_gps_data(&race);
            }else{
                UART_transmit_string(EUSCI_A0_BASE, "signal not fixed\r\n");
            }
        }else{
        }
        UART_clearInterruptFlag(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    }
    Interrupt_disableSleepOnIsrExit();
}
