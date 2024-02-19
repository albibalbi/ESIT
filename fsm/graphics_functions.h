#ifndef __GRAPHICS_FUNCTIONS_H__
#define __GRAPHICS_FUNCTIONS_H__

#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "race.h"
#include "state_machine_arrived.h"
#include "state_machine.h"



//* Graphic library context */
Graphics_Context g_sContext;

//rectangles showed in STATE_PAUSE & STATE_RUNNING
static tRectangle main_rect = { 5, 5, 123, 123};
static tRectangle up_rect = { 5, 5, 123, 70};
static tRectangle back_rect = { 5, 70, 123, 123};
static tRectangle l_rect = { 8, 73, 62, 120};
static tRectangle r_rect = { 66, 73, 120, 120};

//data shown
typedef enum {
    INST_SPEED,
    DISTANCE,
    DURATION,
    AVG_SPEED,
    HIGH_DIFF,
    NUM
}Data_Shown;

// useful structure for data shown
typedef struct{
    Data_Shown data;
    char* data_str;
    const char* (*pfun)(Race* race);
    char* uom;
} Date_struct;
extern Date_struct ds[];

//controls if display has changed
typedef enum {
    CHANGE,
    NO_CHANGE
} Change_display;
extern Change_display chg_dsp;



extern volatile int circular_buffer[];//circular buffer useful to change visualization in STATE_PAUSE & STATE_RUNNING
void change_position_l(); //changes postion of data when left is selected in STATE_PAUSE & STATE_RUNNING
void change_position_r(); //changes postion of data when right is selected in STATE_PAUSE & STATE_RUNNING

char* m_to_km(const char *input_str); // converts meters to kilometers for visualization

char* resize_string(const char *input_str); //resize the string of data for visualization

void lcd_show_state(State_t current_state); //shows in which state is the main fsm in the LCD

void draw_data(int pos,int32_t x_str,int32_t y_str, int32_t x_data, int32_t y_data,Race* race); // draws data in STATE_PAUSE & STATE_RUNNING

void draw_page(State_t_arrived current_state_arrived,Race* race); //draws data in STATE_ARRIVED

void lcd_running_pause_page(Race* race); //set the page in STATE_PAUSE & STATE_RUNNING


#endif
