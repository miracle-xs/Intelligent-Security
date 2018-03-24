#ifndef _UART_H_
#define _UART_H_

#define uchar unsigned char
#define uint unsigned int

#define CPU_F ((double)800000000.0)
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

extern uchar GSM_RXBUF[50];
extern uchar GSM_RXIFG;
extern uchar GSM_COUNT;
/******************/

void InitUART(void);
void R0SendChar(unsigned char sendchar);
void R0SendChar1(char sendchar);
void R0PutString(unsigned char *ptr);
void R0PutString1(char *ptr);
uchar R0ReceiveChar();
#endif