#ifndef _COMMUNICATION_H
#define _COMMUNICATION_H
/*���õ�Ƭ������,�����޸�ΪSTM32��8051*/
#define MCU 51
/////STM32��Ƭ��///////////////
#if MCU==32
#include "sys.h"
///////51��Ƭ��//////////
#elif MCU==51
#include "STC15f2k.h"
#else 
#error  "MCU�ͺŴ���"
#endif
//////��������/////////////////////
extern void ClearFIFO(void);
extern unsigned char UARTFIFOCommand(void);
extern unsigned char UART_GetByte(void);
extern unsigned short UART_GetWord(void);
extern unsigned long int UART_Get4Bytes(void);
extern void UART_GetFloat(float *n_siTemp);
//����
extern void UART_SendFloat(unsigned char flag,float value);
extern void UART_Send4Bytes(unsigned char flag,unsigned long int value);
extern void UART_SendWord(unsigned char flag,unsigned short value);
extern void UART_SendByte(unsigned char flag,unsigned char value);


#endif
