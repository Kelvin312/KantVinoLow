#include "USART.h"

char address, transmit_flag;
unsigned char upTime[4];
char upTimePrescaler;
unsigned int therm_mSec[2];

// USART Receiver buffer
char rx_buffer[RX_BUFFER_SIZE];
unsigned char rx_wr_index, rx_rd_index, rx_counter;

// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
    char status, data; 
    status = UCSRA;
    data = UDR;
   
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
    { 
        rx_buffer[rx_wr_index++] = data;
        if (++rx_counter == RX_BUFFER_SIZE)
        {
            rx_counter = RX_BUFFER_SIZE-1;
            rx_wr_index--;
        }
        if (rx_wr_index == RX_BUFFER_SIZE) rx_wr_index=0;
   }
}
 
inline char GetRxBuffer(char index)
{
    index += rx_rd_index;
    if(index >= RX_BUFFER_SIZE) index -= RX_BUFFER_SIZE;
    return rx_buffer[index];
}

inline void ClearAllRxBuffer()
{
    #asm("cli")
    rx_counter = 0;
    rx_wr_index = 0; 
    rx_rd_index = 0;
    #asm("sei")
}

inline void ClearRxBuffer()
{
    char size = RX_COMMAND_SIZE;
    if(rx_counter < size) return; 
    #asm("cli") 
    rx_counter -= size;
    #asm("sei")
    rx_rd_index += size; 
    if (rx_rd_index >= RX_BUFFER_SIZE) rx_rd_index -= RX_BUFFER_SIZE;
}
 
char GetRxStatus()
{
    char temp, i;
    if(rx_counter < RX_COMMAND_SIZE) return 0; 
    
    for(; rx_counter >= RX_COMMAND_SIZE; rx_rd_index++)
    {         
        if (rx_rd_index == RX_BUFFER_SIZE) rx_rd_index=0; 
        /*
        0  adr  0xCC
        1  0x03 0x10
        2  'A'  'D'  'E' 'O'
        */
        
        temp = GetRxBuffer(0); 
        if (temp == address || temp == SKIP_ADDRESS)
        {  
            temp = GetRxBuffer(1); 
            if(temp == 0x03 || temp == 0x10)
            {
                temp = GetRxBuffer(2); 
                if(temp == 'A' || temp == 'D' || temp == 'E' || temp == 'O')
                {   
                    CRC16_Clear();
                    for(i=0; i < RX_COMMAND_SIZE; i++)
                    { 
                         CRC16_Add(GetRxBuffer(i));
                    } 
                    //Прошли все проверки
                    if(!CRC16_Check()) return 1;
                }
            }
        }
        #asm("cli")
        rx_counter--; 
        #asm("sei")
    }
    return 0;    
}  
  
  
  

// USART Transmitter buffer
char tx_buffer[TX_BUFFER_SIZE];
unsigned char tx_wr_index,tx_rd_index,tx_counter;
unsigned char tx_mSec, tx_status, tx_st_index;

inline void usart_tx_fun()
{
    if (tx_counter && tx_status == TX_WR_PROCESS)
    {
        if(--tx_counter == 0)
        tx_mSec = 0;
        UDR = tx_buffer[tx_rd_index++];
        if (tx_rd_index == TX_BUFFER_SIZE) tx_rd_index = 0;
    }
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
    
    tx_mSec++; 
    if(tx_status == TX_WR_INIT && tx_mSec > TX_START_TIME) 
    {
        tx_status = TX_WR_PROCESS; 
        tx_mSec = 0;
        usart_tx_fun();      
    } 
    
    if(tx_status == TX_WR_PROCESS && tx_mSec > TX_END_TIME) 
    {  
        tx_status = TX_WR_OFF;
        tx_mSec = 0;
        RS485 = RS_READ; 
        if(transmit_flag)
        {
            transmit_flag = 0;
            RS485 = RS_WRITE; 
            tx_status = TX_WR_INIT;  
        } 
    }
  
    if(++upTimePrescaler > 99)
    { 
        upTimePrescaler = 0; 
        if(upTime[0]++ == 0xFF)
        {
            if(upTime[1]++ == 0xFF)
            {
                if(upTime[2]++ == 0xFF)
                {
                    upTime[3]++;
                }
            }
        }
    } 
    
    therm_mSec[0]++;
    therm_mSec[1]++;
} 
  
 
void USART_and_Timer0_Init()
{
    therm_mSec[0]=0;
    therm_mSec[1]=0;
    tx_mSec = 0;
    upTimePrescaler = 0;
    upTime[0] = 0;
    upTime[1] = 0;
    upTime[2] = 0;
    upTime[3] = 0;
    
    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 250,000 kHz
    TCCR0=0x03;
    TCNT0=0x05;   
    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK |= 0x01;
        
     
    transmit_flag = 0;
    tx_status = TX_WR_OFF;
    RS485 = RS_READ; 
    tx_rd_index = 0;
    tx_counter = 0;
    tx_st_index = 0;
    tx_wr_index = tx_st_index + 1;
    
    ClearAllRxBuffer();
    
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

inline char GetTxBuffer(char index)
{
    index += tx_st_index + 1;
    if(index >= TX_BUFFER_SIZE) index -= TX_BUFFER_SIZE;
    return tx_buffer[index];
}

void AddTransmits(char *data, char size)
{
    while(size--)
    {
        tx_buffer[tx_wr_index] = *data++;
        if(++tx_wr_index == TX_BUFFER_SIZE) tx_wr_index = 0;
    }
}

void AddTransmit(char data)
{   
    tx_buffer[tx_wr_index] = data;
    if(++tx_wr_index == TX_BUFFER_SIZE) tx_wr_index = 0;
}

void AddUpTime()
{
    AddTransmits(upTime, 4);
}

void StartTransmit()
{
    char i, temp[2];
    unsigned char size = GetTxBuffer(2); 
    size -= 2;      
    
    CRC16_Clear();
    for(i=0; i < size; i++)
    { 
        CRC16_Add(GetTxBuffer(i));
    }     
    CRC16_Get(temp);
    AddTransmits(temp, 2);
    
    AddTransmit(0x2A);
    tx_buffer[tx_st_index] = 0x55; 
    size += 4;
    #asm("cli")
    tx_counter += size;
    #asm("sei")
    tx_st_index += size;  
    tx_wr_index = tx_st_index + 1; 
    if(tx_st_index >= TX_BUFFER_SIZE) tx_st_index -= TX_BUFFER_SIZE;  
    if(tx_wr_index >= TX_BUFFER_SIZE) tx_wr_index -= TX_BUFFER_SIZE;  
    
     if(tx_status == TX_WR_OFF)
     {
        RS485 = RS_WRITE; //Переключаем на передачу 
        tx_mSec = 0; 
        tx_status = TX_WR_INIT;  
     }
     else
     {
          transmit_flag = 1;
     }
}




