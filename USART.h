#include <mega8.h>
#include "CRC8_16.h"

#define RS485 PORTD.2
#define RS_READ 0
#define RS_WRITE 1


// USART Receiver buffer
#define RX_BUFFER_SIZE 96

#define RX_COMMAND_SIZE 6
#define RX_TIMEOUT 4

#define SKIP_ADDRESS 0xCC
extern char address; //Адрес устройства

char GetRxStatus();
inline char GetRxBuffer(char index);
inline void ClearRxBuffer(char size);


// USART Transmitter buffer
#define TX_BUFFER_SIZE 32

#define TX_WR_OFF 0
#define TX_WR_INIT 1
#define TX_WR_PROCESS 2

#define TX_START_TIME 6
#define TX_END_TIME 8


void AddTransmits(char *data, char size, char index);
void AddTransmit(char data, char index);
void AddUpTime(char index);
void StartTransmit();
inline char GetTxBuffer(char index);


void USART_and_Timer0_Init();

interrupt [USART_RXC] void usart_rx_isr(void);
interrupt [USART_TXC] void usart_tx_isr(void);
interrupt [TIM0_OVF] void timer0_ovf_isr(void);


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

