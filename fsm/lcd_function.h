#ifndef __LCD_FUNCTION_H__
#define __LCD_FUNCTION_H__

#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include "state_machine.h"

Graphics_Context g_sContext;

void _graphicsInit();
void lcd_show_state(State_t current_state);



#endif
