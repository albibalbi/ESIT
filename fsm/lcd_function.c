#include "lcd_function.h"
#include "state_machine.h"
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

void lcd_show_state(State_t current_state){
    char state_str[10];

    switch(current_state){
    case STATE_FIXING:
        sprintf(state_str, "%7s","FIXING ");
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
        break;
    case STATE_IDLE:
        sprintf(state_str, "%7s","IDLE   ");
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
        break;
    case STATE_RUNNING:
        sprintf(state_str, "%7s","RUNNING ");
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        break;
    case STATE_PAUSE:
        sprintf(state_str, "%7s","PAUSE  ");
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_PURPLE);
        break;
    case STATE_ARRIVED:
        sprintf(state_str, "%7s","ARRIVED");
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_YELLOW);
        break;
    default:
        sprintf(state_str, "%7s","NNN");

    }



    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)state_str,
                                7,
                                64,
                                30,
                                OPAQUE_TEXT);
}
