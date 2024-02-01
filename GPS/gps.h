#ifndef __GPS_H__
#define __GPS_H__

#define GPS_INVALID_DATE "99/99/99"
#define GPS_INVALID_TIME "99:99:99"		 
#define GPS_INVALID_LATITUDE "-99.999999"
#define GPS_INVALID_LONGITUDE "-999.999999"
#define GPS_INVALID_ALTITUDE "999.9"

#define GPS_HEADER_GGA "$GPGGA"
#define GPS_HEADER_RMC "$GPRMC"

#define GPS_LEN_BUFFER 100
#define GPS_LEN_MIN_BUFFER 45
#define GPS_LEN_DATE 9
#define GPS_LEN_TIME 9
#define GPS_LEN_LATITUDE 11
#define GPS_LEN_LONGITUDE 12
#define GPS_LEN_ALTITUDE 6
#define GPS_LEN_MEMORY 1000

#define EARTH_RADIUS 6378 //in km
#define PI 3.14159265358979323846

//typedef unsigned short int uint8_t;
typedef unsigned int uint32_t;
typedef char String_time[GPS_LEN_TIME];

typedef struct Position{
    char latitude[GPS_LEN_LATITUDE];
    char longitude[GPS_LEN_LONGITUDE];
    char altitude[GPS_LEN_ALTITUDE];
}Position;

typedef struct Memory{
    char date[GPS_LEN_DATE];
    String_time times[GPS_LEN_MEMORY];
    Position positions[GPS_LEN_MEMORY];
}Memory;

typedef struct Gps{
    //buffer for reading serial
    char buffer[GPS_LEN_BUFFER];
    
    //attributes per save values
    bool fix_valid;
    char date[GPS_LEN_DATE];
    char time[GPS_LEN_TIME];
    Position position;
    Memory memory;

    //internal control values
    uint32_t memory_index;
    int buffer_index;
    bool data_valid_gga;
    bool data_valid_rmc;
    int parity;
}Gps;

//private functions
static void decode_checksum_nibble(int* nibble);
static bool check_type(Gps*gps, char message_type[7]);
static bool check_parity(Gps* gps);
static void parse_gps_data_gga(Gps* gps);
static void parse_gps_data_rmc(Gps* gps);
static void fill_memory(Gps* gps);
static char* convert_pos_to_decimal(char* str);
static char* add_temporal_separator(char* string, char separator);
static double deg_to_rad(double deg);
static float get_distance_m(float lat1, float lon1, float lat2, float lon2);
unsigned int get_seconds(char* time);
static char* convert_seconds_to_hhmmss(uint32_t total_time);
float get_avg_velocity(float total_distance_m, uint32_t total_time_s);

//ext functions
void gps_init(Gps* gps);
bool gps_encode(Gps* gps, char c);
void gps_print_struct(Gps* gps);
bool gps_get_fix(Gps* gps);
float gps_get_latitude_f(Gps* gps);
float gps_get_longitude_f(Gps* gps);
float gps_get_altitude_f(Gps* gps);
char* gps_get_date(Gps* gps);
char* gps_get_time(Gps* gps);
float gps_get_instant_velocity_kmh(Gps* gps);
float gps_get_total_distance_m(Gps* gps);
char* gps_get_total_time(Gps* gps);
uint32_t gps_get_total_time_s(Gps* gps);
bool gps_memory_is_full(Gps* gps);
float gps_get_avg_velocity(Gps* gps);
float gps_total_heigh_diff(Gps* gps);

#endif
