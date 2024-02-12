#include "msp.h"

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>

/* Graphic library context */
Graphics_Context g_sContext;
/* ADC results buffer */
static uint16_t resultsBuffer[2];


typedef enum {
    DISTANCE,
    DURATION,
    AVG_SPEED,
    ALTITUDE,
    NUM
}Data_Shown;

typedef struct{
    Data_Shown data;
    char* (*pfun)(void);
} Date_struct;


char* race_get_distance(){
    return "1200";
}
char* race_get_run_time(){
    return "2000";
}
char* race_get_avg_speed(){
    return "12";
}

char* race_get_heigh_difference(){
    return "81";
}

Date_struct ds[] = {
  {DISTANCE, race_get_distance},
  {DURATION ,race_get_run_time},
  {AVG_SPEED,race_get_avg_speed},
  {ALTITUDE,race_get_heigh_difference}
};

int pos=0;
int joystick_pressed=0;


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

void _graphicsInit()
{
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

void _hwInit()
{
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    _graphicsInit();
    _adcInit();
}



void main(void)
{
    _hwInit();
    Graphics_drawStringCentered(&g_sContext,
                                                            (int8_t *)"Press joystick",
                                                             AUTO_STRING_LENGTH,
                                                             64,
                                                             70,
                                                             OPAQUE_TEXT);


       while (1)
       {
       }
}


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

        char string[25],string1[10];
        char data_str[15],uom[3];
        int page;
        if(resultsBuffer[0]>9600 && joystick_pressed){

                pos++;
                pos=pos%NUM;


            switch(ds[pos].data){
                                       case DISTANCE:
                                           page=1;
                                           sprintf(data_str, "%s","Distance");
                                           sprintf(uom, "%s","m");
                                           break;
                                       case DURATION:
                                           page=2;
                                           sprintf(data_str, "%s","Duration");
                                           sprintf(uom, "%s","s");
                                           break;
                                       case AVG_SPEED:
                                           page=3;
                                           sprintf(data_str, "%s","Avg speed");
                                           sprintf(uom, "%s","km/h");
                                           break;
                                       case ALTITUDE:
                                           page=4;
                                           sprintf(data_str, "%s","Altitude");
                                           sprintf(uom, "%s","m");
                                           break;
                                       default:
                                           page=0;
                                           sprintf(data_str, "%s","NNN");
                                           sprintf(uom, "%s","");

                                   }

            Graphics_clearDisplay(&g_sContext);


            sprintf(string1, "Page: %d",page);
                        Graphics_drawStringCentered(&g_sContext,
                                                    (int8_t *)string1,
                                                     AUTO_STRING_LENGTH,
                                                     64,
                                                     50,
                                                     OPAQUE_TEXT);

            sprintf(string, "%s: %s%s",data_str, ds[pos].pfun(),uom);
            Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)string,
                                         AUTO_STRING_LENGTH,
                                         64,
                                         70,
                                         OPAQUE_TEXT);

        }

        if(resultsBuffer[0]<9200 && joystick_pressed){

            pos--;
            if(pos<0) pos=NUM-1;

            switch(ds[pos].data){
                case DISTANCE:
                    page=1;
                    sprintf(data_str, "%s","Distance");
                    sprintf(uom, "%s","m");
                    break;
                case DURATION:
                    page=2;
                    sprintf(data_str, "%s","Duration");
                    sprintf(uom, "%s","s");
                    break;
                case AVG_SPEED:
                    page=3;
                    sprintf(data_str, "%s","Avg speed");
                    sprintf(uom, "%s","km/h");
                    break;
                case ALTITUDE:
                    page=4;
                    sprintf(data_str, "%s","Altitude");
                    sprintf(uom, "%s","m");
                    break;
               default:
                    sprintf(data_str, "%s","NNN");
                    sprintf(uom, "%s","");

             }

             Graphics_clearDisplay(&g_sContext);



             sprintf(string1, "Page: %d",page);
                                     Graphics_drawStringCentered(&g_sContext,
                                                                 (int8_t *)string1,
                                                                  AUTO_STRING_LENGTH,
                                                                  64,
                                                                  50,
                                                                  OPAQUE_TEXT);

             sprintf(string, "%s: %s%s",data_str, ds[pos].pfun(),uom);
             Graphics_drawStringCentered(&g_sContext,
                                                (int8_t *)string,
                                                 AUTO_STRING_LENGTH,
                                                 64,
                                                 70,
                                                 OPAQUE_TEXT);
        }

        /*char string[10];
        sprintf(string, "X: %5d", resultsBuffer[0]);
        Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)string,
                                        8,
                                        64,
                                        50,
                                        OPAQUE_TEXT);*/





        if (!(P4IN & GPIO_PIN1) && !joystick_pressed){
            joystick_pressed=1;



            Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)"Page: 1",
                                        AUTO_STRING_LENGTH,
                                         64,
                                         50,
                                          OPAQUE_TEXT);

                sprintf(string, "Distance: %s m", ds[pos].pfun());
                Graphics_drawStringCentered(&g_sContext,
                                                        (int8_t *)string,
                                                         AUTO_STRING_LENGTH,
                                                         64,
                                                         70,
                                                         OPAQUE_TEXT);
        }

    }
}

