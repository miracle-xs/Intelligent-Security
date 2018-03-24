/*
               
                   MSP430F全系列Flash存储器通用程序库


  说明：该程序库包含了内部Flash存储器常用的读写功能,还包含了常用数据
        格式变量的保存、读取函数。除了字节读写之外，还可以很方便的向
        FlashROM里写入和读出整型、浮点数、长整型格式的数据。
            由于Flash存储器不能单字节擦除或改写，因此写入任何字节都需
        要先备份整个数据区再整块擦除，之后恢复其他未改变数据。本程序利
        用两个Flash存储区(InfoA和InfoB)互相备份信息，优点是备份过程不
        占用RAM，能在RAM小于256字节的单片机上运行;缺点是速度慢，且浪费
        了1/2的存储容量。适合小批量数据且不关心写入速度的场合，如菜单
        保存设置参数等。另外注意Flash寿命只有10万次左右，不要频繁调用
        写函数。写Flash时，电源电压必须高于2.7V。
            本程序库中所有的读写函数对外已经屏蔽了Flash特征，存储地址
        空间从0开始到127。如果实际用到的数据量很少，可修改MAXNUM定义，
        以节省执行时间。如果需要保存的数据量大于128，可修改存储区首地址
        和备份区首地址的定义，指向程序Flash区中程序代码和中断向量表未
        占用的空白区块，能获得512字节的存储容量。
            要使用该库函数，需要将本文件(Flash.c)添加进工程，并在需要
        调用函数的文件开头处包含"Flash.h"。

  (C)西安电子科技大学 测控技术与仪器教研中心 编写：谢楷 2008/02/04
  
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
#define   MAXNUM  32      /*存储的总数据量(字节),最大128*/
/*由于每个数据都需要备份，总数据量越小速度越快，不要留太多余量*/
/*后面存储、读取函数可用的存储地址空间为0~(MAXNUM-1)*/

#define   FLASH_SAVEADDR  (0x1080) /*Flash数据存储区首地址(InfoB)*/
#define   FLASH_COPYADDR  (0x1000) /*Flash备份存储区首地址(InfoA)*/
#define   SegmentStart 0x01000
#define   SegmentSize 255
union LongChar      //长整字节  复合结构
{ unsigned long int Long;
  struct ByteL4
   { unsigned char  BHH;
     unsigned char  BHL;
     unsigned char  BLH;
     unsigned char  BLL;
   }Bytes;   
};
union FloatChar      //浮点字节  复合结构
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
* 名    称：Flash_Init()
* 功    能：对Flash时钟进行初始化设置
* 入口参数：无 
* 出口参数：无
* 说    明: 如需使用后面的读写函数，在程序开始必须先调用该初始化函数
            配置Flash时钟必须在257~476KHz之间!!
****************************************************************************/
void Flash_Init(void)
{
  FCTL2 = FWKEY + FSSEL_2 + FN1; // 默认 SMCLK/3 =349KHz 
}
  /* Flash时钟必须在257~476KHz之间, */
  /* 分频系数= 32*FN5 + 16*FN4 + 8*FN3 + 4*FN2 + 2*FN1 + FN0 + 1 */

  // Flash时钟选择:   
  /* FSSEL_0 Flash clock select: 0 - ACLK */
  /* FSSEL_1 Flash clock select: 1 - MCLK */
  /* FSSEL_2 Flash clock select: 2 - SMCLK */


/****************************************************************************
* 名    称：Flash_Busy()
* 功    能：Flash忙等待
* 入口参数：无 
* 出口参数：无
* 说    明: 若Flash忙则等待，直到Flash操作完成(空闲)才返回。
****************************************************************************/
void Flash_Busy()
{
  while((FCTL3 & BUSY) == BUSY){_NOP();}    //Busy
}
/****************************************************************************
* 名    称：Flash_Erase()
* 功    能：擦除Flash的一个数据块
* 入口参数：Addr:被擦除数据块的首地址 
* 出口参数：无
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
* 名    称：Flash_CopyA2B()
* 功    能：将数据区的内容拷贝到备份区
* 入口参数：无
* 出口参数：无
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
* 名    称：Flash_WriteChar()
* 功    能：向Flash中写入一个字节(Char型变量)
* 入口参数：Addr:存放数据的地址
            Data:待写入的数据
* 出口参数：无
* 范    例：Flash_WriteChar(0,123);将常数123写入0单元
            Flash_WriteChar(1,a);将整型变量a写入1单元 
****************************************************************************/
void Flash_WriteChar (unsigned int Addr,unsigned char Data)
{
  unsigned char *Flash_ptrA;                         // Segment A pointer
  unsigned char *Flash_ptrB;                         // Segment B pointer
  unsigned int i;
  Flash_CopyA2B();                          //Flash内的数据先保存起来
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
     *Flash_ptrA++ = *Flash_ptrB++;           // 恢复Flash内的其他数据
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}  
/****************************************************************************
* 名    称：Flash_ReadChar()
* 功    能：从Flash中读取一个字节
* 入口参数：Addr:存放数据的地址
* 出口参数：读回的数据
****************************************************************************/
unsigned char Flash_ReadChar (unsigned int Addr)
{ unsigned char Data;
  unsigned char *Flash_ptrA;                         // Segment A pointer
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;             // Initialize Flash segment A pointer
  Data=*(Flash_ptrA+Addr);
  return(Data);
}
/****************************************************************************
* 名    称：Flash_WriteWord()
* 功    能：向Flash写入一个整型变量
* 入口参数：Addr:被写入FlashROM单元的地址
            Data:待写入的整型变量(2字节数据)
* 出口参数：无
* 说    明: 该函数保存数据将占用Addr和Addr+1两个存储单元
* 范    例：Flash_WriteWord(2,1234);将常数1233写入2~3单元
            Flash_WriteWord(4,a);将整型变量a写入4~5单元 
****************************************************************************/
void Flash_WriteWord (unsigned int Addr,unsigned int Data)
{
  unsigned char *Flash_ptrA;                // Segment A pointer
  unsigned char *Flash_ptrB;                // Segment B pointer
  unsigned int i;
  Flash_CopyA2B();                          //Flash内的数据先保存起来
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
     *Flash_ptrA++ = *Flash_ptrB++;            // 恢复Flash内的其他数据
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
} 
/****************************************************************************
* 名    称：Flash_ReadWord()
* 功    能：从FlashROM读回一个整型变量
* 入口参数：Addr:待读出变量FlashROM单元的起始地址(存入的地址)
* 出口参数：读回的整型变量值
****************************************************************************/
unsigned int Flash_ReadWord (unsigned int Addr)
{ unsigned int Data;
  unsigned char *Flash_ptrA;                    // Segment A pointer
  Flash_ptrA = (unsigned char *) FLASH_SAVEADDR;// Initialize Flash segment A pointer
  Data=*(Flash_ptrA+Addr)*256+*(Flash_ptrA+Addr+1);
  return(Data);
}
/****************************************************************************
* 名    称：Flash_WriteLong()
* 功    能：向FlashROM写入一个长整型变量
* 入口参数：Addr:被写入FlashROM单元的地址
            Data:待写入的长整型变量(4字节数据)
* 出口参数：无
* 说    明: 该函数保存数据将占用Addr到Addr+3 共4个存储单元
* 范    例：Flash_WriteLong(7,123456);将长整型数123456写入7~10单元
            Flash_WriteLong(11,a);将长整型变量a写入11~14单元 
****************************************************************************/
void Flash_WriteLong (unsigned int Addr,unsigned long int Data)
{
  union LongChar LData;
  unsigned char *Flash_ptrA;                         // Segment A pointer
  unsigned char *Flash_ptrB;                         // Segment B pointer
  unsigned int i;
  LData.Long=Data;
  Flash_CopyA2B();                          //Flash内的数据先保存起来
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
     *Flash_ptrA++ = *Flash_ptrB++;           // 恢复Flash内的其他数据
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;    
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
} 
/****************************************************************************
* 名    称：Flash_ReadLong()
* 功    能：从FlashROM读回一个长整型变量
* 入口参数：Addr:待读出变量所在FlashROM单元的起始地址(存入的地址)
* 出口参数：读回的长整型变量值
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
* 名    称：Flash_WriteFloat()
* 功    能：向FlashROM写入一个浮点型变量
* 入口参数：Addr:被写入FlashROM单元的地址
            Data:待写入的浮点型变量(4字节数据)
* 出口参数：无
* 说    明: 该函数保存数据将占用Addr到Addr+3 共4个存储单元
* 范    例：Flash_WriteFloat(15,3.14159);将浮点数3.14159写入15~18单元
            Flash_WriteFloat(19,a);将浮点变量a写入19~22单元 
****************************************************************************/
void Flash_WriteFloat (unsigned int Addr,float Data)
{
  union FloatChar FData;
  unsigned char *Flash_ptrA;                // Segment A pointer
  unsigned char *Flash_ptrB;                // Segment B pointer
  unsigned int i;
  FData.Float=Data;
  Flash_CopyA2B();                          //Flash内的数据先保存起来
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
     *Flash_ptrA++ = *Flash_ptrB++;           // 恢复Flash内的其他数据
     Flash_Busy();                              //Busy
    }
    RESTORE_INT;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
} 
/****************************************************************************
* 名    称：Flash_ReadFloat()
* 功    能：从FlashROM读回一个浮点型变量
* 入口参数：Addr:待读出变量所在FlashROM单元的起始地址(存入的地址)
* 出口参数：读回的浮点型变量值
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
*读取片内存储的数据
************************************************************************/
unsigned char ReadSegment_256(char index)
{
  char *flash_ptr=((char *)SegmentStart)+index;
  return *flash_ptr;
}

/************************************************************************
*参数index:保存数组的位置
*参数value:数组的指针
*参数size:数组的大小
************************************************************************/
void WriteSegment_256(int index,unsigned char *value,int size)
{
  unsigned char *buffer;
  int i=0;
  char *flash_ptr=(char *) SegmentStart;
  
  /*
  //先读取原Flash数组
  for(i=0;i<SegmentSize;i++)
    buffer[i]=ReadSegment_256(i);
  */
  
  //擦除FLASH
  FCTL1=FWKEY+ERASE;
  FCTL3=FWKEY;
  DISABLE_INT;
  *flash_ptr=0;
  
  //把整个数组都写入FLASH
  FCTL1=FWKEY+WRT;
  
/*  for(i=0;i<size;i++)//修改原数组
    buffer[i]=value[i];  */
  for(i=0;i<size;i++)//写入数据到FLASH
    *(flash_ptr++)=value[i];
  
  RESTORE_INT; 
  FCTL1=FWKEY;
  FCTL3=FWKEY+LOCK;
  
}
