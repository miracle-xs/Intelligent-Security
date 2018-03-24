/*
               
                   MSP430Fȫϵ��Flash�洢��ͨ�ó����


  ˵�����ó����������ڲ�Flash�洢�����õĶ�д����,�������˳�������
        ��ʽ�����ı��桢��ȡ�����������ֽڶ�д֮�⣬�����Ժܷ������
        FlashROM��д��Ͷ������͡��������������͸�ʽ�����ݡ�
            ����Flash�洢�����ܵ��ֽڲ������д�����д���κ��ֽڶ���
        Ҫ�ȱ������������������������֮��ָ�����δ�ı����ݡ���������
        ������Flash�洢��(InfoA��InfoB)���౸����Ϣ���ŵ��Ǳ��ݹ��̲�
        ռ��RAM������RAMС��256�ֽڵĵ�Ƭ��������;ȱ�����ٶ��������˷�
        ��1/2�Ĵ洢�������ʺ�С���������Ҳ�����д���ٶȵĳ��ϣ���˵�
        �������ò����ȡ�����ע��Flash����ֻ��10������ң���ҪƵ������
        д������дFlashʱ����Դ��ѹ�������2.7V��
            ������������еĶ�д���������Ѿ�������Flash�������洢��ַ
        �ռ��0��ʼ��127�����ʵ���õ������������٣����޸�MAXNUM���壬
        �Խ�ʡִ��ʱ�䡣�����Ҫ���������������128�����޸Ĵ洢���׵�ַ
        �ͱ������׵�ַ�Ķ��壬ָ�����Flash���г��������ж�������δ
        ռ�õĿհ����飬�ܻ��512�ֽڵĴ洢������
            Ҫʹ�øÿ⺯������Ҫ�����ļ�(Flash.c)��ӽ����̣�������Ҫ
        ���ú������ļ���ͷ������"Flash.h"��

  (C)�������ӿƼ���ѧ ��ؼ����������������� ��д��л�� 2008/02/04
  
*/
//
//
//                  MSP430F4XX
//               +---------------+          
//               |               |
//               |           XOUT|-----
//               |               |      32.768KHz Watch Crystal
//               |            XIN|-----
//               |               |
//               +---------------+

#include  "msp430f2132.h"
#include "Flash.h"
#define   MAXNUM  32      /*�洢����������(�ֽ�),���128*/
/*����ÿ�����ݶ���Ҫ���ݣ���������ԽС�ٶ�Խ�죬��Ҫ��̫������*/
/*����洢����ȡ�������õĴ洢��ַ�ռ�Ϊ0~(MAXNUM-1)*/

#define   FLASH_SAVEADDR  (0x1080) /*Flash���ݴ洢���׵�ַ(InfoB)*/
#define   FLASH_COPYADDR  (0x1000) /*Flash���ݴ洢���׵�ַ(InfoA)*/
#define   SegmentStart 0x01000
#define   SegmentSize 255
union LongChar      //�����ֽ�  ���Ͻṹ
{ unsigned long int Long;
  struct ByteL4
   { unsigned char  BHH;
     unsigned char  BHL;
     unsigned char  BLH;
     unsigned char  BLL;
   }Bytes;   
};
union FloatChar      //�����ֽ�  ���Ͻṹ
{ float Float;
  struct ByteF4
   { unsigned char  BHH;
     unsigned char  BHL;
     unsigned char  BLH;
     unsigned char  BLL;
   }Bytes;   
};

unsigned char FLASH_PSR;
//#define DISABLE_INT;  FLASH_PSR=__get_SR_register();_DINT();
//#define RESTORE_INT;  if(FLASH_PSR & GIE)           _EINT();
#define DISABLE_INT _DINT();
#define RESTORE_INT _EINT();
/****************************************************************************
* ��    �ƣ�Flash_Init()
* ��    �ܣ���Flashʱ�ӽ��г�ʼ������
* ��ڲ������� 
* ���ڲ�������
* ˵    ��: ����ʹ�ú���Ķ�д�������ڳ���ʼ�����ȵ��øó�ʼ������
            ����Flashʱ�ӱ�����257~476KHz֮��!!
****************************************************************************/
void Flash_Init(void)
{
  FCTL2 = FWKEY + FSSEL_2 + FN1; // Ĭ�� SMCLK/3 =349KHz 
}
  /* Flashʱ�ӱ�����257~476KHz֮��, */
  /* ��Ƶϵ��= 32*FN5 + 16*FN4 + 8*FN3 + 4*FN2 + 2*FN1 + FN0 + 1 */

  // Flashʱ��ѡ��:   
  /* FSSEL_0 Flash clock select: 0 - ACLK */
  /* FSSEL_1 Flash clock select: 1 - MCLK */
  /* FSSEL_2 Flash clock select: 2 - SMCLK */


/****************************************************************************
* ��    �ƣ�Flash_Busy()
* ��    �ܣ�Flashæ�ȴ�
* ��ڲ������� 
* ���ڲ�������
* ˵    ��: ��Flashæ��ȴ���ֱ��Flash�������(����)�ŷ��ء�
****************************************************************************/
void Flash_Busy()
{
  while((FCTL3 & BUSY) == BUSY){_NOP();}    //Busy
}
/****************************************************************************
* ��    �ƣ�Flash_Erase()
* ��    �ܣ�����Flash��һ�����ݿ�
* ��ڲ�����Addr:���������ݿ���׵�ַ 
* ���ڲ�������
****************************************************************************/
void Flash_Erase(unsigned int Addr)  
{ unsigned char *Flash_ptr;
  Flash_ptr=(unsigned char *)Addr;
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  DISABLE_INT;
  *Flash_ptr = 0;                          // Dummy write to erase Flash segment B
  Flash_Busy();                            //Busy
  RESTORE_INT;
  FCTL1 = FWKEY;                            // Lock
  FCTL3 = FWKEY+LOCK;                       // Set Lock bit  
}
/****************************************************************************
* ��    �ƣ�Flash_CopyA2B()
* ��    �ܣ��������������ݿ�����������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void Flash_CopyA2B()
{
  unsigned char *Flash_ptrA;                      // Segment A pointer
  unsigned char *Flash_ptrB;                      // Segment B pointer
  unsigned int i;
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;  // Initialize Flash segment A pointer
  Flash_ptrB = (unsigned char *) FLASH_COPYADDR;  // Initialize Flash segment B pointer
  Flash_Erase(FLASH_COPYADDR);
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  FCTL3 = FWKEY;                            // Clear Lock bit
  for (i=0; i<MAXNUM; i++)
  {
    DISABLE_INT;
    *Flash_ptrB++ = *Flash_ptrA++;           // copy value segment A to segment B
    Flash_Busy();                              //Busy
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}
/****************************************************************************
* ��    �ƣ�Flash_WriteChar()
* ��    �ܣ���Flash��д��һ���ֽ�(Char�ͱ���)
* ��ڲ�����Addr:������ݵĵ�ַ
            Data:��д�������
* ���ڲ�������
* ��    ����Flash_WriteChar(0,123);������123д��0��Ԫ
            Flash_WriteChar(1,a);�����ͱ���aд��1��Ԫ 
****************************************************************************/
void Flash_WriteChar (unsigned int Addr,unsigned char Data)
{
  unsigned char *Flash_ptrA;                         // Segment A pointer
  unsigned char *Flash_ptrB;                         // Segment B pointer
  unsigned int i;
  Flash_CopyA2B();                          //Flash�ڵ������ȱ�������
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Flash_ptrB = (unsigned char *) FLASH_COPYADDR;// Initialize Flash segment B pointer
  Flash_Erase(FLASH_SAVEADDR);  
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  FCTL3 = FWKEY;                            // Clear Lock bit
  for (i=0; i<MAXNUM; i++)
  {
    DISABLE_INT;
    if(i==Addr)
    {
     *Flash_ptrA++ =Data;                    // Save Data
     Flash_Busy();                              //Busy
     Flash_ptrB++;
    }
    else
    {
     *Flash_ptrA++ = *Flash_ptrB++;           // �ָ�Flash�ڵ���������
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}  
/****************************************************************************
* ��    �ƣ�Flash_ReadChar()
* ��    �ܣ���Flash�ж�ȡһ���ֽ�
* ��ڲ�����Addr:������ݵĵ�ַ
* ���ڲ��������ص�����
****************************************************************************/
unsigned char Flash_ReadChar (unsigned int Addr)
{ unsigned char Data;
  unsigned char *Flash_ptrA;                         // Segment A pointer
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;             // Initialize Flash segment A pointer
  Data=*(Flash_ptrA+Addr);
  return(Data);
}
/****************************************************************************
* ��    �ƣ�Flash_WriteWord()
* ��    �ܣ���Flashд��һ�����ͱ���
* ��ڲ�����Addr:��д��FlashROM��Ԫ�ĵ�ַ
            Data:��д������ͱ���(2�ֽ�����)
* ���ڲ�������
* ˵    ��: �ú����������ݽ�ռ��Addr��Addr+1�����洢��Ԫ
* ��    ����Flash_WriteWord(2,1234);������1233д��2~3��Ԫ
            Flash_WriteWord(4,a);�����ͱ���aд��4~5��Ԫ 
****************************************************************************/
void Flash_WriteWord (unsigned int Addr,unsigned int Data)
{
  unsigned char *Flash_ptrA;                // Segment A pointer
  unsigned char *Flash_ptrB;                // Segment B pointer
  unsigned int i;
  Flash_CopyA2B();                          //Flash�ڵ������ȱ�������
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Flash_ptrB = (unsigned char *) FLASH_COPYADDR;// Initialize Flash segment B pointer
  Flash_Erase(FLASH_SAVEADDR);
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  FCTL3 = FWKEY;                            // Clear Lock bit
  for (i=0; i<MAXNUM; i++)
  {
    DISABLE_INT;
    if(i==Addr)
    {
     *Flash_ptrA++ =Data/256;                    // Save Data
     Flash_Busy();                                //Busy
     *Flash_ptrA++ =Data%256;                    // Save Data     
     Flash_Busy();                                //Busy
     Flash_ptrB+=2;
    }
    else
    {
     *Flash_ptrA++ = *Flash_ptrB++;            // �ָ�Flash�ڵ���������
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
} 
/****************************************************************************
* ��    �ƣ�Flash_ReadWord()
* ��    �ܣ���FlashROM����һ�����ͱ���
* ��ڲ�����Addr:����������FlashROM��Ԫ����ʼ��ַ(����ĵ�ַ)
* ���ڲ��������ص����ͱ���ֵ
****************************************************************************/
unsigned int Flash_ReadWord (unsigned int Addr)
{ unsigned int Data;
  unsigned char *Flash_ptrA;                    // Segment A pointer
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Data=*(Flash_ptrA+Addr)*256+*(Flash_ptrA+Addr+1);
  return(Data);
}
/****************************************************************************
* ��    �ƣ�Flash_WriteLong()
* ��    �ܣ���FlashROMд��һ�������ͱ���
* ��ڲ�����Addr:��д��FlashROM��Ԫ�ĵ�ַ
            Data:��д��ĳ����ͱ���(4�ֽ�����)
* ���ڲ�������
* ˵    ��: �ú����������ݽ�ռ��Addr��Addr+3 ��4���洢��Ԫ
* ��    ����Flash_WriteLong(7,123456);����������123456д��7~10��Ԫ
            Flash_WriteLong(11,a);�������ͱ���aд��11~14��Ԫ 
****************************************************************************/
void Flash_WriteLong (unsigned int Addr,unsigned long int Data)
{
  union LongChar LData;
  unsigned char *Flash_ptrA;                         // Segment A pointer
  unsigned char *Flash_ptrB;                         // Segment B pointer
  unsigned int i;
  LData.Long=Data;
  Flash_CopyA2B();                          //Flash�ڵ������ȱ�������
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Flash_ptrB = (unsigned char *) FLASH_COPYADDR;// Initialize Flash segment B pointer
  Flash_Erase(FLASH_SAVEADDR);
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  FCTL3 = FWKEY;                            // Clear Lock bit
  for (i=0; i<MAXNUM; i++)
  {
    DISABLE_INT;
    if(i==Addr)
    {
     *Flash_ptrA++ =LData.Bytes.BHH;            // Save Data
     Flash_Busy();                              //Busy
     *Flash_ptrA++ =LData.Bytes.BHL;            // Save Data   
     Flash_Busy();                              //Busy
     *Flash_ptrA++ =LData.Bytes.BLH;            // Save Data
     Flash_Busy();                              //Busy
     *Flash_ptrA++ =LData.Bytes.BLL;            // Save Data     
     Flash_Busy();                              //Busy
     Flash_ptrB+=4;
     }
    else
    {
     *Flash_ptrA++ = *Flash_ptrB++;           // �ָ�Flash�ڵ���������
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;    
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
} 
/****************************************************************************
* ��    �ƣ�Flash_ReadLong()
* ��    �ܣ���FlashROM����һ�������ͱ���
* ��ڲ�����Addr:��������������FlashROM��Ԫ����ʼ��ַ(����ĵ�ַ)
* ���ڲ��������صĳ����ͱ���ֵ
****************************************************************************/
unsigned long int Flash_ReadLong (unsigned int Addr)
{
  unsigned char *Flash_ptrA;                    // Segment A pointer
  union LongChar Data;
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Flash_ptrA+=Addr;
  Data.Bytes.BHH=*Flash_ptrA++;
  Data.Bytes.BHL=*Flash_ptrA++;
  Data.Bytes.BLH=*Flash_ptrA++;
  Data.Bytes.BLL=*Flash_ptrA++;     
  return(Data.Long);
}
/****************************************************************************
* ��    �ƣ�Flash_WriteFloat()
* ��    �ܣ���FlashROMд��һ�������ͱ���
* ��ڲ�����Addr:��д��FlashROM��Ԫ�ĵ�ַ
            Data:��д��ĸ����ͱ���(4�ֽ�����)
* ���ڲ�������
* ˵    ��: �ú����������ݽ�ռ��Addr��Addr+3 ��4���洢��Ԫ
* ��    ����Flash_WriteFloat(15,3.14159);��������3.14159д��15~18��Ԫ
            Flash_WriteFloat(19,a);���������aд��19~22��Ԫ 
****************************************************************************/
void Flash_WriteFloat (unsigned int Addr,float Data)
{
  union FloatChar FData;
  unsigned char *Flash_ptrA;                // Segment A pointer
  unsigned char *Flash_ptrB;                // Segment B pointer
  unsigned int i;
  FData.Float=Data;
  Flash_CopyA2B();                          //Flash�ڵ������ȱ�������
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Flash_ptrB = (unsigned char *) FLASH_COPYADDR;// Initialize Flash segment B pointer
  Flash_Erase(FLASH_SAVEADDR);
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  FCTL3 = FWKEY;                            // Clear Lock bit
  for (i=0; i<MAXNUM; i++)
  {
    DISABLE_INT;
    if(i==Addr)
    {
     *Flash_ptrA++ =FData.Bytes.BHH;            // Save Data
     Flash_Busy();                              //Busy
     *Flash_ptrA++ =FData.Bytes.BHL;            // Save Data   
     Flash_Busy();                              //Busy
     *Flash_ptrA++ =FData.Bytes.BLH;            // Save Data
     Flash_Busy();                              //Busy
     *Flash_ptrA++ =FData.Bytes.BLL;            // Save Data     
     Flash_Busy();                              //Busy
     Flash_ptrB+=4;
    }
    else
    {
     *Flash_ptrA++ = *Flash_ptrB++;           // �ָ�Flash�ڵ���������
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
} 
/****************************************************************************
* ��    �ƣ�Flash_ReadFloat()
* ��    �ܣ���FlashROM����һ�������ͱ���
* ��ڲ�����Addr:��������������FlashROM��Ԫ����ʼ��ַ(����ĵ�ַ)
* ���ڲ��������صĸ����ͱ���ֵ
****************************************************************************/
float Flash_ReadFloat (unsigned int Addr)
{
  unsigned char *Flash_ptrA;                   // Segment A pointer
  union FloatChar Data;
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Flash_ptrA+=Addr;
  Data.Bytes.BHH=*Flash_ptrA++;
  Data.Bytes.BHL=*Flash_ptrA++;
  Data.Bytes.BLH=*Flash_ptrA++;
  Data.Bytes.BLL=*Flash_ptrA++;     
  return(Data.Float);
}
//==============================================================================================

/************************************************************************
*��ȡƬ�ڴ洢������
************************************************************************/
unsigned char ReadSegment_256(char index)
{
  char *flash_ptr=((char *)SegmentStart)+index;
  return *flash_ptr;
}

/************************************************************************
*����index:���������λ��
*����value:�����ָ��
*����size:����Ĵ�С
************************************************************************/
void WriteSegment_256(int index,unsigned char *value,int size)
{
  unsigned char *buffer;
  int i=0;
  char *flash_ptr=(char *) SegmentStart;
  
  /*
  //�ȶ�ȡԭFlash����
  for(i=0;i<SegmentSize;i++)
    buffer[i]=ReadSegment_256(i);
  */
  
  //����FLASH
  FCTL1=FWKEY+ERASE;
  FCTL3=FWKEY;
  DISABLE_INT;
  *flash_ptr=0;
  
  //���������鶼д��FLASH
  FCTL1=FWKEY+WRT;
  
/*  for(i=0;i<size;i++)//�޸�ԭ����
    buffer[i]=value[i];  */
  for(i=0;i<size;i++)//д�����ݵ�FLASH
    *(flash_ptr++)=value[i];
  
  RESTORE_INT; 
  FCTL1=FWKEY;
  FCTL3=FWKEY+LOCK;
  
}
