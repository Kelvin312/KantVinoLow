#include <mega8.h>
#include "CRC8_16.h"


#define RS485 PORTD.2

#define READ_WAIT 0
#define READ_DATA 1
#define READ_COMPLETE 2

#define WRITE_WAIT 0
#define WRITE_START 1
#define WRITE_END 2
#define WRITE_IN_PROCESSED 3

#define RX_COMMAND_SIZE 6
#define RX_TIMEOUT 4

#define TX_START_TIME 10
#define TX_END_TIME 10

#define SKIP_ADDRESS 0xCC

extern char address; //Адрес устройства
extern unsigned int mSec;

// USART Receiver buffer
#define RX_BUFFER_SIZE 8
extern char rx_buffer[RX_BUFFER_SIZE];

// USART Transmitter buffer
#define TX_BUFFER_SIZE 32
extern char tx_buffer[TX_BUFFER_SIZE];

void USART_Init();
void start_transmit();
char get_status();
interrupt [USART_RXC] void usart_rx_isr(void);
interrupt [USART_TXC] void usart_tx_isr(void);
interrupt [TIM0_OVF] void timer0_ovf_isr(void);
