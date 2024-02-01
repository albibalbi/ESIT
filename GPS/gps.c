#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gps.h"


//initialize gps with invalid values
void gps_init(Gps* gps){
    strcpy(gps->date, GPS_INVALID_DATE);
    strcpy(gps->time, GPS_INVALID_TIME);
    strcpy(gps->position.latitude, GPS_INVALID_LATITUDE);
    strcpy(gps->position.longitude, GPS_INVALID_LONGITUDE);
    strcpy(gps->position.altitude, GPS_INVALID_ALTITUDE);
    gps->fix_valid = 0;
    gps->parity = 0;  
    gps->buffer_index = 0;
    gps->memory_index = 0;
    gps->data_valid_gga = 0;
    gps->data_valid_rmc = 0;
}

//check the type of NMEA segment
static bool check_type(Gps*gps, char message_type[7]){
    if(strncmp(message_type, gps->buffer, 6) == 0) return 1;
    else return 0;
}

static void fill_memory(Gps* gps){
    if (strlen(gps->memory.date)==0)strcpy(gps->memory.date, gps->date);
    strcpy(gps->memory.times[gps->memory_index], gps->time);
    strcpy(gps->memory.positions[gps->memory_index].latitude,gps->position.latitude);
    strcpy(gps->memory.positions[gps->memory_index].longitude, gps->position.longitude);
    strcpy(gps->memory.positions[gps->memory_index].altitude, gps->position.altitude);
    gps->memory_index++;
}

//fills the Gps struct with data from gps
bool gps_encode(Gps* gps, char c){
    bool encode_ok = 0;
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
                    encode_ok = 1;
                    fill_memory(gps);
                }
            }else{
                gps->fix_valid = 0;
                encode_ok = 1;
            }
        }
        gps->parity = 0;
        gps->buffer_index = 0;
        memset(gps->buffer,0,sizeof(gps->buffer));
    }
    return encode_ok;
}

// GPS check sum
static bool check_parity(Gps* gps){

    int data_lenght = strlen(gps->buffer);

    for (int i = 1; i < (data_lenght-4); i++){
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

static void parse_gps_data_gga(Gps* gps){
	char * token = strtok(gps->buffer, ",");	
	token = strtok(NULL, ",");
	token = fill_string_attribute(gps->time, add_temporal_separator(token, ':'));
	token = fill_string_attribute(gps->position.latitude, convert_pos_to_decimal(token));
	token = fill_string_attribute(gps->position.latitude, set_position_sign(gps->position.latitude, token));
	token = fill_string_attribute(gps->position.longitude, convert_pos_to_decimal(token));
	token = fill_string_attribute(gps->position.longitude, set_position_sign(gps->position.longitude, token));
	for (int i=0; i<3; i++){
		token = strtok(NULL, ",");
	}
	token = fill_string_attribute(gps->position.altitude, token);
}

//parsing of RMC segment
static void parse_gps_data_rmc(Gps* gps){
    char * token = strtok(gps->buffer, ",");
	for (int i=0; i<2; i++){
		token = strtok(NULL, ",");
	}
	if (!strcmp(token, "A")){
		gps->fix_valid = 1;
	}else{
		gps->fix_valid = 0;
	}
	for (int i=0; i<6; i++){
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

//add a separator for the temporal datas
static char* add_temporal_separator(char* string, char separator){
    static char buffer_temporal_separator[GPS_LEN_DATE];
    int counter = 0;
    for (int i=0; i<8; i++){
        if (i == 2 || i == 5){
            buffer_temporal_separator[i] = separator;
            counter++;
        }else{
            buffer_temporal_separator[i] = string[i-counter];
        }
    }
    return buffer_temporal_separator;
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

//return the latitude values in float
float gps_get_latitude_f(Gps* gps){
    return atof(gps->position.latitude);
}

//return the longitude values in float
float gps_get_longitude_f(Gps* gps){
    return atof(gps->position.longitude);
}

//return the longitude values in float
float gps_get_altitude_f(Gps* gps){
    return atof(gps->position.altitude);
}

//return if the signal is fixed
bool gps_get_fix(Gps* gps){
    return gps->fix_valid;
}

bool gps_memory_is_full(Gps* gps){
    return gps->memory_index < (GPS_LEN_MEMORY - 1) ? 0 : 1;
}

//return the date
char* gps_get_date(Gps* gps){
    return gps->date;
}

//return the time
char* gps_get_time(Gps* gps){
    return gps->time;
}

//convert degrees in radiants
static double deg_to_rad(double deg) {
  return deg * (PI/180);
}

//get the distance between two points in meters
static float get_distance_m(float lat1, float lon1, float lat2, float lon2) {

  double dLat = deg_to_rad((double)lat2-(double)lat1);  // deg2rad below
  double dLon = deg_to_rad((double)lon2-(double)lon1); 
  double a = 
    sin(dLat/2) * sin(dLat/2) + cos(deg_to_rad(lat1)) * cos(deg_to_rad(lat2)) * sin(dLon/2) * sin(dLon/2); 
  double c = 2 * atan2(sqrt(a), sqrt(1-a)); 
  return (EARTH_RADIUS * (float)c)*1000;
}

float gps_get_total_distance_m(Gps* gps){
    float lat1, lat2, lon1, lon2;
    float total_distance = 0;
    for(int i=1; i<gps->memory_index; i++){
        lat1 = atof(gps->memory.positions[i-1].latitude);
        lon1 = atof(gps->memory.positions[i-1].longitude);
        lat2 = atof(gps->memory.positions[i].latitude);
        lon2 = atof(gps->memory.positions[i].longitude);
        total_distance += get_distance_m(lat1, lon1, lat2, lon2);
    }
    return total_distance;
}

uint32_t get_seconds(char* time){
	char buffer[GPS_LEN_TIME];
	strcpy(buffer, time);
    char* token = strtok(buffer, ":");
    uint32_t a;
    a = atoi(token) * 60 * 60; token = strtok(NULL, ":");
    a += atoi(token)*60; token = strtok(NULL, ":");
    a += atoi(token);
    return (uint32_t)a;
}

uint32_t gps_get_total_time_s(Gps* gps){
    if (gps->memory_index > 0){
        uint32_t t1 = get_seconds(gps->memory.times[0]);
        uint32_t t2 = get_seconds(gps->memory.times[gps->memory_index-1]);
        return t2 - t1;
    }else{
        return 0;
    } 
}

static char* convert_seconds_to_hhmmss(uint32_t total_time){
    static char time[GPS_LEN_TIME];
    int hours = total_time/3600;
    int minutes = (total_time % 3600)/60;
    int seconds = total_time % 60;
    sprintf(time, "%02d:%02d:%02d", hours, minutes, seconds);
    return time;
}

char* gps_get_total_time(Gps* gps){
	static char time[GPS_LEN_TIME];
    strcpy(time, convert_seconds_to_hhmmss(gps_get_total_time_s(gps))); 
    return time;
}

float gps_get_instant_velocity_kmh(Gps* gps){
    if (gps->memory_index > 1){
        float lat1, lat2, lon1, lon2, distance;
        uint32_t time1, time2;
        lat1 = atof(gps->memory.positions[gps->memory_index-2].latitude);
        lon1 = atof(gps->memory.positions[gps->memory_index-2].longitude);
        lat2 = atof(gps->memory.positions[gps->memory_index-1].latitude);
        lon2 = atof(gps->memory.positions[gps->memory_index-1].longitude);
        distance = get_distance_m(lat1, lon1, lat2, lon2);
        time1 = get_seconds(gps->memory.times[gps->memory_index-2]);
        time2 = get_seconds(gps->memory.times[gps->memory_index-1]);
        return (float)((distance/(time2-time1))*3.6); 
    }else{
        return 0;
    }
}

float get_avg_velocity(float total_distance_m, uint32_t total_time_s){
	return (total_distance_m/(total_time_s))*3.6;
}

float gps_get_avg_velocity(Gps* gps){
    return get_avg_velocity(gps_get_total_distance_m(gps), gps_get_total_time_s(gps));
}

float gps_total_heigh_diff(Gps* gps){
    float heigh_diff, tot_heigh_diff = 0;
    for(int i=1; i<gps->memory_index; i++){
        heigh_diff = atof(gps->memory.positions[i].altitude) - atof(gps->memory.positions[i-1].altitude);
        if (heigh_diff > 0){
            tot_heigh_diff += heigh_diff;
        }
    }
    return tot_heigh_diff;
}
