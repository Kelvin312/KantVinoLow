#ifndef _ONE_WIRE_INCLUDED_
#define _ONE_WIRE_INCLUDED_

#include <mega8.h>
#include <delay.h>
#include "CRC8_16.h"

#define OW_DDR DDRB
#define OW_PORT PORTB
#define OW_PIN PINB

#pragma used+
extern unsigned int therm_mSec[2];

unsigned char Therm_GetTemp(char *temp, char ow_pin);
unsigned char Therm_SetConfig(char config, char ow_pin);
unsigned char Therm_SaveConfig(char ow_pin);
#pragma used-

#define sbi(reg,bt) reg |= (1<<bt)
#define cbi(reg,bt) reg &= ~(1<<bt)
#define ibi(reg,bt) reg ^= (1<<bt)
#define CheckBit(reg,bt) (reg&(1<<bt))

#define OW_CMD_SEARCHROM    0xF0
#define OW_CMD_READROM      0x33
#define OW_CMD_MATCHROM     0x55
#define OW_CMD_SKIPROM      0xCC

#define THERM_CMD_CONVERTTEMP 0x44
#define THERM_CMD_RSCRATCHPAD 0xBE
#define THERM_CMD_WSCRATCHPAD 0x4E
#define THERM_CMD_CPYSCRATCHPAD 0x48
#define THERM_CMD_RECEEPROM   0xB8
#define THERM_CMD_RPWRSUPPLY  0xB4

#define THERM_CMD_ALARMSEARCH 0xEC

#define THERM_TIME_OUT_MS 750

#define THERM_NOT_FOUND 3
#define THERM_CRC_ERROR 1
#define THERM_NOT_CONVERT 4
#define THERM_CONFIG_ERR 8

#define THERM_SET_9BIT  0b00000000 
#define THERM_SET_10BIT 0b00100000
#define THERM_SET_11BIT 0b01000000 
#define THERM_SET_12BIT 0b01100000 

#endif
