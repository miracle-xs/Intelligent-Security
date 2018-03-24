#include <msp430f2132.h>
#include<stdio.h>
#include<stdlib.h>
#include"uart.h"
#include "Flash.h"
#define CPU_F ((double)800000000.0)
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))
#define uchar unsigned char
#define uint unsigned int
#define temp0 (P2IN&BIT4)   //������˿�
#define LED_ON0 (P1IN&BIT1)  //����ģʽ���
#define LED_AUTO0  (P1IN&BIT2) //�Զ�ģʽ���
#define GM0  (P3IN&BIT7) //�������˿�

/*AD�ɼ���������*/
float P20_v;
unsigned int a[34],b;
unsigned int i,j;
float v;
uint P20_int;
uchar shi,ge,shi_fen,bai_fen;
unsigned char asca,ascb,ascc,ascd;


/*�����ַ�����������*/
uchar GSM_RXBUF[50];
uchar GSM_RXIFG = 0;
uchar GSM_COUNT = 0;

int m=1,f=0,t=0,n=1;    //aΪ���Ͳ�ѯMAC��ַָ��ı�־λ
uchar mac[17];  //MAC��ַ������
uchar zhanghao[15]; //�û�wifi�˺Ż�����
uchar mima[16];   //�û�wifi���뻺����  
uchar data[8];   //�����������������ݻ�����
uchar w;

/*WIFIģ�齨���������뽨��WIFI*/
  //unsigned char AT[]="AT\r\n";
  //static unsigned char CWMODE[]="AT+CWMODE=3\r\n";
  //static unsigned char CWSAP[]="AT+CWSAP=\"YD\",\"0\",11,0\r\n";
  //unsigned char CWSAP1[]="AT+CWSAP=\"YDC\",\"0\",11,0\r\n";
  //static unsigned char CIPMUX[]="AT+CIPMUX=1\r\n";
  //static unsigned char CIPAP[]="AT+CIPAP=\"192.168.4.1\"\r\n";
  //static unsigned char CIPSERVER[]="AT+CIPSERVER=1,5000\r\n";
  //unsigned char CIPSEND1[]="AT+CIPSEND=0,17\r\n";
  static unsigned char CIPSEND[]="AT+CIPSEND=17\r\n";//Ȼ��ʼ���մ������ݣ������ݳ�����17ʱ��������
  static unsigned char CIPSEND2[]="AT+CIPSEND=8\r\n";//Ȼ��ʼ���մ������ݣ������ݳ�����8ʱ��������
  /*WIFIģ������WIFI�����ӷ�����*/
  //unsigned char CWJAP[]="AT+CWJAP=\"112233\",\"1234567890\"\r\n";
  //static unsigned char CIPMUX0[]="AT+CIPMUX=0\r\n";
  //static unsigned char CIFSR[]="AT+CIFSR\r\n";
  uchar CWJAP[]="AT+CWJAP=\"\",\"\"\r\n";//��������
  uchar CWJAP1[35];
  uchar CWJAP2[45];
  uchar test[100];
  //static unsigned char CIPSERVER1[]="AT+CIPSERVER=0\r\n";
  //static unsigned char CIPSTART[]="AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n";
  static unsigned char space[]="\r\n";
  
  /*��־λ*/
  //unsigned char aaa[]="cf";   //������־λ
  //unsigned char bbb[]="bf";   //������־λ
  unsigned int ccc=0;         //������������־λ
  unsigned int ddd=0;         //WIFI���ͱ�־λ
  
  
  int len(uchar ch[]);
  void chIns(uchar ch[],uchar ch1[],int k);
  void Read_Flash(void);
  void Wifi_initial_1(void);
  void Wifi_initial_2(void);
  void Copy(uchar s1[] , uchar s2[]);
int main( void )
{
  /*�������������*/
  uchar temp;
  P2DIR &=~ BIT4;//����,������˿�
  /*���ؼ���������*/
  uchar LED_ON,LED_AUTO;
  P1DIR &=~ BIT1;//����,�����ض˿�
  P1DIR &=~ BIT2;//����,�����ض˿�
  P3DIR |=BIT4;    //TXD���
  P3DIR &=~BIT5;   //RXD����
  /*��������������*/
  uchar GM;
  P3DIR &=~ BIT7;//����,�������˿�
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT 
  
  TA0CCTL0 = CCIE;                           // TA0CCR0 interrupt enabled
  TA0CCR0 = 50000;
  TA0CTL = TASSEL_2 + MC_2;                  // SMCLK, contmode
  //__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
  
  InitUART();   //��ʼ������
  
  
  /*��ʼ��AD�ɼ�����*/
  //P2OUT = 0;
  ADC10CTL1 = INCH_0 + CONSEQ_3;         // Temp Sensor ADC10CLK/4
  ADC10CTL0 |= ADC10ON + ADC10IE + REFON + REF2_5V + SREF_1 + MSC + ADC10SHT_2;
  
  ADC10AE0 = 0X01;  // P2.0 analog enable
  //P2DIR = BIT0;
  ADC10DTC1|=0X20;  //����32��
  
  /*�ж�test���Ƿ�������*/
  for(i=0;i<100;i++)                 //��ȡFlash����,����test��
  {
    test[i]=ReadSegment_256(i);
 //   test1[i]=test[i];    
  }
  

 if(test[0]==0xFF)
  { 
    Wifi_initial_1();  
   }
  else
  {
    
   Wifi_initial_2();
  
  }
  
 
  
  /*WIFI������ʼ��*/
  
  //R0PutString(AT);
 //Wifi_initial();

  while(1)
  {  
    if(m)
    {
      m=0;
      R0PutString("AT+CIFSR\r\n"); //AT+CIFSR  ��ȡ���� IP  
    }
    
    /*AD�ɼ�*/
    ADC10CTL0 &= ~ENC;//�رղ���ʹ��
    while(ADC10CTL1 & ADC10BUSY);//�ж��Ƿ��в�����ת������
    ADC10CTL0 |= ADC10SC + ENC;//��ʼ����
    ADC10SA = (uint)a;
    __bis_SR_register(LPM0_bits+GIE);//�ر�CPU���������ж�
    
    b=0;
    for(j=0;j<32;j++)
    b += a[j];

    b=b/32;
    v=b*25;
    v=(v/1023)/10;
    
    P20_v = v;
   
    P20_v=P20_v*1000;//���õ�����*100Ϊ�˷���ȡ��С������
    P20_int=(uint)P20_v;//������λС�� ת��Ϊ����
    
    shi =P20_int/1000;
    ge=P20_int/100%10;
    shi_fen=P20_int/10%10;
    bai_fen=P20_int%10;
    
    
    /*������*/
    if(temp0)
    {
      temp = '1';   //����
    }
    else
    {
      temp = '0';   //����
    }
    
    /*�������*/
    if(GM0)
    {
      GM = '1';   //�ڰ�����
    }
    else
    {
      GM = '0';   //��������
    }
    
    /*���ؼ��*/
    if(LED_ON0)
    {
      LED_ON = '1';   //����ģʽ����
    }
    else
    {
      LED_ON = '0';
    }
    if(LED_AUTO0)
    {
      LED_AUTO = '1';   //�Զ�ģʽ����
    }
    else
    {
      LED_AUTO = '0';
    }
    
    /*�������ؼ��*/
    if(!LED_AUTO0)  //�Զ�
    {
        if(temp0&&GM0)    //�����Һڰ�ʱ����
        {
          P1DIR=0x01;      //����
          P1OUT|=0x01;
        }
        else    //���˻����ʱ�ص�
        {
          P1DIR=0x01;      //�ص�
          P1OUT&=~0x01;
        }
    }
    else if(LED_AUTO0&&LED_ON0) //�ص�
    {
        P1DIR=0x01;      //�ص�
        P1OUT&=~0x01;
        P2DIR=0X08;   
        P2OUT&=~0x08;   //�ط�����
    }
    else if(!LED_ON0) //����
    {
        P1DIR=0x01;      //����
        P1OUT|=0x01;
    }
    
    /*�������ݣ�ASC��ת��*/
    asca = shi&0x0f;
    ascb = ge&0x0f;
    ascc = shi_fen&0x0f;
    ascd = bai_fen&0x0f;
    
    if(asca<10)
	asca=asca+0x30;
    else
	asca=asca+0x37;
    if(ascb<10)
	ascb=ascb+0x30;
    else
	ascb=ascb+0x37;
    if(ascc<10)
	ascc=ascc+0x30;
    else
	ascc=ascc+0x37;
    if(ascd<10)
	ascd=ascd+0x30;
    else
        ascd=ascd+0x37;
		
    data[0]=asca;
    //data[1]='_';
    //data[1]='.';
    data[1]=ascb;
    //data[3]='_';
    data[2]=ascc;
    //data[5]='_';
    data[3]=ascd;
    data[4]='_';
    data[5]=temp;
    data[6]='_';
    
    //R0PutString(data);          
    
    //delay_ms(1000);
    //delay_ms(1000);
    //delay_ms(1000);
    
    /*����WIFI������ݷ���
    if(ddd == 100)
    {
      ddd = 0;
      delay_ms(1000);
      delay_ms(1000);
      delay_ms(1000);
      R0PutString(CIPSEND);
      delay_ms(1000);
      //delay_ms(1000);
      //delay_ms(1000);
      //R0PutString(MAS1);
      R0PutString(data);
      R0PutString(space);
      delay_ms(1000);
      delay_ms(1000);
      delay_ms(1000);
      //for(i=0;i<7;i++)
      //{delay_ms(1000);}
    }
    */
    
    /*���ռ��*/
    if(GSM_RXIFG)
    {
      int count = 0;
      int k=0,p=0;
      delay_ms(30);
      GSM_RXIFG = 0;
      for(count=0;count<GSM_COUNT-1;count++)
      {
         if(GSM_RXBUF[count]=='M'&&GSM_RXBUF[count+1]=='A'&&GSM_RXBUF[count+2]=='C')  //��ȡmac��ַ
         {
            for(i=0;i<17;i++)
            {
              mac[i] = GSM_RXBUF[count+5+i];
            }  
            R0PutString(CIPSEND);
            delay_ms(1000);
            R0PutString(mac);
            R0PutString(space);
            delay_ms(1000); 
	    break;
         }
         /*
         if(GSM_RXBUF[count]=='#'&&GSM_RXBUF[count+1]=='*') //���ֻ��˷�mac��ַ
         {
            R0PutString(CIPSEND1);
            delay_ms(1000);
            R0PutString(mac);
            R0PutString(space);
            delay_ms(1000);          
	    break;
         }
         */
         if(GSM_RXBUF[count]=='*'&&GSM_RXBUF[count+1]=='#') //����������mac��ַ
         {
            R0PutString(CIPSEND);
            delay_ms(1000);
            R0PutString(mac);
            R0PutString(space);
            delay_ms(1000);          
	    break;
         }
         if((GSM_RXBUF[count]=='#'&&GSM_RXBUF[count+1]=='$')) //�����û�wifi��Ϣ
         {
            for( int i=0;i<sizeof(zhanghao);i++ ) 
            zhanghao[i]=0;
            for( int i=0;i<sizeof(mima);i++ ) 
            mima[i]=0;
            
            while(GSM_RXBUF[count+2] != '[')
            {
              zhanghao[k] = GSM_RXBUF[count+2];
              k++;
              count++;
            }
            while(GSM_RXBUF[count+3] != ']')
            {
              mima[p] = GSM_RXBUF[count+3];
              p++;
              count++;
            }
            
            /*ƴ���ַ���*/
            for( int i=0;i<sizeof(CWJAP1);i++ ) 
            CWJAP1[i]=0;
            for( int i=0;i<sizeof(CWJAP2);i++ ) 
            CWJAP2[i]=0;
            
            i=0;
            while(CWJAP[i] != '\0')
            {
              CWJAP1[i] = CWJAP[i];
              i++;
            } 
            
           
            chIns(CWJAP1,zhanghao,10);
            
            i=0;
            while(CWJAP1[i] != '\0')
            {
              CWJAP2[i] = CWJAP1[i];
              i++;
            }
            
            chIns(CWJAP2,mima,10+k+3);
            /*
            R0PutString(space);
            delay_ms(1000);   
            delay_ms(1000);
            R0PutString(CWJAP2);
            */
            k=0;
            p=0;
            //sprintf(CWJAP,"AT+CWJAP=\"%s\",\"%s\"\r\n",zhanghao,mima);
            WriteSegment_256(0,CWJAP2,sizeof(CWJAP2));  //��CWJAPд��Flash������ԭ�ͼ�Flash.c
             //WriteSegment_256(0,str1,3);  
  
            
            
            
            break;
         }
         
         if((GSM_RXBUF[count]=='#'&&GSM_RXBUF[count+1]=='@')) //���ռ��ױ��
         {
              data[7] = GSM_RXBUF[count+2];
              w = GSM_RXBUF[count+2];
              //data[8] = GSM_RXBUF[count+3];
              f=1;
              break;
         }
         
         if((GSM_RXBUF[count]=='w'&&GSM_RXBUF[count+1]=='c')) //����wifi
         {
              delay_ms(1000);   
              delay_ms(1000);
              R0PutString("AT+CIPSERVER=0\r\n");
              delay_ms(1000);delay_ms(1000);
              //R0PutString(CWSAP1); 
              //delay_ms(1000);delay_ms(1000);
              R0PutString("AT+CIPMUX=0\r\n");
              delay_ms(1000);delay_ms(1000);
              R0PutString(CWJAP2);
              delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
              R0PutString("AT\r\n");
              delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
              delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
              R0PutString("AT+CWMODE=1\r\n");
              delay_ms(1000);delay_ms(1000);//delay_ms(1000);delay_ms(1000); 
              R0PutString("AT+CWSAP=\"YD-WCSZ\",\"0\",11,0\r\n"); //����apģʽ�²���
              delay_ms(1000);delay_ms(1000);
              R0PutString("AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n");//���� TCP ����,�����Ʒ�����
              delay_ms(1000);
              delay_ms(1000);
              delay_ms(1000);
              m = 1;
              break;
         }
          //c1��c2�Զ�����������c3��c4�ֶ�
         if((GSM_RXBUF[count]=='c'&&GSM_RXBUF[count+1]=='1')||
            (GSM_RXBUF[count]=='c'&&GSM_RXBUF[count+1]=='2')||
            (GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='3')||
            (GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='4') )     
         {
            if((GSM_RXBUF[count]=='c') && (GSM_RXBUF[count+1]=='1')) //�Զ�����
            {
              //P2OUT=0X00;
              ccc=0;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('a');R0SendChar('a');R0SendChar('a');R0SendChar('a');
              //R0SendChar('a');R0SendChar('a');R0SendChar('a');
            }
            if((GSM_RXBUF[count]=='c') && (GSM_RXBUF[count+1]=='2')) //�Զ�����
            {
              ccc = 1;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');R0SendChar('b');
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');
            }
            
            if(GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='3') //�ֶ�����
            {
              ccc = 1;
              //ccc = 1;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');R0SendChar('b');
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');
            }
            if(GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='4') //�ֶ�����
            {
              ccc = 0;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');R0SendChar('b');
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');
            }
            
            //n=1;
	    break;
         }
      }
      GSM_COUNT=0;
    }
     /*���������ж��󽫷�������*/
                 if(ccc == 0)    //����״̬
                  {
                    P2DIR=0x08;       //�ط�����
                    P2OUT&=~0x08;
                  }
                  else if(ccc == 1)   //����״̬
                  {
                        if(temp0) //����
                        {
                          //temp = '1';
                          P2DIR=0X08;   //��������
                          P2OUT|=0x08;
                        }
                        else if(!temp0)  //����
                        {
                          //temp = '0';
                          P2DIR=0X08;
                          P2OUT&=~0x08;   //��������
                        }
                  }
    
    
   
    //ddd++;
    /*�������ͼ������*/
    if(t==6000) //5minһ��
    {
      t=0;
      R0PutString(CIPSEND2);
      delay_ms(1000);
      R0PutString(data);
      R0PutString(space); 
    }
    
    if(f)
    {
       //n=0; 
       if((temp0) && n ) //����
       {
          R0PutString(CIPSEND2);
          delay_ms(1000);
          R0PutString(data);
          R0PutString(space);
          n=0;
       }
       else if(!temp0)
       {
        n=1;
       }
    }
  }
}

/*�ַ������ȼ��㺯��*/
int len(uchar ch[])
{
    int i=0;
    while (ch[i]!='\0'){                 
          i++;                   
    }            
    return i;
}

/*�ַ��������ַ�������*/
void chIns(uchar ch[],uchar ch1[],int k)
{
     int i;
     int len_ch=len(ch);
     int len_ch1=len(ch1);

         for (i=len_ch+len_ch1-1;i>=k+len_ch1;i--){
             ch[i]=ch[i-len_ch1];       /*�ƶ��ַ���Ҫ�����λ�ÿճ���*/
         }
         for (i=0;i<len_ch1;i++){
             ch[k+i]=ch1[i];     /*�����ַ���*/                       
         }
         ch[len_ch+len_ch1]='\0';  /*�����ַ�������*/
         //return 1;
    
     
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
        LPM0_EXIT;//�˳�CPU�ж�
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    GSM_RXIFG = 1;
    GSM_RXBUF[GSM_COUNT] = UCA0RXBUF;
    GSM_COUNT++;
    
    if(GSM_COUNT >= 45)
    {
      GSM_COUNT=0;    
    }
   //GSM_RXIFG = 0; 
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR     //��ʱ����ʱ��������
__interrupt void Timer_A0 (void)
{
  //P1OUT ^= 0x01;                            // Toggle P1.0
  TA0CCR0 += 50000;                          // Add Offset to TA0CCR0
  t++;
  /*
  if(t==20) //1s�ı�һ��
    {
      t=0;
      P1OUT ^= 0x01; 
    }
  */
  /*
  if(t==6000) //5minһ��
    {
      t=0;
      R0PutString(CIPSEND2);
      delay_ms(1000);
      R0PutString(data);
      R0PutString(space); 
    }
  */
}
//void Read_Flash(uchar)
//{
// int i=0,size=0;
//  uchar test[100],test1[45];
// for(i=0;i<255;i++)  
//    _NOP();  
//  size=sizeof(CWJAP2);
//  test=(uchar *)malloc(size*sizeof(uchar));
//  for(i=0;i<100;i++)                 //��ȡFlash����,����test��
//  {
//    test[i]=ReadSegment_256(i);
 //   test1[i]=test[i];    
//  }
 
 //  free(test);  
//}
 /*WIFI������ʼ��*/
  
 //R0PutString(AT);
void Wifi_initial_1(void)
{
  delay_ms(1000);
  R0PutString("AT+RESTORE\r\n");//�ָ��������� 
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CWMODE=3\r\n");//ap+stationģʽ 
  delay_ms(1000);
  R0PutString("AT+CWSAP=\"YD\",\"12345678\",11,3\r\n"); //����ap���� 
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CIPMUX=1\r\n");//����������0Ϊ��·���ӣ�1Ϊ��·����ģʽ
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CIPAP=\"192.168.4.1\"\r\n"); //����ESP8266 AP �ӿڵ�IP��ַ
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CIPSERVER=1,5000\r\n");//AT+CIPSERVER����Ϊ TCP ������
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000); 

   }
void Wifi_initial_2(void)
{
  /*  delay_ms(1000);
    R0PutString("AT+RESTORE\r\n");//�ָ��������� 
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
    R0PutString("AT+CWMODE=3\r\n");//stationģʽ 
    delay_ms(1000);
    R0PutString(test);
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
 //   R0PutString("AT+RST\r\n");
    delay_ms(1000);
    delay_ms(1000);
    R0PutString("AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n");//�����Ʒ�����
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);*/
    delay_ms(1000);   
    delay_ms(1000);
    R0PutString("AT+CIPSERVER=0\r\n");
    delay_ms(1000);delay_ms(1000);
    //R0PutString(CWSAP1); 
    //delay_ms(1000);delay_ms(1000);
    R0PutString("AT+CIPMUX=0\r\n");
    delay_ms(1000);delay_ms(1000);
    R0PutString(test);  
    delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
    R0PutString("AT\r\n");
    delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
    delay_ms(1000);delay_ms(1000);delay_ms(1000);delay_ms(1000);
    R0PutString("AT+CWMODE=3\r\n");
    delay_ms(1000);delay_ms(1000);//delay_ms(1000);delay_ms(1000); 
    R0PutString("AT+CWSAP=\"YD-WCSZ\",\"0\",11,0\r\n"); 
    delay_ms(1000);delay_ms(1000);
    R0PutString("AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n");
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
       

     
}
