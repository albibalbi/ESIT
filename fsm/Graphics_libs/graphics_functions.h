#ifndef __GRAPHICS_FUNCTIONS_H__
#define __GRAPHICS_FUNCTIONS_H__

#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Running_libs/race.h"
#include "state_machine_arrived.h"
#include "state_machine.h"



//* Graphic library context */
Graphics_Context g_sContext;

static tRectangle main_rect = { 5, 5, 123, 123};
static tRectangle up_rect = { 5, 5, 123, 70};
static tRectangle back_rect = { 5, 70, 123, 123};
static tRectangle l_rect = { 8, 73, 62, 120};
static tRectangle r_rect = { 66, 73, 120, 120};

typedef enum {
    INST_SPEED,
    DISTANCE,
    DURATION,
    AVG_SPEED,
    HIGH_DIFF,
    NUM
}Data_Shown;

typedef struct{
    Data_Shown data;
    char* data_str;
    const char* (*pfun)(Race* race);
    char* uom;
} Date_struct;

extern Date_struct ds[];

typedef enum {
    CHANGE,
    NO_CHANGE
} Change_display;

extern Change_display chg_dsp;



extern volatile int circular_buffer[];

char* m_to_km(const char *input_str);

char* resize_string(const char *input_str) ;

void draw_data(int pos,int32_t x_str,int32_t y_str, int32_t x_data, int32_t y_data,Race* race);

void draw_page(State_t_arrived current_state_arrived,Race* race);

void change_position_l();
void change_position_r();

void lcd_running_pause_page(Race* race);




#endif
