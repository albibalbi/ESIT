#ifndef __GPS_H__
#define __GPS_H__

#include <stdbool.h>

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

typedef struct Position{
    char latitude[GPS_LEN_LATITUDE];
    char longitude[GPS_LEN_LONGITUDE];
    char altitude[GPS_LEN_ALTITUDE];
}Position;

typedef struct Gps{
    //buffer for reading serial
    char buffer[GPS_LEN_BUFFER];
    unsigned short int buffer_index;

    //attributes per save values
    bool fix_valid;
    char date[GPS_LEN_DATE];
    char time[GPS_LEN_TIME];
    Position position;

    char old_time[GPS_LEN_TIME];
    Position old_position;

    //internal control values
    bool data_valid_gga;
    bool data_valid_rmc;
    bool data_valid;
    int parity;
}Gps;

//parse functions
static void decode_checksum_nibble(int* nibble);
static bool check_type(Gps*gps, char message_type[7]);
static bool check_parity(Gps* gps);
static void parse_gps_data_gga(Gps* gps);
static void parse_gps_data_rmc(Gps* gps);
static char* convert_pos_to_decimal(char* str);
static char* fill_string_attribute(char* attribute, char* value );
static char* set_position_sign(char* value, char* token);
static char* add_temporal_separator(char* string, char separator);

//x speed caluclations
static double deg_to_rad(double deg);
float get_distance_m(float lat1, float lon1, float lat2, float lon2);
unsigned int get_seconds_from_time(char* time);


//----GPS functions----
//encoding and control functions
void gps_init(Gps* gps);
void gps_encode(Gps* gps, char c);
bool gps_data_valid(Gps* gps);
void gps_set_data_valid(Gps* gps, bool value);
bool gps_get_fix(Gps* gps);

//get attribute functions
Position gps_get_positions(Gps gps);
const char* gps_get_latitude(Gps* gps);
const char* gps_get_longitude(Gps* gps);
const char* gps_get_altitude(Gps* gps);
const char* gps_get_date(Gps* gps);
const char* gps_get_time(Gps* gps);
unsigned int gps_get_last_seconds(Gps* gps);
float gps_get_last_distance(Gps* gps);
float gps_get_last_altitude(Gps* gps);
float gps_get_instant_speed(Gps* gps);

void gps_print_struct(Gps* gps);      //test function

#endif
