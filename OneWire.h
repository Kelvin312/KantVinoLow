#include <mega8.h>
#include <delay.h>
#include "CRC8_16.h"

#define OW_DDR DDRB
#define OW_PORT PORTB
#define OW_PIN PINB

#define DEVICE_NOT_FOUND 3
#define ERROR_CRC 1

#define TERM_9BIT  0b00011111 
#define TERM_10BIT 0b00111111
#define TERM_11BIT 0b01011111 
#define TERM_12BIT 0b01111111 

#pragma used+
unsigned char GetTemperature(char *temper, unsigned char ow_pin);
unsigned char SetConfigTherm(char config, unsigned char ow_pin);
#pragma used-
