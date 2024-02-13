#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "race.h"

void race_init(Race* race){
    strcpy(race->start_date, GPS_INVALID_DATE);
    strcpy(race->start_time, GPS_INVALID_TIME);
    memset(race->positions,0,sizeof(race->positions));
    race->instant_speed = 0;
    race->run_duration = 0;
    race->pause_duration = 0;
    race->distance = 0;
    race->heigh_difference = 0;
    race->positions_index = 0;
    race->start_done = 0;
}
void race_set_start_date_time(Race* race,const char* date, const char* time){
    strcpy(race->start_date , date);
    strcpy(race->start_time , time);
}

void race_set_instant_speed(Race* race, float speed){
    race->instant_speed = speed;
}

void race_set_run_duration(Race* race, const char* time){
    char _time[1];
    strcpy(_time,time);
    race->run_duration = get_seconds_from_time(_time) - get_seconds_from_time(race->start_time) - race->pause_duration;
}

void race_set_pause_duration(Race* race, const char* time){
    char _time[1];
    strcpy(_time,time);
    race->pause_duration = get_seconds_from_time(_time) - (get_seconds_from_time(race->start_time) + race->run_duration);
}

void race_set_distance(Race* race, float last_distance){
    race->distance += last_distance;
}

void race_set_heigh_difference(Race* race, float last_altitudes){
    if (last_altitudes > 0){
        race->heigh_difference += last_altitudes;
    }
}

void race_set_positions(Race* race, Position pos){
    strcpy(race->positions[race->positions_index].latitude, pos.latitude);
    strcpy(race->positions[race->positions_index].longitude, pos.longitude);
    strcpy(race->positions[race->positions_index].altitude, pos.altitude);
    race->positions_index++;
}

void race_set_start_run(Race* race, bool value){
    race->start_done = value;
}

const char* float_to_string(float f){
    static char buffer[17];
    sprintf(buffer, "%.2f", f);
    return buffer;
}

const char* int_to_string(int i){
    static char buffer[17];
    sprintf(buffer, "%d", i);
    return buffer;
}

static char* convert_seconds_to_hhmmss(unsigned int total_time){
    static char time[GPS_LEN_TIME];
    int hours = total_time/3600;
    int minutes = (total_time % 3600)/60;
    int seconds = total_time % 60;
    sprintf(time, "%02d:%02d:%02d", hours, minutes, seconds);
    return time;
}

bool race_get_start_done(Race* race){
    return race->start_done;
}

const char* race_get_start_date(Race* race){
    return race->start_date;
}

const char* race_get_start_time(Race* race){
    return race->start_time;
}

const char* race_get_instant_speed(Race* race){
    return float_to_string(race->instant_speed);
}

const char* race_get_distance(Race* race){
    return float_to_string(race->distance);
}

const char* race_get_run_time(Race* race){
    return convert_seconds_to_hhmmss(race->run_duration);
}

const char* race_get_avg_speed(Race* race){
    return float_to_string((race->distance/race->run_duration)*3.6);
}

const char* race_get_heigh_difference(Race* race){
    return float_to_string(race->heigh_difference);
}

char* race_make_data_title(Race* race){
    char* str = (char*)malloc(sizeof(char) * 20);  // Allocate memory for a string of size 20

    if (str == NULL) {
        exit(EXIT_FAILURE);
    }
    strcpy(str, race->start_date);strcat(str, ":");strcat(str, race->start_time);strcat(str, ";");
    strcat(str, int_to_string(race->run_duration)); strcat(str, ";");
    strcat(str, float_to_string(race->distance)); strcat(str, ";");
    strcat(str, float_to_string((race->distance/race->run_duration)*3.6)); strcat(str, ";");
    strcat(str, float_to_string(race->heigh_difference)); strcat(str, ";");

    return str;

}

char* race_make_data_string(Race* race) {
    char* str = (char*)malloc(sizeof(char) * 20);  // Allocate memory for a string of size 20

    if (str == NULL) {
        exit(EXIT_FAILURE);
    }
    strcpy(str, race->start_date);strcat(str, ":");strcat(str, race->start_time);strcat(str, ";");
    strcat(str, int_to_string(race->run_duration)); strcat(str, ";");
    strcat(str, float_to_string(race->distance)); strcat(str, ";");
    strcat(str, float_to_string((race->distance/race->run_duration)*3.6)); strcat(str, ";");
    strcat(str, float_to_string(race->heigh_difference)); strcat(str, ";");

    int i;
    for (i=0; i<race->positions_index; i++){
        strcat(str, race->positions[i].longitude); strcat(str, ";");
        strcat(str, race->positions[i].latitude); strcat(str, ";");
        strcat(str, race->positions[i].altitude); strcat(str, ";");
    }

    return str;
}

void race_print_struct(Race* race){
    printf("start date: %s\n", race_get_start_date(race));
    printf("start time: %s\n", race_get_start_time(race));
    printf("instant speed: %s\n", race_get_instant_speed(race));
    printf("distance: %s\n", race_get_distance(race));
    printf("high difference: %s\n", race_get_heigh_difference(race));
    int i;
    for (i=0; i<race->positions_index; i++){
        printf("latitude %d: %s ", i, race->positions[i].latitude);
        printf("longitude %d: %s ", i, race->positions[i].longitude);
        printf("altitude %d: %s\n", i, race->positions[i].altitude);
    }
}
