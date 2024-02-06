#ifndef __BOOSTER_PACK_PIN_MAP_H__
#define __BOOSTER_PACK_PIN_MAP_H__

//MSP42
#define BUTTON_LEFT         GPIO_PORT_P1, GPIO_PIN1
#define BUTTON_RIGHT        GPIO_PORT_P1, GPIO_PIN4
#define UART0_RX            GPIO_PORT_P1, GPIO_PIN2
#define UART0_TX            GPIO_PORT_P1, GPIO_PIN3

//J1
#define JOYSTICK_X          GPIO_PORT_P6, GPIO_PIN0
#define UART2_RX            GPIO_PORT_P3, GPIO_PIN2
#define UART2_TX            GPIO_PORT_P3, GPIO_PIN3
#define JOYSTIC_BUTTON      GPIO_PORT_P4, GPIO_PIN1
#define MICHROPHONE         GPIO_PORT_P4, GPIO_PIN3
#define LCD_SPI_CLK         GPIO_PORT_P1, GPIO_PIN5
#define AMBIENT_LIGHT_INT   GPIO_PORT_P4, GPIO_PIN6
#define AMBIENT_LIGHT_SCL   GPIO_PORT_P6, GPIO_PIN5
#define AMBIENT_LIGHT_SDA   GPIO_PORT_P6, GPIO_PIN4
#define TEMP_SENSOR_SCL     GPIO_PORT_P6, GPIO_PIN5
#define TEMP_SENSOR_SDA     GPIO_PORT_P6, GPIO_PIN4

//J2
#define SERVO_PWM           GPIO_PORT_P2, GPIO_PIN5
#define LCD_RST             GPIO_PORT_P5, GPIO_PIN7
#define LCD_MOSI            GPIO_PORT_P1, GPIO_PIN6
#define LCD_CS              GPIO_PORT_P5, GPIO_PIN0
#define TEMP_SENSOR         GPIO_PORT_P3, GPIO_PIN6

//J3
#define ACCELEROMETER_X     GPIO_PORT_P6, GPIO_PIN1
#define ACCELEROMETER_Y     GPIO_PORT_P4, GPIO_PIN0
#define ACCELEROMETER_Z     GPIO_PORT_P4, GPIO_PIN2
#define JOYSTICK_Y          GPIO_PORT_P4, GPIO_PIN5

//J4
#define BUZZER_OUT          GPIO_PORT_P2, GPIO_PIN7
#define RGB_LED_RED         GPIO_PORT_P2, GPIO_PIN6
#define RGB_LED_GREEN       GPIO_PORT_P2, GPIO_PIN4
#define RGB_LED_BLUE        GPIO_PORT_P5, GPIO_PIN6
#define WIFI_CONN           GPIO_PORT_P6, GPIO_PIN7
#define GATOR_HOLE          GPIO_PORT_P2, GPIO_PIN3
#define BUTTON_UP           GPIO_PORT_P5, GPIO_PIN1
#define BUTTON_DOWN         GPIO_PORT_P3, GPIO_PIN5
#define LCD_RS              GPIO_PORT_P3, GPIO_PIN7

#endif
