#include "UART_function.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void UART0_config(eUSCI_UART_ConfigV1 uartConfig){
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    UART_enableModule(EUSCI_A0_BASE);
}
void UART2_config(eUSCI_UART_ConfigV1 uartConfig){
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A2_BASE, &uartConfig);
    UART_enableModule(EUSCI_A2_BASE);
    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
}

void UART_transmit_string(uint32_t moduleInstance, const char* str){
    int i;
    for(i=0; i<strlen(str); i++){
        UART_transmitData(moduleInstance, str[i]);
    }
}

void UART_transmit_all_data_race(uint32_t moduleInstance, Race* race){
        char* data_title = race_make_data_title(race);
        UART_transmit_string(moduleInstance,data_title);
        int i;
        for (i=0 ; i<race->positions_index; i++){
                UART_transmit_string(moduleInstance, race->positions[i].longitude);UART_transmit_string(moduleInstance,";");
                UART_transmit_string(moduleInstance, race->positions[i].latitude);UART_transmit_string(moduleInstance,";");
                UART_transmit_string(moduleInstance, race->positions[i].altitude);UART_transmit_string(moduleInstance,";");
            }
        UART_transmit_string(moduleInstance,"\r\n");
        free(data_title);
    }

void UART0_transmit_gps_data(Race* race){
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "speed: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_instant_speed(race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "distance: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_distance(race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "duration time: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_run_time(race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "high difference: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_heigh_difference(race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
    UART_transmit_string(EUSCI_A0_BASE, "avg speed: ");
    UART_transmit_string(EUSCI_A0_BASE, race_get_avg_speed(race));
    UART_transmit_string(EUSCI_A0_BASE, "\r\n");
}

