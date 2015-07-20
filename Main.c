/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Professional
Automatic Program Generator
� Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
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


#define ADC_ERROR0 1
#define ADC_ERROR1 3

eeprom char addressEep;

struct ADCstruct
{
    union
    {
        unsigned long real;
        char to_byte[4];
    };
    unsigned long summ;
} pressure, level; 

unsigned int meteringCount;
char temperature[2][2], swap_therm, tmp;
char error_code, errors_count[2][4], last_error;


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
error_code = 0x00;
swap_therm = 0;

Therm_SetConfig(THERM_SET_11BIT, 0);
Therm_SetConfig(THERM_SET_11BIT, 1);

while (1)
      {
          #asm("wdr")   
          
          if(++meteringCount > 512)
          {   
              pressure.real = pressure.summ;
              level.real = level.summ;
           
              pressure.summ = 0;
              level.summ = 0;
              meteringCount = 0;  
          } 
          else   //������ ���
          {
              pressure.summ += read_adc(0);
              level.summ += read_adc(1);
          }
           
          last_error = Therm_GetTemp(temperature[swap_therm], swap_therm);
          if(last_error == THERM_CONFIG_ERR || last_error == 0)
          {   
              error_code = 0;
          }
          swap_therm ^= 1;
          
          
     //     if(mSec > 800)
     //     {     
     //         mSec = 0; 
     //         error_code = 0;
     //         error_code |= GetTemperature(temperature[1], 1);
     //         error_code <<= 2;
     //         error_code |= GetTemperature(temperature[0], 0);     
     //     } 
          
          
          
          if(GetRxStatus())  //���-�� �������
          {   
          if(GetRxBuffer(0) == SKIP_ADDRESS)
          { 
              if(GetRxBuffer(1) == 0x03 && GetRxBuffer(2) == 'A') //������ ������
              {    
                  AddTransmit(address, 0);  
                  AddTransmit(GetRxBuffer(1), 1);
                  AddTransmit(6, 2);
                  AddTransmit(address, 3);   
                  StartTransmit(); 
              }   
          }
          else
          { 
              if(GetRxBuffer(1) == 0x03 && GetRxBuffer(2) == 'D') //������ ������
              {
                  error_code &= 0x0F; 
                  // 0 < x < 0x7FE00
                  
                  if(pressure.real < 16) error_code |= ADC_ERROR0<<4;
                  if(pressure.real > 523760) error_code |= ADC_ERROR1<<4;
                  if(level.real < 16) error_code |= ADC_ERROR0<<6;
                  if(level.real > 523760) error_code |= ADC_ERROR1<<6;
                     
                  tmp = GetRxBuffer(1);
                  if(error_code) tmp |= 0x80;
                  
                  AddTransmit(address, 0);  
                  AddTransmit(tmp, 1);
                  AddTransmit(18, 2);
                  
                  AddTransmits(temperature[0], 2, 3); 
                  AddTransmits(temperature[1], 2, 5);
                  AddTransmits(pressure.to_byte, 4, 7); 
                  AddTransmits(level.to_byte, 4, 11);
                  
                  AddTransmit(error_code, 15); 
                  StartTransmit();     
              }
              if(GetRxBuffer(1) == 0x10 && GetRxBuffer(2) == 'A') //����� ������
              {
                  address = GetRxBuffer(3);
                  addressEep = address;
                  
                  AddTransmit(address, 0);  
                  AddTransmit(GetRxBuffer(1), 1);
                  AddTransmit(6, 2);
                  AddTransmit('S', 3);   
                  StartTransmit();   
              }
          }  
          }
      }
}
