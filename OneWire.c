#include "OneWire.h"

unsigned char ow_bit;
//unsigned int therm_mSec[2];
unsigned char therm_config[2];

unsigned char shiftMas[8] = {1, 2, 4, 8, 16, 32, 64, 128};


inline void OW_Set(unsigned char mode)
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

inline unsigned char OW_CheckIn()
{
    return CheckBit(OW_PIN, ow_bit);
}

unsigned char OW_Reset(void) //Сброс и сигнал присутствия
{
	unsigned char status;
	OW_Set(1);
	delay_us(480);
	OW_Set(0);
	delay_us(60);
	//Store line value and wait until the completion of 480uS period
	status = OW_CheckIn();
	delay_us(420);
	//Return the value read from the presence pulse (0=OK, 1=WRONG)
  return status; 
}

inline void OW_WriteBit(unsigned char bt)
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

inline unsigned char OW_ReadBit(void)
{
	unsigned char bt=0;
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
	unsigned char i, res = 0;
	for (i=0; i<8; i++) if (OW_ReadBit()) sbi(res, i);
	return res;
}


unsigned char Therm_ReadData(char *data)
{
    unsigned char i, res, err_code = 0;  
    char temp[2];
    if(OW_Reset()) return THERM_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);        //Широковещательный адрес 
    OW_WriteByte(THERM_CMD_RSCRATCHPAD); //Считать 9 байт памяти
    
    CRC8_Clear();
    for(i=0; i<9; i++)
    {
        res = OW_ReadByte();
        CRC8_Add(res);
        if(i<2) temp[i] = res; 
        //Проверка конфигурации
        if(i==4 && therm_config[ow_bit] != (res & 0b01100000)) 
            err_code = THERM_CONFIG_ERR;   
    }
    if(CRC8_Check()) return THERM_CRC_ERROR; 
    data[0] = temp[0];
    data[1] = temp[1]; 
    return err_code;      
}

unsigned char Therm_WriteData(char *data)
{
    unsigned char i;
    if(OW_Reset()) return THERM_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);
    OW_WriteByte(THERM_CMD_WSCRATCHPAD);  //Запись 3х байт
    for(i=0; i<3; i++)
    {
        OW_WriteByte(data[i]);
    }
    return 0;
}

unsigned char Therm_StartConvert()
{
    if(OW_Reset()) return THERM_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);
    OW_WriteByte(THERM_CMD_CONVERTTEMP);
    return 0; 
}

unsigned char Therm_SaveData()
{
    if(OW_Reset()) return THERM_NOT_FOUND;
    OW_WriteByte(OW_CMD_SKIPROM);           //Широковещательный адрес
    OW_WriteByte(THERM_CMD_CPYSCRATCHPAD);  //Сохранение настроек
    return 0; 
}



unsigned char Therm_GetTemp(char *temp, char ow_pin)
{
    unsigned char res = 0;
    ow_bit = ow_pin;
    if(!OW_ReadBit()) //Не прочитали
    {
        if(therm_mSec[ow_bit] > THERM_TIME_OUT_MS)
        {
            res |= THERM_NOT_CONVERT;
        }
        else return res;
    }
    //Прочитали 
       
        therm_mSec[ow_bit] = 0;
        res |= Therm_ReadData(temp);
        if(res == THERM_CONFIG_ERR || res == 0)
        {
            if(temp[0] == 0x50 && temp[1] == 0x05) 
                res |= THERM_NOT_CONVERT;    
                
            res |= Therm_StartConvert();
        }
         
    return res;
}

void Therm_SetConfig(char config, char ow_pin)
{
    ow_bit = ow_pin; 
    therm_config[ow_bit] = config;
}

unsigned char Therm_SaveConfig(char ow_pin)
{
    unsigned char res=0, temp[3]={0x00, 0x7F, 0}; 
    ow_bit = ow_pin; 
    temp[2] = therm_config[ow_bit];  
    
    res |= Therm_WriteData(temp); 
    if(res == THERM_NOT_FOUND) return THERM_NOT_FOUND;
    
    res |= Therm_ReadData(temp); 
    if(res == 0)
    {
        res |= Therm_SaveData();        
    }
    return res;  
}





