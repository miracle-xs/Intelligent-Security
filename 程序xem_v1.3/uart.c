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
�������ƣ�R0SendChar
��    �ܣ���PC������һ���ַ�
��    ����sendchar--Ҫ���͵��ַ�
����ֵ  ����
********************************************/
void R0SendChar(unsigned char sendchar)
{
  while (!(IFG2&UCA0TXIFG));    //�ȴ����ͼĴ���Ϊ��         
  UCA0TXBUF = sendchar; 
}

/*******************************************
�������ƣ�R0SendChar1
��    �ܣ���PC������һ���ַ�
��    ����sendchar--Ҫ���͵��ַ�
����ֵ  ����
********************************************/
void R0SendChar1(char sendchar)
{
  while (!(IFG2&UCA0TXIFG));    //�ȴ����ͼĴ���Ϊ��         
  UCA0TXBUF = sendchar; 
}

/*******************************************
�������ƣ�R0PutString
��    �ܣ���PC�������ַ������޻���
��    ����ptr--ָ�����ַ�����ָ��
����ֵ  ����
********************************************/
void R0PutString(unsigned char *ptr)
{
  while(*ptr != '\0')
  {
    R0SendChar(*ptr++);                     // ��������
  }
}

/*******************************************
�������ƣ�R0PutString1
��    �ܣ���PC�������ַ������޻���
��    ����ptr--ָ�����ַ�����ָ��
����ֵ  ����
********************************************/
void R0PutString1(char *ptr)
{
  while(*ptr != '\0')
  {
    R0SendChar1(*ptr++);                     // ��������
  }
}
/*******************************************
�������ƣ�R0ReceiveChar
��    �ܣ�����PC�����������ַ�
��    ������
����ֵ  �����������ַ�
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


