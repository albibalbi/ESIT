#ifndef __UART_FUNCTION_H__
#define __UART_FUNCTION_H__


#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "race.h"
#include "booster_pack_pin_map.h"



void UART0_config(eUSCI_UART_ConfigV1 uartConfig);  //config UART instance 0 and enable interrupt. Serial monitor UART
void UART2_config(eUSCI_UART_ConfigV1 uartConfig);  //config UART instance 2 and enable interrupt. GPS UART
void UART0_transmit_gps_data( Race* race);                    //transmit gps data via serial monitor (testing function)
void UART_transmit_string(uint32_t moduleInstance, const char* str);    //send string via UART
void UART_transmit_all_data_race(uint32_t moduleInstance, Race* race);  //send all data collected via UART







#endif
