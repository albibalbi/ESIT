
#include <Running_libs/race.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "booster_pack_pin_map.h"
#include "state_machine.h"
#include "state_machine_run.h"
#include "state_machine_arrived.h"
#include "Graphics_libs/graphics_functions.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>

#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define JOYSTICK_UP_LIMIT 14000
#define JOYSTICK_RIGHT_LIMIT 11000
#define JOYSTICK_LEFT_LIMIT 5000


//* Graphic library context */
Graphics_Context g_sContext;

/* ADC results buffer */
static uint16_t resultsBuffer[2];

//gps.h & race.h
Gps gps;
Race race;



//structure used for visualization of date in the LCD screen
Date_struct ds[] = {
                    {INST_SPEED,"SPEED",race_get_instant_speed ,"Kmh"},
                    {DISTANCE,"DISTANCE",race_get_distance ,"km"},
                    {DURATION ,"TIME",race_get_run_time,""},
                    {AVG_SPEED,"AVG SPEED",race_get_avg_speed,"kmh"},
                    {HIGH_DIFF,"HIGH DIFF",race_get_heigh_difference,"m"}
};

//event for the main fsm
typedef enum {
    SW_NONE,
    UP_PRESSED,
    DOWN_PRESSED,
    RESTART_PRESSED
} Event_t;
Event_t event;

//main fsm
State_t current_state;
StateMachine_t fsm[] = {
                        {STATE_FIXING, fn_FIXING},
                        {STATE_IDLE, fn_IDLE},
                        {STATE_RUNNING, fn_RUNNING},
                        {STATE_PAUSE, fn_PAUSE},
                        {STATE_ARRIVED, fn_ARRIVED}
};

//color assigned to the main fsm states
typedef enum Color{
    OFF, GREEN, RED, BLUE, PURPLE, YELLOW, WHITE
}Color;

//joystick event (used for internal fsm (state RUNNING & PAUSE & ARRIVED))
typedef enum {
    JSW_NONE,
    JR_PRESSED,
    JL_PRESSED,
    JUP_PRESSED,
    J_PRESSED
} Event_j;
Event_j jevent=JSW_NONE;

//internal fsm (state RUNNING & PAUSE)
StateMachine_t_run fsm_run[] = {
                                {STATE_DESELECT, fn_DESELECT},
                                {STATE_RIGHT, fn_RIGHT},
                                {STATE_LEFT, fn_LEFT}
};
State_t_run current_state_run = STATE_DESELECT;

//internal fsm (state ARRIVED)
StateMachine_t_arrived fsm_arrived[] = {
                                {STATE_PAGE0, fn_PAGE0},
                                {STATE_PAGE1, fn_PAGE1},
                                {STATE_PAGE2, fn_PAGE2},
                                {STATE_PAGE3, fn_PAGE3},
                                {STATE_PAGE4, fn_PAGE4}
};
State_t_arrived current_state_arrived = STATE_PAGE0;

//control variables
volatile bool timer_a_edge = 0;
volatile bool wifi_conn = 0;

volatile bool transmitted=0;

volatile bool b=0;
volatile bool running_pause_entered=0;

int pos=1;
volatile int joystick_pressed=0;

bool page_drawn=0;

Change_display chg_dsp=CHANGE;

volatile int circular_buffer[]={0,1,2};


void _adcInit();    // initialization of the ADC
void _graphicsInit();   //Initialization of the LDC graphics
void _hwInit();     //general hardware initialization

void GPIO_enable_pullup_all_buttons();      //enable button LEFT, RIGHT, UP, DOWN
void GPIO_enable_interrupt_all_buttons();   //enable interrupt for all buttonss
void GPIO_enable_leds();                    //enable all leds
void LED_set_color(Color color);            //set the color for RGB LED

void UART2_config(eUSCI_UART_ConfigV1 uartConfig);  //config UART instance 2 and enable interrupt. GPS UART                 //transmit gps data via serial monitor (testing function)
void UART_transmit_string(uint32_t moduleInstance, const char* str);    //send string via UART
void UART_transmit_all_data_race(uint32_t moduleInstance, Race* race);  //send all data collected via UART

void INTERRUPT_enable(); //enable interrupt PORT1, EUSCI_2, TIMER_A

void RACE_set_run_values(); //set all run values (speed, distance, run duration, high difference)

//UART to use GPS and ESP32 (9600 baudrate 3Mhz clock)
const eUSCI_UART_ConfigV1 uartConfig1 =
{
 EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
 78,                                      // BRDIV = 19
 2,                                       // UCxBRF = 8
 0,                                      // UCxBRS = 85
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


/*
 * Main function
 */
int main(void){
    WDT_A_holdTimer();
    _hwInit();

    //set frequency at 12Mhz
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    gps_init(&gps);
    race_init(&race);

    GPIO_enable_pullup_all_buttons();
    GPIO_enable_interrupt_all_buttons();
    GPIO_enable_leds();
    GPIO_setAsInputPinWithPullUpResistor(WIFI_CONN);
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

/*
MAIN FSM STATE
*/

//wait gps to fix
void fn_FIXING(){
    LED_set_color(RED);
    lcd_show_state(current_state);
    if (gps_get_fix(&gps)){
        current_state = STATE_IDLE;
    }else{
        current_state = STATE_FIXING;
    }
}

//waiting runs start (play button UP)
void fn_IDLE(){
    LED_set_color(GREEN);
    lcd_show_state(current_state);
    GrRectDraw(&g_sContext, &main_rect );

    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)"Press UP to start",
                                20,
                                64,
                                50,
                                OPAQUE_TEXT);
    joystick_pressed=0;
    transmitted=0;
    page_drawn=0;
    jevent=JSW_NONE;
    current_state_arrived=STATE_PAGE0;
    if (event==UP_PRESSED){
        event=SW_NONE;
        race_init(&race);
        race_set_start_date_time(&race, gps_get_date(&gps), gps_get_time(&gps));
        Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)"                  ",
                                    20,
                                    64,
                                    50,
                                    OPAQUE_TEXT);
        current_state = STATE_RUNNING;
    }else{
        current_state = STATE_IDLE;
    }
}

/*the date are being acquired
 * UP--> STATE_PAUSE
 * RESTART--> STATE_IDLE
*/
void fn_RUNNING(){
    LED_set_color(BLUE);
    lcd_show_state(current_state);
    if (event==UP_PRESSED && current_state_run==STATE_DESELECT){
        event=SW_NONE;
        current_state = STATE_PAUSE;
        running_pause_entered=0;
    }else if(event==UP_PRESSED && (current_state_run!=STATE_DESELECT)){
        event=SW_NONE;
    }
    else if(event==RESTART_PRESSED && current_state_run==STATE_DESELECT){
        running_pause_entered=0;
        Graphics_clearDisplay(&g_sContext);
        current_state = STATE_IDLE;
    }else if(event==RESTART_PRESSED && (current_state_run!=STATE_DESELECT)){
        event=SW_NONE;
    }
    else{
        if (gps_data_valid(&gps) && gps_get_fix(&gps)){
            gps_set_data_valid(&gps, 0);
            RACE_set_run_values();
            if (timer_a_edge){
                timer_a_edge = !timer_a_edge;
                race_set_positions(&race, gps_get_positions(gps));
            }
        }
        if(!running_pause_entered){
            running_pause_entered=!running_pause_entered;
            lcd_running_pause_page(&race);
        }

        if(current_state_run < NUM_STATES_RUN){
            (*fsm_run[current_state_run].state_function_run)();
        }else{
            /* serious error */
        }

        current_state = STATE_RUNNING;
    }
}


/*the acquisition of data is suspended
 * UP--> STATE_RUNNING
 * RESTART--> STATE_IDLE
 * DOWN--> STATE_ARRIVED
*/
void fn_PAUSE(){
    LED_set_color(PURPLE);
    lcd_show_state(current_state);
    if (event==UP_PRESSED && (current_state_run==STATE_DESELECT)){
        running_pause_entered=0;
        event=SW_NONE;
        race_set_pause_duration(&race, gps_get_time(&gps));
        current_state = STATE_RUNNING;
    }else if(event==UP_PRESSED && (current_state_run!=STATE_DESELECT)){
        event=SW_NONE;
    }
    else if (event==DOWN_PRESSED && (current_state_run==STATE_DESELECT)){
        running_pause_entered=0;
        event=SW_NONE;
        Graphics_clearDisplay(&g_sContext);
        current_state = STATE_ARRIVED;
    }else if(event==DOWN_PRESSED && (current_state_run!=STATE_DESELECT)){
        event=SW_NONE;
    }
    else if(event==RESTART_PRESSED && current_state_run==STATE_DESELECT){
        running_pause_entered=0;
        Graphics_clearDisplay(&g_sContext);
        current_state = STATE_IDLE;
    }else if(event==RESTART_PRESSED && (current_state_run!=STATE_DESELECT)){
        event=SW_NONE;
    }
    else{
        if(!running_pause_entered){
            running_pause_entered=!running_pause_entered;
            lcd_running_pause_page(&race);
        }

        if(current_state_run < NUM_STATES_RUN){
            (*fsm_run[current_state_run].state_function_run)();
        }else{
            /* serious error */
        }
        current_state = STATE_PAUSE;
    }
}


/*the data are sent
 * DOWN--> STATE_IDLE
 */
void fn_ARRIVED(){


    LED_set_color(WHITE);
    lcd_show_state(current_state);

    jevent=JSW_NONE;

    if(event==DOWN_PRESSED){
        Graphics_clearDisplay(&g_sContext);
        if (!transmitted  && wifi_conn){
            transmitted =1;
            UART_disableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

            //UART_transmit_all_data_race(EUSCI_A0_BASE, &race);
            UART_transmit_all_data_race(EUSCI_A2_BASE, &race);

            UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

            race_init(&race);
        }
        current_state = STATE_IDLE;
    }else{
        if(!joystick_pressed){
            Graphics_setFont(&g_sContext,&g_sFontCmss14);
            Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)"Press joystick",
                                        AUTO_STRING_LENGTH,
                                        64,
                                        40,
                                        OPAQUE_TEXT);

            GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        }else {
            Graphics_setFont(&g_sContext,&g_sFontCmss14);
            Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)"              ",
                                        AUTO_STRING_LENGTH,
                                        64,
                                        40,
                                        OPAQUE_TEXT);

            GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        }

        GrRectDraw(&g_sContext, &main_rect );
        if(current_state_arrived < NUM_STATES_ARRIVED){
            (*fsm_arrived[current_state_arrived].state_function_arrived)();
        }else{
            /* serious error */
        }
        current_state = STATE_ARRIVED;
    }
}

/*
 * END OF MAIN FSM
 */





/*
 * STATE RUNNING AND PAUSE FSM (also called run fsm)
 */

/*squares are not selected
 * JR --> STATE_RIGHT
 * JL --> STATE_LEFT
*/
void fn_DESELECT(){
    b=0;
    if(jevent==JR_PRESSED){
        current_state_run=STATE_RIGHT;
    }else if(jevent==JL_PRESSED){
        current_state_run=STATE_LEFT;
    }else{
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);

        GrContextFontSet(&g_sContext, &g_sFontCmtt14);
        draw_data(circular_buffer[0],64,30,64,50,&race);

        GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        draw_data(circular_buffer[1],36,82,36,97,&race);
        draw_data(circular_buffer[2],94,82,94,97,&race);
        current_state_run=STATE_DESELECT;
    }

}

/*the right square is selected
 * JUP --> STATE_DESELECTED
 * JL --> STATE_LEFT
 * J --> STATE_DESELECTED (and the visualization order of data changes)
*/
void fn_RIGHT(){
    if(jevent==JUP_PRESSED){

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrRectFill(&g_sContext, &r_rect);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        current_state_run=STATE_DESELECT;
    }else if(jevent==JL_PRESSED){
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrRectFill(&g_sContext, &r_rect);
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        draw_data(circular_buffer[2],94,82,94,97,&race);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        current_state_run=STATE_LEFT;
        b=0;
    }else if(jevent==J_PRESSED){
        change_position_r();
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        //Graphics_clearDisplay(&g_sContext);
        GrRectDraw(&g_sContext, &up_rect );
        GrRectDraw(&g_sContext, &back_rect );
        GrRectFill(&g_sContext, &back_rect);


        GrRectDraw(&g_sContext, &l_rect );
        GrRectDraw(&g_sContext, &r_rect );

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrRectFill(&g_sContext, &l_rect);
        GrRectFill(&g_sContext, &r_rect);

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        GrContextFontSet(&g_sContext, &g_sFontCmtt14);
        Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)"            ",
                                    AUTO_STRING_LENGTH,
                                    64,
                                    50,
                                    OPAQUE_TEXT);
        GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        current_state_run=STATE_DESELECT;
    }else{
        if(!b){
            b=!b;
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            GrRectFill(&g_sContext, &r_rect);
        }

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        draw_data(circular_buffer[2],94,82,94,97,&race);
        current_state_run=STATE_RIGHT;
    }
}



/*the left square is selected
 * JUP --> STATE_DESELECTED
 * JR --> STATE_RIGHT
 * J --> STATE_DESELECTED (and the visualization order of data changes)
*/
void fn_LEFT(){
    if(jevent==JUP_PRESSED){

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrRectFill(&g_sContext, &l_rect);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        current_state_run=STATE_DESELECT;
    }else if(jevent==JR_PRESSED){
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrRectFill(&g_sContext, &l_rect);
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        draw_data(circular_buffer[1],36,82,36,97,&race);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        current_state_run=STATE_RIGHT;
        b=0;
    }else if(jevent==J_PRESSED){
        change_position_l();
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        //Graphics_clearDisplay(&g_sContext);
        GrRectDraw(&g_sContext, &up_rect );
        GrRectDraw(&g_sContext, &back_rect );
        GrRectFill(&g_sContext, &back_rect);


        GrRectDraw(&g_sContext, &l_rect );
        GrRectDraw(&g_sContext, &r_rect );

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrRectFill(&g_sContext, &l_rect);
        GrRectFill(&g_sContext, &r_rect);

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        GrContextFontSet(&g_sContext, &g_sFontCmtt14);
        Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)"           ",
                                    AUTO_STRING_LENGTH,
                                    64,
                                    50,
                                    OPAQUE_TEXT);
        GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        current_state_run=STATE_DESELECT;
    }else{
        if(!b){
            b=!b;
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            GrRectFill(&g_sContext, &l_rect);
        }

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        draw_data(circular_buffer[1],36,82,36,97,&race);
        current_state_run=STATE_LEFT;
    }
}
/*
 * END OF STATE RUNNING & PAUSE FSM
 */


/*
 * START OF STATE ARRIVED FSM
 */

/*initial page (STATE_PAGE0)
 * J --> STATE_PAGE1
*/
void fn_PAGE0(){
    if(jevent==J_PRESSED){
        joystick_pressed=1;
        current_state_arrived=STATE_PAGE1;
    }else{
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE0;
    }
}


/*distance page (STATE_PAGE1)
 * JR --> STATE_PAGE2
 * JL --> STATE_PAGE4
*/
void fn_PAGE1(){

    if(jevent==JR_PRESSED && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE2;
    }else if(jevent==JL_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE4;
    }
    else{
        if(!page_drawn){
            page_drawn=!page_drawn;
            draw_page(current_state_arrived,&race);
        }

        current_state_arrived=STATE_PAGE1;
    }
}

/*time page (STATE_PAGE2)
 * JR --> STATE_PAGE3
 * JL --> STATE_PAGE1
*/
void fn_PAGE2(){

    if(jevent==JR_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE3;
    }else if(jevent==JL_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE1;
    }
    else{
        if(!page_drawn){
            page_drawn=!page_drawn;
            draw_page(current_state_arrived,&race);
        }
        current_state_arrived=STATE_PAGE2;
    }
}

/*AVG speed page (STATE_PAGE3)
 * JR --> STATE_PAGE4
 * JL --> STATE_PAGE2
*/
void fn_PAGE3(){

    if(jevent==JR_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE4;
    }else if(jevent==JL_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE2;
    }
    else{
        if(!page_drawn){
            page_drawn=!page_drawn;
            draw_page(current_state_arrived,&race);
        }
        current_state_arrived=STATE_PAGE3;
    }
}


/*high diff page (STATE_PAGE4)
 * JR --> STATE_PAGE1
 * JL --> STATE_PAGE3
*/
void fn_PAGE4(){

    if(jevent==JR_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE1;
    }else if(jevent==JL_PRESSED  && page_drawn){
        page_drawn=!page_drawn;
        jevent=JSW_NONE;
        current_state_arrived=STATE_PAGE3;
    }
    else{
        if(!page_drawn){
            page_drawn=!page_drawn;
            draw_page(current_state_arrived,&race);
        }
        current_state_arrived=STATE_PAGE4;
    }
}
/*
 * END  OF STATE ARRIVED FSM
 */



// timer interrupt handler
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


//port5 interrupt handler (used for button at P5.1-->UP BUTTON)
void PORT5_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);

    while(!(P5->IN & BIT1));

    if(status & GPIO_PIN1 && (current_state == STATE_IDLE ||  current_state == STATE_RUNNING ||  current_state == STATE_PAUSE)){
        event=UP_PRESSED;
    }
}

//port3 interrupt handler (used for button at P3.5-->DOWN BUTTON)
void PORT3_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);
    GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    while(!(P3->IN & BIT5));

    if(status & GPIO_PIN5 && (current_state == STATE_PAUSE ||  current_state == STATE_ARRIVED )){
        event=DOWN_PRESSED;
    }

}

//port1 interrupt handler (used for button at P1.4-->RESTART BUTTON)
void PORT1_IRQHandler(void)
{
    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);


    if(status & GPIO_PIN4 && (current_state == STATE_RUNNING || current_state == STATE_PAUSE)){
        event=RESTART_PRESSED;
    }

}


void EUSCIA2_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char ch = UART_receiveData(EUSCI_A2_BASE);
        gps_encode(&gps, ch);
        UART_clearInterruptFlag(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    }
    Interrupt_disableSleepOnIsrExit();
}

//ADC interrupt handler (used for joystick interrupt)
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    /* ADC_MEM1 conversion completed */
    if(status & ADC_INT1)
    {
        /* Store ADC14 conversion results */
        resultsBuffer[0] = ADC14_getResult(ADC_MEM0);
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1);

        //for state_machine_run
        if(resultsBuffer[0]>JOYSTICK_RIGHT_LIMIT && (current_state_run==STATE_DESELECT || current_state_run==STATE_LEFT ||
                current_state_arrived==STATE_PAGE1 || current_state_arrived==STATE_PAGE2 || current_state_arrived==STATE_PAGE3 || current_state_arrived==STATE_PAGE4)){
            jevent=JR_PRESSED;
        }

        if(resultsBuffer[0]<JOYSTICK_LEFT_LIMIT && (current_state_run==STATE_DESELECT || current_state_run==STATE_RIGHT ||
                current_state_arrived==STATE_PAGE1 || current_state_arrived==STATE_PAGE2 || current_state_arrived==STATE_PAGE3 || current_state_arrived==STATE_PAGE4)){
            jevent=JL_PRESSED;
        }

        if(resultsBuffer[1]>JOYSTICK_UP_LIMIT  && (current_state_run==STATE_RIGHT || current_state_run==STATE_LEFT)){
            jevent=JUP_PRESSED;
        }

        if (!(P4IN & GPIO_PIN1) && (current_state_run==STATE_RIGHT || current_state_run==STATE_LEFT || current_state_arrived==STATE_PAGE0)){
            jevent=J_PRESSED;
        }


    }
}


//Implementation of function declared before main
void GPIO_enable_pullup_all_buttons(){
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_LEFT);  //sinistra
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_RIGHT);  //destra
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_UP);  //giu
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

//set a color for a led
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



// initialization of the ADC
void _adcInit(){
    /* Configures Pin 6.0 and 4.4 as ADC input */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initializing ADC (ADCOSC/64/8) */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM1 (A15, A9)  with repeat)
     * with internal 2.5v reference */
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
    ADC14_configureConversionMemory(ADC_MEM0,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_configureConversionMemory(ADC_MEM1,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    /* Enabling the interrupt when a conversion on channel 1 (end of sequence)
     *  is complete and enabling conversions */
    ADC14_enableInterrupt(ADC_INT1);

    /* Enabling Interrupts */
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    /* Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
}

//Initialization of the LDC graphics
void _graphicsInit()
{
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

//general hardware initialization
void _hwInit()
{
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    _graphicsInit();
    _adcInit();
}



