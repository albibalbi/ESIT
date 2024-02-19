#include "graphics_functions.h"

void change_position_r(){
    int temp;
    temp=circular_buffer[1];
    circular_buffer[1]=circular_buffer[0];
    circular_buffer[0]=circular_buffer[2];
    circular_buffer[2]=temp;
}

void change_position_l(){
    int temp;
    temp=circular_buffer[2];
    circular_buffer[2]=circular_buffer[0];
    circular_buffer[0]=circular_buffer[1];
    circular_buffer[1]=temp;
}




char* m_to_km(const char *input_str) {
    
    float input_float = atof(input_str);
    float result = input_float / 1000.0;


    static char output_str[20]; 
    sprintf(output_str, "%.2f", result);

    return output_str;
}

char* resize_string(const char *input_str) {
    static char last_five[6]; 
    int input_length = strlen(input_str);


    int zeros_to_add = (input_length < 5) ? 5 - input_length : 0;

    int i;
    for (i = 0; i < zeros_to_add; i++) {
        last_five[i] = '0';
    }

    strcpy(last_five + zeros_to_add, (input_length >= 5) ? input_str + input_length - 5 : input_str);

    return last_five;
}


void draw_data(int pos,int32_t x_str,int32_t y_str, int32_t x_data, int32_t y_data,Race* race){
    char data_name[10];
    sprintf(data_name, "%-8s", ds[pos].data_str);
    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)data_name,
                                AUTO_STRING_LENGTH,
                                x_str,
                                y_str,
                                OPAQUE_TEXT);
    char data_uom[10];
    if(pos==2){
        sprintf(data_uom, "%-8s", ds[pos].pfun(race));
    }else{
        sprintf(data_uom, "%s%s", ds[pos].pfun(race),ds[pos].uom);
    }

    if(pos==0 && current_state==STATE_RUNNING){
        sprintf(data_uom, "%s%s", resize_string(ds[pos].pfun(race)),ds[pos].uom);
     }else if(pos==0 && current_state==STATE_PAUSE){
         sprintf(data_uom, "%s%s", " - - ",ds[pos].uom);
     }

    if(pos==1){
        sprintf(data_uom, "%s%s", resize_string(m_to_km(ds[pos].pfun(race))),ds[pos].uom);
    }

    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)data_uom,
                                AUTO_STRING_LENGTH,
                                x_data,
                                y_data,
                                OPAQUE_TEXT);
}

void draw_page(State_t_arrived current_state_arrived, Race* race){


    GrRectDraw(&g_sContext, &main_rect );
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    char str_data_shown[25],str_page[10];
    sprintf(str_page, "Page %d", (current_state_arrived));
    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)str_page,
                                AUTO_STRING_LENGTH,
                                64,
                                60,
                                OPAQUE_TEXT);


    Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)"                   ",
                                    AUTO_STRING_LENGTH,
                                    64,
                                    80,
                                    OPAQUE_TEXT);
    switch(current_state_arrived){
    case 1:
        sprintf(str_data_shown, "%s: %s%s",ds[current_state_arrived].data_str, resize_string(m_to_km(ds[current_state_arrived].pfun(race))),ds[current_state_arrived].uom);
        break;
    case 3:
        sprintf(str_data_shown, "%s: %s%s",ds[current_state_arrived].data_str, resize_string(ds[current_state_arrived].pfun(race)),ds[current_state_arrived].uom);
        break;
    default:
        sprintf(str_data_shown, "%s: %s%s",ds[current_state_arrived].data_str, ds[current_state_arrived].pfun(race),ds[current_state_arrived].uom);
    }

    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)str_data_shown,
                                AUTO_STRING_LENGTH,
                                64,
                                80,
                                OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)"next>",
                                    AUTO_STRING_LENGTH,
                                    107,
                                    115,
                                    OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)"<prev",
                                        AUTO_STRING_LENGTH,
                                        23,
                                        115,
                                        OPAQUE_TEXT);
}


void lcd_running_pause_page( Race* race){

    GrContextFontSet(&g_sContext, &g_sFontCmtt14);
    draw_data(circular_buffer[0],64,30,64,50,race);


    GrRectDraw(&g_sContext, &up_rect );
    GrRectDraw(&g_sContext, &back_rect );
    GrRectFill(&g_sContext, &back_rect);


    GrRectDraw(&g_sContext, &l_rect );
    GrRectDraw(&g_sContext, &r_rect );

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrRectFill(&g_sContext, &l_rect);
    GrRectFill(&g_sContext, &r_rect);

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

    draw_data(circular_buffer[1],36,82,36,97,race);
    draw_data(circular_buffer[2],94,82,94,97,race);
}

void lcd_show_state(State_t current_state){
    char state_str[10];

    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
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
        sprintf(state_str, "%s","ARRIVED ");
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        break;
    default:
        sprintf(state_str, "%7s","NNN");

    }

    Graphics_drawStringCentered(&g_sContext,
                                (int8_t *)state_str,
                                7,
                                64,
                                15,
                                OPAQUE_TEXT);


    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
}

