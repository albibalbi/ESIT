#include "graphics_functions.h"
#include "state_machine.h"


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
    // Converti la stringa in un float
    float input_float = atof(input_str);

    // Divide il float per 1000
    float result = input_float / 1000.0;


    static char output_str[20]; // Definizione di una stringa locale statica

    // Formatta il risultato come una stringa con due cifre decimali dopo la virgola
    sprintf(output_str, "%.2f", result);

    return output_str;
}

char* resize_string(const char *input_str) {
    static char last_five[6]; // Buffer per contenere gli ultimi 5 caratteri e terminatore nullo
    int input_length = strlen(input_str);

    // Calcola il numero di zeri da aggiungere all'inizio
    int zeros_to_add = (input_length < 5) ? 5 - input_length : 0;

    // Aggiunge zeri all'inizio della stringa risultante
    int i;
    for (i = 0; i < zeros_to_add; i++) {
        last_five[i] = '0';
    }

    // Copia gli ultimi 5 caratteri della stringa di input nella stringa risultante
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

void draw_page(int pos, Race* race){


    GrRectDraw(&g_sContext, &main_rect );
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    char str_data_shown[25],str_page[10];
    sprintf(str_page, "Page %d", (pos));
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
    switch(pos){
    case 1:
        sprintf(str_data_shown, "%s: %s%s",ds[pos].data_str, resize_string(m_to_km(ds[pos].pfun(race))),ds[pos].uom);
        break;
    case 3:
        sprintf(str_data_shown, "%s: %s%s",ds[pos].data_str, resize_string(ds[pos].pfun(race)),ds[pos].uom);
        break;
    default:
        sprintf(str_data_shown, "%s: %s%s",ds[pos].data_str, ds[pos].pfun(race),ds[pos].uom);
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
