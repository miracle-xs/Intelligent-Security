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
#define temp0 (P2IN&BIT4)   //红外检测端口
#define LED_ON0 (P1IN&BIT1)  //开灯模式检测
#define LED_AUTO0  (P1IN&BIT2) //自动模式检测
#define GM0  (P3IN&BIT7) //光敏检测端口

/*AD采集变量定义*/
float P20_v;
unsigned int a[34],b;
unsigned int i,j;
float v;
uint P20_int;
uchar shi,ge,shi_fen,bai_fen;
unsigned char asca,ascb,ascc,ascd;


/*接收字符检测变量定义*/
uchar GSM_RXBUF[50];
uchar GSM_RXIFG = 0;
uchar GSM_COUNT = 0;

int m=1,f=0,t=0,n=1;    //a为发送查询MAC地址指令的标志位
uchar mac[17];  //MAC地址缓存区
uchar zhanghao[15]; //用户wifi账号缓存区
uchar mima[16];   //用户wifi密码缓存区  
uchar data[8];   //电量及有人无人数据缓存区
uchar w;

/*WIFI模块建立服务器与建立WIFI*/
  //unsigned char AT[]="AT\r\n";
  //static unsigned char CWMODE[]="AT+CWMODE=3\r\n";
  //static unsigned char CWSAP[]="AT+CWSAP=\"YD\",\"0\",11,0\r\n";
  //unsigned char CWSAP1[]="AT+CWSAP=\"YDC\",\"0\",11,0\r\n";
  //static unsigned char CIPMUX[]="AT+CIPMUX=1\r\n";
  //static unsigned char CIPAP[]="AT+CIPAP=\"192.168.4.1\"\r\n";
  //static unsigned char CIPSERVER[]="AT+CIPSERVER=1,5000\r\n";
  //unsigned char CIPSEND1[]="AT+CIPSEND=0,17\r\n";
  static unsigned char CIPSEND[]="AT+CIPSEND=17\r\n";//然后开始接收串口数据，当数据长度满17时发送数据
  static unsigned char CIPSEND2[]="AT+CIPSEND=8\r\n";//然后开始接收串口数据，当数据长度满8时发送数据
  /*WIFI模块连接WIFI与连接服务器*/
  //unsigned char CWJAP[]="AT+CWJAP=\"112233\",\"1234567890\"\r\n";
  //static unsigned char CIPMUX0[]="AT+CIPMUX=0\r\n";
  //static unsigned char CIFSR[]="AT+CIFSR\r\n";
  uchar CWJAP[]="AT+CWJAP=\"\",\"\"\r\n";//加入接入点
  uchar CWJAP1[35];
  uchar CWJAP2[45];
  uchar test[100];
  //static unsigned char CIPSERVER1[]="AT+CIPSERVER=0\r\n";
  //static unsigned char CIPSTART[]="AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n";
  static unsigned char space[]="\r\n";
  
  /*标志位*/
  //unsigned char aaa[]="cf";   //撤防标志位
  //unsigned char bbb[]="bf";   //布防标志位
  unsigned int ccc=0;         //蜂鸣器启动标志位
  unsigned int ddd=0;         //WIFI发送标志位
  
  
  int len(uchar ch[]);
  void chIns(uchar ch[],uchar ch1[],int k);
  void Read_Flash(void);
  void Wifi_initial_1(void);
  void Wifi_initial_2(void);
  void Copy(uchar s1[] , uchar s2[]);
int main( void )
{
  /*红外检测变量定义*/
  uchar temp;
  P2DIR &=~ BIT4;//输入,读红外端口
  /*开关检测变量定义*/
  uchar LED_ON,LED_AUTO;
  P1DIR &=~ BIT1;//输入,读开关端口
  P1DIR &=~ BIT2;//输入,读开关端口
  P3DIR |=BIT4;    //TXD输出
  P3DIR &=~BIT5;   //RXD输入
  /*光敏检测变量定义*/
  uchar GM;
  P3DIR &=~ BIT7;//输入,读光敏端口
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT 
  
  TA0CCTL0 = CCIE;                           // TA0CCR0 interrupt enabled
  TA0CCR0 = 50000;
  TA0CTL = TASSEL_2 + MC_2;                  // SMCLK, contmode
  //__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
  
  InitUART();   //初始化串口
  
  
  /*初始化AD采集参数*/
  //P2OUT = 0;
  ADC10CTL1 = INCH_0 + CONSEQ_3;         // Temp Sensor ADC10CLK/4
  ADC10CTL0 |= ADC10ON + ADC10IE + REFON + REF2_5V + SREF_1 + MSC + ADC10SHT_2;
  
  ADC10AE0 = 0X01;  // P2.0 analog enable
  //P2DIR = BIT0;
  ADC10DTC1|=0X20;  //采样32次
  
  /*判断test中是否有数据*/
  for(i=0;i<100;i++)                 //读取Flash数据,存入test中
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
  
 
  
  /*WIFI建立初始化*/
  
  //R0PutString(AT);
 //Wifi_initial();

  while(1)
  {  
    if(m)
    {
      m=0;
      R0PutString("AT+CIFSR\r\n"); //AT+CIFSR  获取本地 IP  
    }
    
    /*AD采集*/
    ADC10CTL0 &= ~ENC;//关闭采样使能
    while(ADC10CTL1 & ADC10BUSY);//判断是否有采样或转换动作
    ADC10CTL0 |= ADC10SC + ENC;//开始采样
    ADC10SA = (uint)a;
    __bis_SR_register(LPM0_bits+GIE);//关闭CPU，开启总中断
    
    b=0;
    for(j=0;j<32;j++)
    b += a[j];

    b=b/32;
    v=b*25;
    v=(v/1023)/10;
    
    P20_v = v;
   
    P20_v=P20_v*1000;//将得到的数*100为了方便取出小数部分
    P20_int=(uint)P20_v;//保留两位小数 转化为整型
    
    shi =P20_int/1000;
    ge=P20_int/100%10;
    shi_fen=P20_int/10%10;
    bai_fen=P20_int%10;
    
    
    /*红外检测*/
    if(temp0)
    {
      temp = '1';   //有人
    }
    else
    {
      temp = '0';   //无人
    }
    
    /*光敏检测*/
    if(GM0)
    {
      GM = '1';   //黑暗条件
    }
    else
    {
      GM = '0';   //光照条件
    }
    
    /*开关检测*/
    if(LED_ON0)
    {
      LED_ON = '1';   //开灯模式按下
    }
    else
    {
      LED_ON = '0';
    }
    if(LED_AUTO0)
    {
      LED_AUTO = '1';   //自动模式按下
    }
    else
    {
      LED_AUTO = '0';
    }
    
    /*三挡开关检测*/
    if(!LED_AUTO0)  //自动
    {
        if(temp0&&GM0)    //有人且黑暗时开灯
        {
          P1DIR=0x01;      //开灯
          P1OUT|=0x01;
        }
        else    //无人或光照时关灯
        {
          P1DIR=0x01;      //关灯
          P1OUT&=~0x01;
        }
    }
    else if(LED_AUTO0&&LED_ON0) //关灯
    {
        P1DIR=0x01;      //关灯
        P1OUT&=~0x01;
        P2DIR=0X08;   
        P2OUT&=~0x08;   //关蜂鸣器
    }
    else if(!LED_ON0) //开灯
    {
        P1DIR=0x01;      //开灯
        P1OUT|=0x01;
    }
    
    /*处理数据，ASC码转换*/
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
    
    /*连接WIFI后的数据发送
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
    
    /*接收检测*/
    if(GSM_RXIFG)
    {
      int count = 0;
      int k=0,p=0;
      delay_ms(30);
      GSM_RXIFG = 0;
      for(count=0;count<GSM_COUNT-1;count++)
      {
         if(GSM_RXBUF[count]=='M'&&GSM_RXBUF[count+1]=='A'&&GSM_RXBUF[count+2]=='C')  //获取mac地址
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
         if(GSM_RXBUF[count]=='#'&&GSM_RXBUF[count+1]=='*') //给手机端发mac地址
         {
            R0PutString(CIPSEND1);
            delay_ms(1000);
            R0PutString(mac);
            R0PutString(space);
            delay_ms(1000);          
	    break;
         }
         */
         if(GSM_RXBUF[count]=='*'&&GSM_RXBUF[count+1]=='#') //给服务器发mac地址
         {
            R0PutString(CIPSEND);
            delay_ms(1000);
            R0PutString(mac);
            R0PutString(space);
            delay_ms(1000);          
	    break;
         }
         if((GSM_RXBUF[count]=='#'&&GSM_RXBUF[count+1]=='$')) //接收用户wifi信息
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
            
            /*拼接字符串*/
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
            WriteSegment_256(0,CWJAP2,sizeof(CWJAP2));  //将CWJAP写入Flash，函数原型见Flash.c
             //WriteSegment_256(0,str1,3);  
  
            
            
            
            break;
         }
         
         if((GSM_RXBUF[count]=='#'&&GSM_RXBUF[count+1]=='@')) //接收简易编号
         {
              data[7] = GSM_RXBUF[count+2];
              w = GSM_RXBUF[count+2];
              //data[8] = GSM_RXBUF[count+3];
              f=1;
              break;
         }
         
         if((GSM_RXBUF[count]=='w'&&GSM_RXBUF[count+1]=='c')) //连接wifi
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
              R0PutString("AT+CWSAP=\"YD-WCSZ\",\"0\",11,0\r\n"); //设置ap模式下参数
              delay_ms(1000);delay_ms(1000);
              R0PutString("AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n");//建立 TCP 连接,连接云服务器
              delay_ms(1000);
              delay_ms(1000);
              delay_ms(1000);
              m = 1;
              break;
         }
          //c1、c2自动布防撤防，c3、c4手动
         if((GSM_RXBUF[count]=='c'&&GSM_RXBUF[count+1]=='1')||
            (GSM_RXBUF[count]=='c'&&GSM_RXBUF[count+1]=='2')||
            (GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='3')||
            (GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='4') )     
         {
            if((GSM_RXBUF[count]=='c') && (GSM_RXBUF[count+1]=='1')) //自动撤防
            {
              //P2OUT=0X00;
              ccc=0;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('a');R0SendChar('a');R0SendChar('a');R0SendChar('a');
              //R0SendChar('a');R0SendChar('a');R0SendChar('a');
            }
            if((GSM_RXBUF[count]=='c') && (GSM_RXBUF[count+1]=='2')) //自动布防
            {
              ccc = 1;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');R0SendChar('b');
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');
            }
            
            if(GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='3') //手动布防
            {
              ccc = 1;
              //ccc = 1;
              //R0PutString(CIPSEND2);
              //delay_ms(1000);
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');R0SendChar('b');
              //R0SendChar('b');R0SendChar('b');R0SendChar('b');
            }
            if(GSM_RXBUF[count]==w&&GSM_RXBUF[count+1]=='c'&&GSM_RXBUF[count+2]=='4') //手动撤防
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
     /*布防撤防判定后将蜂鸣器打开*/
                 if(ccc == 0)    //撤防状态
                  {
                    P2DIR=0x08;       //关蜂鸣器
                    P2OUT&=~0x08;
                  }
                  else if(ccc == 1)   //布防状态
                  {
                        if(temp0) //有人
                        {
                          //temp = '1';
                          P2DIR=0X08;   //蜂鸣器响
                          P2OUT|=0x08;
                        }
                        else if(!temp0)  //无人
                        {
                          //temp = '0';
                          P2DIR=0X08;
                          P2OUT&=~0x08;   //蜂鸣器关
                        }
                  }
    
    
   
    //ddd++;
    /*正常发送检测数据*/
    if(t==6000) //5min一次
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
       if((temp0) && n ) //有人
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

/*字符串长度计算函数*/
int len(uchar ch[])
{
    int i=0;
    while (ch[i]!='\0'){                 
          i++;                   
    }            
    return i;
}

/*字符串插入字符串函数*/
void chIns(uchar ch[],uchar ch1[],int k)
{
     int i;
     int len_ch=len(ch);
     int len_ch1=len(ch1);

         for (i=len_ch+len_ch1-1;i>=k+len_ch1;i--){
             ch[i]=ch[i-len_ch1];       /*移动字符将要插入的位置空出来*/
         }
         for (i=0;i<len_ch1;i++){
             ch[k+i]=ch1[i];     /*插入字符串*/                       
         }
         ch[len_ch+len_ch1]='\0';  /*设置字符结束符*/
         //return 1;
    
     
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
        LPM0_EXIT;//退出CPU中断
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
#pragma vector=TIMER0_A0_VECTOR     //定时器定时发送数据
__interrupt void Timer_A0 (void)
{
  //P1OUT ^= 0x01;                            // Toggle P1.0
  TA0CCR0 += 50000;                          // Add Offset to TA0CCR0
  t++;
  /*
  if(t==20) //1s改变一次
    {
      t=0;
      P1OUT ^= 0x01; 
    }
  */
  /*
  if(t==6000) //5min一次
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
//  for(i=0;i<100;i++)                 //读取Flash数据,存入test中
//  {
//    test[i]=ReadSegment_256(i);
 //   test1[i]=test[i];    
//  }
 
 //  free(test);  
//}
 /*WIFI建立初始化*/
  
 //R0PutString(AT);
void Wifi_initial_1(void)
{
  delay_ms(1000);
  R0PutString("AT+RESTORE\r\n");//恢复出厂设置 
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CWMODE=3\r\n");//ap+station模式 
  delay_ms(1000);
  R0PutString("AT+CWSAP=\"YD\",\"12345678\",11,3\r\n"); //设置ap参数 
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CIPMUX=1\r\n");//启动多连接0为单路连接，1为多路连接模式
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CIPAP=\"192.168.4.1\"\r\n"); //设置ESP8266 AP 接口的IP地址
  delay_ms(1000);
  delay_ms(1000);
  R0PutString("AT+CIPSERVER=1,5000\r\n");//AT+CIPSERVER配置为 TCP 服务器
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000); 

   }
void Wifi_initial_2(void)
{
  /*  delay_ms(1000);
    R0PutString("AT+RESTORE\r\n");//恢复出厂设置 
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
    R0PutString("AT+CWMODE=3\r\n");//station模式 
    delay_ms(1000);
    R0PutString(test);
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
 //   R0PutString("AT+RST\r\n");
    delay_ms(1000);
    delay_ms(1000);
    R0PutString("AT+CIPSTART=\"TCP\",\"115.159.220.110\",8282\r\n");//连接云服务器
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
