/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Professional
Automatic Program Generator
© Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : KantVino
Version : 0.1
Date    : 29.06.2015
Author  : 
Company : Deskom
Comments: 


Chip type               : ATmega8
Program type            : Application
AVR Core Clock frequency: 16,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*****************************************************/

#include <mega8.h>

#include <delay.h>

#include "USART.h"
#include "OneWire.h"
 
#define HOT PORTD.4
#define COLD PORTD.5
#define CLAP PORTD.6
#define ALARM PORTD.7

#define ADC_ERROR_VAL 32
#define ADC_ERROR0 1
#define ADC_ERROR1 3

eeprom char addressEep;

struct ADCstruct
{
    union
    {
        unsigned long real;
        unsigned int to_int;
        char to_byte[4];
    };
    unsigned long summ;
} pressure, level; 

unsigned int meteringCount, adc_calib1V3;
char temperature[2][2], swap_therm, isSaveConfig;
char tmp, last_error, therm_error, adc_error; 
unsigned char errors_count[2][4];


// Standard Input/Output functions
#include <stdio.h>


#define ADC_VREF_TYPE 0x40

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=0x40;
// Wait for the AD conversion to complete
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;
return ADCW;
}


// Declare your global variables here

void main(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTB=0x00;
DDRB=0x00;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=In Func2=Out Func1=In Func0=In 
// State7=0 State6=0 State5=0 State4=0 State3=T State2=0 State1=T State0=T 
PORTD=0x00;
DDRD=0xF4;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x01;

USART_and_Timer0_Init();

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// ADC initialization
// ADC Clock frequency: 125,000 kHz
// ADC Voltage Reference: AVCC pin
ADMUX=ADC_VREF_TYPE & 0xff;
ADCSRA=0x87;

// SPI initialization
// SPI disabled
SPCR=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;

// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/64k
#pragma optsize-
WDTCR=0x1A;
WDTCR=0x0A;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Global enable interrupts
#asm("sei")

   
address = addressEep;
meteringCount = 0;
pressure.summ = 0;
level.summ = 0;
swap_therm = 0;
therm_error = 0x0F;
isSaveConfig = 3;

Therm_SetConfig(THERM_SET_11BIT, 0);
Therm_SetConfig(THERM_SET_11BIT, 1);

while (1)
      {
          #asm("wdr")   
          
          if(++meteringCount > 512)
          {   
              pressure.real = pressure.summ >> 3;
              level.real = level.summ >> 3;
           
              pressure.summ = 0;
              level.summ = 0;
              meteringCount = 0;  
          } 
          else   //Читаем АЦП
          {
              pressure.summ += read_adc(0);
              level.summ += read_adc(1);
          }
           
          //Читаем термометры
          last_error = Therm_GetTemp(temperature[swap_therm], swap_therm); 

          if(swap_therm)
          {
              therm_error &= (last_error<<2) | 3;
              if((isSaveConfig & 2) && last_error == THERM_CONFIG_ERR)
              {
                        Therm_SaveConfig(swap_therm);
                        isSaveConfig &= 1;
              }
          }
          else
          {
              therm_error &= (last_error) | 12;
              if((isSaveConfig & 1) && last_error == THERM_CONFIG_ERR)
              {
                        Therm_SaveConfig(swap_therm);
                        isSaveConfig &= 2;
              }
          }
          
          if((last_error & THERM_NOT_FOUND) == THERM_NOT_FOUND)
          {
              errors_count[swap_therm][0]++;
          }
          else 
          {
            if(last_error & THERM_CRC_ERROR)
            {
              errors_count[swap_therm][1]++;
            }
            if(last_error & THERM_NOT_CONVERT)
            {
              errors_count[swap_therm][2]++;
            }
            if(last_error & THERM_CONFIG_ERR)
            {
              errors_count[swap_therm][3]++;
            }
          }
          
          swap_therm ^= 1;
          //Закончили с термометрами
          
          if(GetRxStatus())  //Что-то приняли
          {   
          if(GetRxBuffer(0) == SKIP_ADDRESS)
          { 
              if(GetRxBuffer(1) == 0x03 && GetRxBuffer(2) == 'A') //Чтение адреса
              {    
                  AddTransmit(address);  
                  AddTransmit(GetRxBuffer(1));
                  AddTransmit(6);
                  AddTransmit(address);   
                  StartTransmit(); 
              }   
          }
          else
          { 
              if(GetRxBuffer(1) == 0x03 && GetRxBuffer(2) == 'D') //Чтение данных
              {
                  adc_error = 0x00; 
                  // 0 < x < 0x7FE00
                  
                  if(pressure.to_int < ADC_ERROR_VAL) adc_error |= ADC_ERROR0;
                  if(pressure.to_int > (0xFFC0 - ADC_ERROR_VAL)) adc_error |= ADC_ERROR1;
                  if(level.to_int < ADC_ERROR_VAL) adc_error |= ADC_ERROR0<<2;
                  if(level.to_int > (0xFFC0 - ADC_ERROR_VAL)) adc_error |= ADC_ERROR1<<2;
                     
                  tmp = GetRxBuffer(1);
                  if(adc_error || therm_error) tmp |= 0x80;
                  
                  AddTransmit(address);  
                  AddTransmit(tmp);
                  AddTransmit(14);
                  
                  AddTransmits(temperature[0], 2); 
                  AddTransmits(temperature[1], 2);
                  AddTransmits(pressure.to_byte, 2); 
                  AddTransmits(level.to_byte, 2);
                  
                  AddTransmit((adc_error<<4) | therm_error); 
                  
                  StartTransmit();
                  
                  therm_error = 0x0F;     
              }
              if(GetRxBuffer(1) == 0x10 && GetRxBuffer(2) == 'A') //Смена адреса
              {
                  address = GetRxBuffer(3);
                  addressEep = address;
                  
                  AddTransmit(address);  
                  AddTransmit(GetRxBuffer(1));
                  AddTransmit(6);
                  AddTransmit('S');   
                  StartTransmit();   
              }
              
              if(GetRxBuffer(1) == 0x03 && GetRxBuffer(2) == 'E') //Чтение ошибок 
              {
                  adc_calib1V3 = read_adc(14); //1110 = 1.30V (VBG)
              
                  AddTransmit(address);  
                  AddTransmit(GetRxBuffer(1));
                  AddTransmit(9 + 10);
                  AddUpTime(); //3+4+2=9 
                  
                  AddTransmits(errors_count[0], 4);
                  AddTransmits(errors_count[1], 4);
                  AddTransmit(adc_calib1V3 & 0xFF);
                  AddTransmit(adc_calib1V3 >> 8);
                     
                  StartTransmit(); 
                  
                  for(tmp=0; tmp<4; tmp++)
                  {
                      errors_count[0][tmp]=0; 
                      errors_count[1][tmp]=0;
                  }
              }
              
              if(GetRxBuffer(1) == 0x10 && GetRxBuffer(2) == 'O') //Установка выходов
              {        
                  tmp = GetRxBuffer(3);
                  if(CheckBit(tmp,7)) HOT = (CheckBit(tmp,3))? 1:0;
                  if(CheckBit(tmp,6)) COLD = (CheckBit(tmp,2))? 1:0;
                  if(CheckBit(tmp,5)) CLAP = (CheckBit(tmp,1))? 1:0;
                  if(CheckBit(tmp,4)) ALARM = (CheckBit(tmp,0))? 1:0;
                  tmp = (HOT<<3) | (COLD<<2) | (CLAP<<1) | ALARM; 
                  
                  AddTransmit(address);  
                  AddTransmit(GetRxBuffer(1));
                  AddTransmit(6);
                  AddTransmit(tmp);
                  
                  StartTransmit();    
              }
              
          }
          ClearRxBuffer();  
          }
      }
}
