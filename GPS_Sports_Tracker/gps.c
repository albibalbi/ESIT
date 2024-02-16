#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gps.h"

//check the type of NMEA segment
static bool check_type(Gps*gps, char message_type[7]){
    if(strncmp(message_type, gps->buffer, 6) == 0) return 1;
    else return 0;
}

// GPS check sum
static bool check_parity(Gps* gps){

    int data_lenght = strlen(gps->buffer);

    int i;
    for (i = 1; i < (data_lenght-4); i++){
        gps->parity ^= gps->buffer[i];
    }

    int ls_nibble = gps->parity & 0x0f;
    int ms_nibble = (gps->parity & 0xf0) >> 4;

    decode_checksum_nibble(&ls_nibble);
    decode_checksum_nibble(&ms_nibble);

    if ((ms_nibble == gps->buffer[data_lenght-3]) && (ls_nibble == gps->buffer[data_lenght-2])){
        return 1;
    }else{
        return 0;
    }
}

// convert checksum exa values in char value
static void decode_checksum_nibble(int* nibble){
    if (*nibble <= 0x9) *nibble += '0';
    else *nibble = (*nibble - 10) + 'A';
}

//fill the attribute of Gps structure in parsing mode
static char* fill_string_attribute(char* attribute, char* value ){
    strcpy(attribute, value);
    return strtok(NULL, ",");
}

static void parse_gps_data_gga(Gps* gps){
    strcpy(gps->old_time, gps->time);
    gps->old_position = gps->position;
    char * token = strtok(gps->buffer, ",");
    token = strtok(NULL, ",");
    token = fill_string_attribute(gps->time, add_temporal_separator(token, ':'));
    token = fill_string_attribute(gps->position.latitude, convert_pos_to_decimal(token));
    token = fill_string_attribute(gps->position.latitude, set_position_sign(gps->position.latitude, token));
    token = fill_string_attribute(gps->position.longitude, convert_pos_to_decimal(token));
    token = fill_string_attribute(gps->position.longitude, set_position_sign(gps->position.longitude, token));
    int i;
    for (i=0; i<3; i++){
        token = strtok(NULL, ",");
    }
    token = fill_string_attribute(gps->position.altitude, token);
}

//parsing of RMC segment
static void parse_gps_data_rmc(Gps* gps){
    char * token = strtok(gps->buffer, ",");
    int i;
    for (i=0; i<2; i++){
        token = strtok(NULL, ",");
    }
    if (strcmp(token, "A") == 0){
        gps->fix_valid = 1;
    }else{
        gps->fix_valid = 0;
    }
    for (i=0; i<6; i++){
        token = strtok(NULL, ",");
    }
    strcpy(gps->date, add_temporal_separator(token, '/'));
}

//convert NMEA latitude and longitude to decimal degree
static char* convert_pos_to_decimal(char* str){
    static char buffer_pos_to_decimal[12];
    char d[4];
    char m[8];

    if (strlen(str) < 11){
        strcpy(buffer_pos_to_decimal, "0");
        strcat(buffer_pos_to_decimal, str);
    }else{
        strcpy(buffer_pos_to_decimal, str);
    }
    strncpy(d,buffer_pos_to_decimal,3);
    strncpy(m,buffer_pos_to_decimal+3,8);
    float result = atof(d) + atof(m)/60;
    sprintf(buffer_pos_to_decimal, "%f", result);
    return buffer_pos_to_decimal;
}

//assign the sign at latitude and longitude attribute
static char* set_position_sign(char* value, char* token){
    static char buffer_set_position[11];
    if (!strcmp(token, "S") || !strcmp(token, "W")){
        strcpy(buffer_set_position, "-");
        strcat(buffer_set_position, value);
    }else{
        strcpy(buffer_set_position, "+");
        strcat(buffer_set_position, value);
    }
    return buffer_set_position;
}

//add a separator for the temporal datas
static char* add_temporal_separator(char* string, char separator){
    static char buffer_temporal_separator[GPS_LEN_DATE];
    int counter = 0;
    int i;
    for (i=0; i<8; i++){
        if (i == 2 || i == 5){
            buffer_temporal_separator[i] = separator;
            counter++;
        }else{
            buffer_temporal_separator[i] = string[i-counter];
        }
    }
    return buffer_temporal_separator;
}

//convert degrees in radiants
static double deg_to_rad(double deg) {
  return deg * (PI/180);
}

//get the distance between two points in meters
float get_distance_m(float lat1, float lon1, float lat2, float lon2) {

  double dLat = deg_to_rad((double)lat2-(double)lat1);  // deg2rad below
  double dLon = deg_to_rad((double)lon2-(double)lon1);
  double a =
    sin(dLat/2) * sin(dLat/2) + cos(deg_to_rad(lat1)) * cos(deg_to_rad(lat2)) * sin(dLon/2) * sin(dLon/2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  return (EARTH_RADIUS * (float)c)*1000;
}

unsigned int get_seconds_from_time(char* time){
    char buffer[GPS_LEN_TIME];
    strcpy(buffer, time);
    char* token = strtok(buffer, ":");
    unsigned int a;
    a = atoi(token) * 60 * 60; token = strtok(NULL, ":");
    a += atoi(token)*60; token = strtok(NULL, ":");
    a += atoi(token);
    return (unsigned int)a;
}

//initialize gps with invalid values
void gps_init(Gps* gps){
    strcpy(gps->date, GPS_INVALID_DATE);
    strcpy(gps->time, GPS_INVALID_TIME);
    strcpy(gps->position.latitude, "0");
    strcpy(gps->position.longitude, "0");
    strcpy(gps->position.altitude, "-0");
    strcpy(gps->old_position.latitude, "0");
    strcpy(gps->old_position.longitude, "0");
    strcpy(gps->old_position.altitude, "-0");
    gps->fix_valid = 0;
    gps->parity = 0;
    gps->buffer_index = 0;
    gps->data_valid_gga = 0;
    gps->data_valid_rmc = 0;
    gps->data_valid = 0;
}

void gps_encode(Gps* gps, char c){
    //gps->data_valid = 0;
    if (c != '\n'){
        gps->buffer[gps->buffer_index++] = c;
    }else{
        if (check_type(gps,GPS_HEADER_GGA) || check_type(gps,GPS_HEADER_RMC)){
            if (check_parity(gps) && (strlen(gps->buffer) > GPS_LEN_MIN_BUFFER)){
                if (gps->buffer[3] == 'G' && !gps->data_valid_gga){
                    parse_gps_data_gga(gps);
                    gps->data_valid_gga = 1;
                }
                if(gps->buffer[3] == 'R' && !gps->data_valid_rmc){
                    parse_gps_data_rmc(gps);
                    gps->data_valid_rmc = 1;
                }
                if (gps->data_valid_gga && gps->data_valid_rmc){
                    gps->data_valid_gga = 0;
                    gps->data_valid_rmc = 0;
                    gps->data_valid = 1;
                }
            }else{
                gps->fix_valid = 0;
                gps->data_valid_gga = 0;
                gps->data_valid_rmc = 0;
            }
        }
        gps->parity = 0;
        gps->buffer_index = 0;
        memset(gps->buffer,0,sizeof(gps->buffer));
    }
}

bool gps_data_valid(Gps* gps){
    return gps->data_valid ? 1 : 0;
}

void gps_set_data_valid(Gps* gps, bool value){
    gps->data_valid = value;
}

//return if the signal is fixed
bool gps_get_fix(Gps* gps){
    return gps->fix_valid == 1 ? 1 : 0;
}

Position gps_get_positions(Gps gps){
    return gps.position;
}

const char* gps_get_latitude(Gps* gps){
    return gps->position.latitude;
}
const char* gps_get_longitude(Gps* gps){
    return gps->position.longitude;
}
const char* gps_get_altitude(Gps* gps){
    return gps->position.altitude;
}

const char* gps_get_date(Gps* gps){
    return gps->date;
}

const char* gps_get_time(Gps* gps){
    return gps->time;
}

unsigned int gps_get_last_seconds(Gps* gps){
    return get_seconds_from_time(gps->time) - get_seconds_from_time(gps->old_time);
}

float gps_get_last_distance(Gps* gps){
    float lat1, lat2, lon1, lon2;
    lat1 = atof(gps->old_position.latitude);
    lon1 = atof(gps->old_position.longitude);
    lat2 = atof(gps->position.latitude);
    lon2 = atof(gps->position.longitude);
    return get_distance_m(lat1, lon1, lat2, lon2);
}

float gps_get_last_altitude(Gps* gps){
    if (!(strcmp(gps->old_position.altitude, "-0"))){
        return 0;
    }else{
        return atof(gps->position.altitude)- atof(gps->old_position.altitude);
    }
}

float gps_get_instant_speed(Gps* gps){
    return (float)((gps_get_last_distance(gps)/(gps_get_last_seconds(gps)))*3.6);
}

//print Gps structure
void gps_print_struct(Gps* gps){
    printf("fix: %s\n", gps->fix_valid ? "true" : "false");
    printf("date: %s\n", gps->date);
    printf("time: %s\n", gps->time);
    printf("latitude: %s\n", gps->position.latitude);
    printf("longitude: %s\n", gps->position.longitude);
    printf("altitude: %s\n", gps->position.altitude);
}

