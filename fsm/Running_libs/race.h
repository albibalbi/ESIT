#ifndef __RACE_H__
#define __RACE_H__
#include "gps.h"
#include <stdbool.h>

// typedef struct Position{
//     char latitude[GPS_LEN_LATITUDE];
//     char longitude[GPS_LEN_LONGITUDE];
//     char altitude[GPS_LEN_ALTITUDE];
// }Position;

typedef struct Race{
    char start_date[GPS_LEN_DATE];
    char start_time[GPS_LEN_TIME];
    float instant_speed;
    unsigned int run_duration;          //seconds
    unsigned int pause_duration;        //seconds
    float distance;                     //meters
    float heigh_difference;             //meters
    Position positions[GPS_LEN_MEMORY]; //lat, lon, alt
    int positions_index;
    bool start_done;
}Race;

// settings
void race_init(Race* race);
void race_set_start_date_time(Race* race,const char* date, const char* time);
void race_set_instant_speed(Race* race, float speed);
void race_set_run_duration(Race* race, const char* time);
void race_set_pause_duration(Race* race, const char* time);
void race_set_distance(Race* race, float last_distance);
void race_set_heigh_difference(Race* race, float last_altitudes);
void race_set_positions(Race* race, Position pos);
void race_set_start_run(Race* race, bool value);

// conversions
const char* float_to_string(float f);
const char* int_to_string(int i);
static char* convert_seconds_to_hhmmss(unsigned int total_time);

// gets
bool race_get_start_done(Race* race);
const char* race_get_start_date(Race* race);
const char* race_get_start_time(Race* race);
const char* race_get_instant_speed(Race* race);
const char* race_get_distance(Race* race);
const char* race_get_run_time(Race* race);
const char* race_get_avg_speed(Race* race);
const char* race_get_heigh_difference(Race* race);
char* race_make_data_string(Race* race);
char* race_make_data_title(Race* race);

//print
void race_print_struct(Race* race);

#endif
