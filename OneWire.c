#include "OneWire.h"

unsigned char ow_bit;

#define sbi(reg,bt) reg |= (1<<bt)
#define cbi(reg,bt) reg &= ~(1<<bt)
#define ibi(reg,bt) reg ^= (1<<bt)
#define CheckBit(reg,bt) (reg&(1<<bt))

#define OW_CMD_SEARCHROM	0xF0
#define OW_CMD_READROM		0x33
#define OW_CMD_MATCHROM		0x55
#define OW_CMD_SKIPROM		0xCC

#define THERM_CMD_CONVERTTEMP 0x44
#define THERM_CMD_RSCRATCHPAD 0xBE
#define THERM_CMD_WSCRATCHPAD 0x4E
#define THERM_CMD_CPYSCRATCHPAD 0x48
#define THERM_CMD_RECEEPROM 0xB8
#define THERM_CMD_RPWRSUPPLY 0xB4

#define THERM_CMD_ALARMSEARCH 0xEC

void OW_Set(unsigned char mode)
{
  if (mode) 
  {
    cbi(OW_PORT, ow_bit); 
    sbi(OW_DDR, ow_bit);
  }
  else 
  {
    cbi(OW_PORT, ow_bit); 
    cbi(OW_DDR, ow_bit);
  }
}

unsigned char OW_CheckIn()
{
  return CheckBit(OW_PIN, ow_bit);
}


unsigned char OW_Reset(void) //Сброс и сигнал присутствия
{
	unsigned char	status;
	OW_Set(1);
	delay_us(480);
	OW_Set(0);
	delay_us(60);
	//Store line value and wait until the completion of 480uS period
	status = OW_CheckIn();
	delay_us(420);
	//Return the value read from the presence pulse (0=OK, 1=WRONG)
 return !status;
//	return 1 if found
//	return 0 if not found
}


void OW_WriteBit(unsigned char bt)
{
	//Pull line low for 1uS
	OW_Set(1);
	delay_us(1);
	//If we want to write 1, release the line (if not will keep low)
	if(bt) OW_Set(0); 
	//Wait for 60uS and release the line
	delay_us(60);
	OW_Set(0);
  delay_us(1);
}

unsigned char OW_ReadBit(void)
{
	unsigned char	bt=0;
	//Pull line low for 1uS
	OW_Set(1);
	delay_us(1);
	//Release line and wait for 14uS
	OW_Set(0);
	delay_us(13);
	//Read line value
	if(OW_CheckIn()) bt=1;
	//Wait for 45uS to end and return read value
	delay_us(47);
	return bt;	
}

void OW_WriteByte(unsigned char byte)
{
  unsigned char i;
	for (i=0; i<8; i++) OW_WriteBit(CheckBit(byte, i));
}

unsigned char OW_ReadByte(void)
{
	unsigned char n=0, i;
	for (i=0; i<8; i++) if (OW_ReadBit()) sbi(n, i);
	return n;
}

unsigned char GetTemperature(char *temper, unsigned char ow_pin)
{
    char i, temp; 
    ow_bit = ow_pin;
     
    //Читаем температуру
    if(!OW_Reset()) return DEVICE_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);         //Широковещательный адрес
    OW_WriteByte(THERM_CMD_RSCRATCHPAD);  //Считать 9 байт памяти
    CRC8_clear();
    temper[0] = temp = OW_ReadByte();
    CRC8_add(temp);
    temper[1] = temp = OW_ReadByte();
    CRC8_add(temp);
    for(i=2; i<9; i++)
    {
        temp = OW_ReadByte();
        CRC8_add(temp);
    }
 
    //Запускаем конвертирование
    if(!OW_Reset()) return DEVICE_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);
    OW_WriteByte(THERM_CMD_CONVERTTEMP);
     
    if(CRC8_check()) return ERROR_CRC;
    return 0;
}

unsigned char SetConfigTherm(char config, unsigned char ow_pin)
{
    ow_bit = ow_pin;
    if(!OW_Reset()) return DEVICE_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);         //Широковещательный адрес
    OW_WriteByte(THERM_CMD_WSCRATCHPAD);  //Запись 3х байт
    OW_WriteByte(0x00);
    OW_WriteByte(0x7F);
    OW_WriteByte(config);
    
    if(!OW_Reset()) return DEVICE_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);           //Широковещательный адрес
    OW_WriteByte(THERM_CMD_CPYSCRATCHPAD);  //Сохранение настроек
    return 0;
}






