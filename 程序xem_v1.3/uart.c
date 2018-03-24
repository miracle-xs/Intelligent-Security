#include "msp430f2132.h"
#include "UART.h"

void InitUART(void)
{
  /*
  P3SEL |= BIT4+BIT5;                       // P3.4,5 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
  UCA0BR0 = 0x03;                           // 32kHz/9600 = 3.41
  UCA0BR1 = 0x00;                           //
  UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  */
   
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;                    
  P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 8;                              // 1MHz 115200
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  __bis_SR_register(GIE); 
 //__bis_SR_register(LPM3_bits + GIE);       // Enter LPM3, interrupts enabled
}
/*******************************************
函数名称：R0SendChar
功    能：向PC机发送一个字符
参    数：sendchar--要发送的字符
返回值  ：无
********************************************/
void R0SendChar(unsigned char sendchar)
{
  while (!(IFG2&UCA0TXIFG));    //等待发送寄存器为空         
  UCA0TXBUF = sendchar; 
}

/*******************************************
函数名称：R0SendChar1
功    能：向PC机发送一个字符
参    数：sendchar--要发送的字符
返回值  ：无
********************************************/
void R0SendChar1(char sendchar)
{
  while (!(IFG2&UCA0TXIFG));    //等待发送寄存器为空         
  UCA0TXBUF = sendchar; 
}

/*******************************************
函数名称：R0PutString
功    能：向PC机发送字符串，无换行
参    数：ptr--指向发送字符串的指针
返回值  ：无
********************************************/
void R0PutString(unsigned char *ptr)
{
  while(*ptr != '\0')
  {
    R0SendChar(*ptr++);                     // 发送数据
  }
}

/*******************************************
函数名称：R0PutString1
功    能：向PC机发送字符串，无换行
参    数：ptr--指向发送字符串的指针
返回值  ：无
********************************************/
void R0PutString1(char *ptr)
{
  while(*ptr != '\0')
  {
    R0SendChar1(*ptr++);                     // 发送数据
  }
}
/*******************************************
函数名称：R0ReceiveChar
功    能：接收PC机发过来的字符
参    数：无
返回值  ：接收来的字符
********************************************/
uchar R0ReceiveChar()
{
  uchar receivechar;
  while(IFG2&UCA0RXIFG)
  {
    //delay_ms(1000);
    //break;
    receivechar = UCA0RXBUF;
    //IFG2 = 0;
  }
   //receivechar = UCA0RXBUF;
   return receivechar;
}


