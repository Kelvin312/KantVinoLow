#include "USART.h"


#ifndef RXB8
#define RXB8 1
#endif

#ifndef TXB8
#define TXB8 0
#endif

#ifndef UPE
#define UPE 2
#endif

#ifndef DOR
#define DOR 3
#endif

#ifndef FE
#define FE 4
#endif

#ifndef UDRE
#define UDRE 5
#endif

#ifndef RXC
#define RXC 7
#endif

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

char address;
unsigned int mSec;

// USART Receiver buffer
char rx_buffer[RX_BUFFER_SIZE];
unsigned char rx_counter, rx_mSec, rx_status;

// USART Transmitter buffer
char tx_buffer[TX_BUFFER_SIZE];
unsigned char tx_index, tx_counter, tx_mSec, tx_status;


// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
  char status, data; 
  status = UCSRA;
  data = UDR;
    
  rx_mSec = 0;
  if(rx_status > READ_DATA) return;
  
  if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN)) == 0)
  {   
      if((data == address || data == SKIP_ADDRESS)  && rx_status == READ_WAIT)
      { 
          rx_status = READ_DATA;  //Начинаем чтение посылки с адреса устройства
          rx_counter = 0; 
          CRC16_clear();
      }
      if(rx_status == READ_DATA)
      {
          rx_buffer[rx_counter++] = data;
          
          CRC16_add(data);  //Считаем CRC
          
          if(rx_counter == RX_COMMAND_SIZE)
          {
              rx_status = READ_COMPLETE;
              if(CRC16_check()) rx_status = READ_WAIT;  //Не прошли проверку CRC    
          }
      } 
   }
}


inline void usart_tx_fun()
{
    if (tx_index < tx_counter)
    {
        UDR = tx_buffer[tx_index++];
    }
    else
    {
       tx_status = WRITE_END; //Переключение на прием
       tx_mSec = 0;
    }
}

char get_status()
{
    if(rx_status == READ_COMPLETE)
    {
        rx_status = READ_WAIT;
        return 1;
    }
    return 0;
}

void start_transmit()
{
    unsigned int this_crc;
    unsigned char count = tx_buffer[3] - 3;
    while(tx_status == WRITE_IN_PROCESSED); //Если передача не закончилась     
    
    this_crc = CRC16_all(tx_buffer+1, count - 1); //Считаем CRC с 1 по последний элемент
    tx_buffer[count++] = this_crc >> 8; 
    tx_buffer[count++] = this_crc & 0xFF;
    tx_buffer[count++] = '*';
     
    #asm("cli")
    tx_counter = count;
    tx_index = 0; 
    
    if(tx_status == WRITE_WAIT)
    {
        RS485 = 1; //Переключаем на передачу  
        tx_status = WRITE_START;
        tx_mSec = 0;
    }
    else if(tx_status == WRITE_END)
    { 
        tx_status = WRITE_IN_PROCESSED; 
        if((UCSRA & DATA_REGISTER_EMPTY) != 0)
        { 
            usart_tx_fun();
        }
    }
    #asm("sei")  
}

// USART Transmitter interrupt service routine
interrupt [USART_TXC] void usart_tx_isr(void)
{
    usart_tx_fun();
}


// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    TCNT0=0x05; // Reinitialize Timer 0 value
    mSec++;
 
    if(rx_status == READ_DATA && ++rx_mSec > RX_TIMEOUT)
    {
        rx_status = READ_WAIT; //Время чтения посылки вышло
    }
    
    tx_mSec++;
    if(tx_status == WRITE_START && tx_mSec > TX_START_TIME)
    {
         tx_status = WRITE_IN_PROCESSED;
         usart_tx_fun();
    }
    
    if(tx_status == WRITE_END && tx_mSec > TX_END_TIME)
    {
        tx_status = WRITE_WAIT;
        RS485 = 0;
    }
}


void USART_Init()
{
    rx_status = READ_WAIT;
    tx_status = WRITE_WAIT;
    RS485 = 0; 
    
    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 38400
    UCSRA=0x00;
    UCSRB=0xD8;
    UCSRC=0x86;
    UBRRH=0x00;
    UBRRL=0x19;
}
