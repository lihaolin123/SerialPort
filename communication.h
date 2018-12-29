#ifndef _COMMUNICATION_H
#define _COMMUNICATION_H
/*设置单片机类型,可以修改为STM32或8051*/
#define MCU 51
/////STM32单片机///////////////
#if MCU==32
#include "sys.h"
///////51单片机//////////
#elif MCU==51
#include "STC15f2k.h"
#else 
#error  "MCU型号错误"
#endif
//////函数声明/////////////////////
extern void ClearFIFO(void);
extern unsigned char UARTFIFOCommand(void);
extern unsigned char UART_GetByte(void);
extern unsigned short UART_GetWord(void);
extern unsigned long int UART_Get4Bytes(void);
extern void UART_GetFloat(float *n_siTemp);
//发送
extern void UART_SendFloat(unsigned char flag,float value);
extern void UART_Send4Bytes(unsigned char flag,unsigned long int value);
extern void UART_SendWord(unsigned char flag,unsigned short value);
extern void UART_SendByte(unsigned char flag,unsigned char value);


#endif
