//========================================================================
// �ļ���: communication.c
// ��  ��: lhl(lihaolinwork@foxmail.com)
// ��  ��: 2017/4/14
// ��  ��: ����ͨѶ�շ�Э��
// ��  ��:
//      
//========================================================================
#include "communication.h"
#define FRAMEHEAD 0x40//֡ͷ
#define FRAMETAIL 0x0D//֡β
volatile unsigned char UART_RX_Status=0;//���ձ�־
//===================================================//
//UART1�жϷ������
//===================================================//

#define UART_RX_BUFFER_SIZE		32				//���ݻ������Ĵ�С
unsigned char UART_RX_Buffer[UART_RX_BUFFER_SIZE];//һ�����ε����ݻ�����
unsigned char UART_Buffer_Top=0;
unsigned char UART_Buffer_Bom=0;
#if MCU==32
void USART1_IRQHandler(void)                	//����1�жϷ������
{	
	unsigned char recive;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�
	{	
		recive =USART_ReceiveData(USART1);//(USART1->DR);	
		UART_RX_Buffer[UART_Buffer_Bom++] = recive;			
		UART_Buffer_Bom = UART_Buffer_Bom&(UART_RX_BUFFER_SIZE-1);
	} 
} 
#else
void UART1_Interrup(void) interrupt 4
{
	unsigned char recive;
	if(RI)
	{
		RI=0;						
		recive = SBUF;
		UART_RX_Buffer[UART_Buffer_Bom++] = recive;					
		UART_Buffer_Bom = UART_Buffer_Bom&(UART_RX_BUFFER_SIZE-1);
	}
}
#endif

//��ջ�����
void ClearFIFO(void)
{
	unsigned char i=UART_RX_BUFFER_SIZE;
	for(i=0;i<UART_RX_BUFFER_SIZE;i++)
	{
		UART_RX_Buffer[i]=0;
	}

}
//===================================================//
//UART���մ������
//===================================================//
/**
*����:�����жϵ�ǰUART���յ������ݳ�����,�Ƿ�����Ч�����ݰ�
	  ע��,�жϹ������ݽ��ᶪ����,����!����:��������if���,��switch()����
*����:��
*����:����Ч���ݰ�,����0,���򷵻���Ч���ݰ��е�ָ���ʶ	
**/
unsigned short m_PackedDataNumber=0;	
unsigned char UARTFIFOCommand(void)
{
	unsigned short usTemp;
	unsigned char n_PackedDataNumber;  
	unsigned char n_PackedDataCheck;
	unsigned short n_usTemp;
	unsigned short n_Buffer_Top;

	usTemp = UART_Buffer_Bom-UART_Buffer_Top;
	usTemp = usTemp&(UART_RX_BUFFER_SIZE-1); 
	if(usTemp==0) return 0;	
	if(UART_RX_Buffer[UART_Buffer_Top]==FRAMEHEAD)
	{
		if(usTemp<4) return 0;	//���ݰ�������Ҫ��
		n_Buffer_Top = UART_Buffer_Top+1;	
		n_Buffer_Top = n_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
		n_PackedDataNumber = UART_RX_Buffer[n_Buffer_Top++];
		n_Buffer_Top = n_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
		if((n_PackedDataNumber+4)>usTemp) //���ݰ�������Ҫ��
		{
			if(m_PackedDataNumber) 
			{
				m_PackedDataNumber--; 
				UART_Buffer_Top++;
				UART_Buffer_Top = UART_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
			}
			return 0;
		}
		if(n_PackedDataNumber>(UART_RX_BUFFER_SIZE-4))
		{										
			if(m_PackedDataNumber)
				m_PackedDataNumber--;
			UART_Buffer_Top++;
			UART_Buffer_Top = UART_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
			return 0;
		}
		n_usTemp = n_Buffer_Top+n_PackedDataNumber+1;
		n_usTemp = n_usTemp&(UART_RX_BUFFER_SIZE-1);
		if(UART_RX_Buffer[n_usTemp]==FRAMETAIL)//������ݻ��������Ƿ�����ȷ��λ���������ݰ���֡β��ʶ
		{
			m_PackedDataNumber = n_PackedDataNumber+1;
			n_PackedDataCheck = 0;
			n_usTemp = n_Buffer_Top;  
			while(n_PackedDataNumber--)//������ݰ�����Ч�����ۼӺ�ֵ
			{
				n_PackedDataCheck += UART_RX_Buffer[n_usTemp++];
				n_usTemp = n_usTemp&(UART_RX_BUFFER_SIZE-1);
			}
			if(n_PackedDataCheck==UART_RX_Buffer[n_usTemp])
			{
				UART_Buffer_Top += 2;	
				UART_Buffer_Top = UART_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
				n_PackedDataCheck = UART_RX_Buffer[UART_Buffer_Top++];
				UART_Buffer_Top = UART_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
				return n_PackedDataCheck;//�������ݰ��е�ָ���ʶ����
			}
		}	
		else	  	
		{
			if(m_PackedDataNumber)
				m_PackedDataNumber--;
			UART_Buffer_Top++;
			UART_Buffer_Top = UART_Buffer_Top&(UART_RX_BUFFER_SIZE-1);
			return 0;
		}				
	}
	if(m_PackedDataNumber) 
		m_PackedDataNumber--;
	UART_Buffer_Top++;
	UART_Buffer_Top = UART_Buffer_Top&(UART_RX_BUFFER_SIZE-1);	  
	return 0;		
}
/**
*����:��UART�������ݻ���������ȡһ���ֽ�(8λ)����
*����:��
*����:һ���ֽڵ�����
**/
unsigned char UART_GetByte(void)
{
	unsigned char ucTemp;
	if(m_PackedDataNumber) 
		m_PackedDataNumber--;
	ucTemp = UART_RX_Buffer[UART_Buffer_Top++];	  //++��ָ�� У���
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	return ucTemp;
}
/**
*����:��UART�������ݻ���������ȡһ�� �� (16λ)������
*����:��
*����:һ���ֵ�����
**/
unsigned short UART_GetWord(void)
{
	unsigned short usTemp;
	if(m_PackedDataNumber>=2) m_PackedDataNumber-=2;
	else m_PackedDataNumber = 0;
	usTemp = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	usTemp = usTemp<<8;
	usTemp += UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);	
	return usTemp;
}
/**
����:��UART�������ݻ���������ȡ�޷���4���ֽ�����
����:��
����:�޷���4���ֽڴ�С������
**/
unsigned long int UART_Get4Bytes(void)
{
	unsigned long int n_siTemp;
	if(m_PackedDataNumber>=4) m_PackedDataNumber-=4;
	else m_PackedDataNumber = 0;
	n_siTemp = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	n_siTemp = n_siTemp<<8;
	n_siTemp |= UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	n_siTemp = n_siTemp<<8;
	n_siTemp |= UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	n_siTemp = n_siTemp<<8;
	n_siTemp |= UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	return n_siTemp;
}
/**
*����:��UART�������ݻ���������ȡһ���з�float(32λ)������
*����:n_siTemp,�����ȡ��float ֵ�ĵ�ַ
*����:��
**/
void UART_GetFloat(float *n_siTemp)
{
	//static float n_siTemp;
	unsigned char *p;
	p = (unsigned char *)n_siTemp;
	if(m_PackedDataNumber>=4) m_PackedDataNumber-=4;
	else m_PackedDataNumber = 0;
	#if MCU==32
	p[3] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	p[2] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	p[1] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	p[0] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	#else
	p[0] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	p[1] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	p[2] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	p[3] = UART_RX_Buffer[UART_Buffer_Top++];
	UART_Buffer_Top &= (UART_RX_BUFFER_SIZE-1);
	#endif
//	return n_siTemp;
}
//===================================================//
//UART������س���
//===================================================//
volatile unsigned char UART_TX_Status=0;//���ͱ�־
union
{
    char a[4];
    float b;
}tempf;
unsigned char tempArray[4];

static void FloatToBinary(float valuef)
{
     tempf.b   = valuef;      
	#if MCU==32 
     tempArray[0] = tempf.a[0];        
     tempArray[1] = tempf.a[1];
     tempArray[2] = tempf.a[2];
     tempArray[3] = tempf.a[3];
	#else
	 tempArray[0] = tempf.a[3];        
     tempArray[1] = tempf.a[2];
     tempArray[2] = tempf.a[1];
     tempArray[3] = tempf.a[0];
	#endif
}
static void UART_SendBuf(unsigned char *p ,unsigned char len)
{
	unsigned char i=0;
	for(i=0;i<len+5;i++)
	{
		#if MCU==32
		USART1->DR =p[i];
		while(!(USART1->SR &(1<<6)));
		(USART1->SR)&=(~(0X0001<<6));	
		#else
		SBUF=p[i];
		while(!TI);
		TI=0;
		#endif
	}
}
/**
*����:UART����float������
*����:flag:��־��
	  valuef:��Χ3.40E+38 ~ +3.40E+38
*����:��
**/
void UART_SendFloat(unsigned char flag,float value)
{
	unsigned char UART_TX_Buffer[9];
	while(UART_TX_Status==1);
	UART_TX_Status = 1;
	UART_TX_Buffer[0]=FRAMEHEAD;
	UART_TX_Buffer[1]=0x05;
	UART_TX_Buffer[2]=flag;
	FloatToBinary(value);
	UART_TX_Buffer[3]=tempArray[0];
	UART_TX_Buffer[4]=tempArray[1];
	UART_TX_Buffer[5]=tempArray[2];
	UART_TX_Buffer[6]=tempArray[3];
	UART_TX_Buffer[7]=UART_TX_Buffer[2]+UART_TX_Buffer[3]+UART_TX_Buffer[4]+UART_TX_Buffer[5]+UART_TX_Buffer[6];
	UART_TX_Buffer[8]=FRAMETAIL;
	UART_SendBuf(UART_TX_Buffer,4);
	UART_TX_Status =0;
}
/**
*����:UART����4���ֽ�����
*����:flag:��־��
	  value:��Χ0~2^64 ��0x00000000--0xffffffff
*����:��
**/
void UART_Send4Bytes(unsigned char flag,unsigned long int value)
{
	unsigned char UART_TX_Buffer[9];
	while(UART_TX_Status==1);
	UART_TX_Status = 1;
	UART_TX_Buffer[0]=FRAMEHEAD;
	UART_TX_Buffer[1]=0x05;
	UART_TX_Buffer[2]=flag;
	UART_TX_Buffer[3]=(unsigned char)(value>>24);
	UART_TX_Buffer[4]=(unsigned char)(value>>16);
	UART_TX_Buffer[5]=(unsigned char)(value>>8);
	UART_TX_Buffer[6]=(unsigned char)(value);
	UART_TX_Buffer[7]=UART_TX_Buffer[2]+UART_TX_Buffer[3]+UART_TX_Buffer[4]+UART_TX_Buffer[5]+UART_TX_Buffer[6];
	UART_TX_Buffer[8]=FRAMETAIL;
	UART_SendBuf(UART_TX_Buffer,4);
	UART_TX_Status =0;
}

/**
*����:UART����һ����(2�ֽ�)
*����:flag:��־��
	  value:��Χ0~65536
*����:��
**/
void UART_SendWord(unsigned char flag,unsigned short value)
{
	unsigned char UART_TX_Buffer[7];
	while(UART_TX_Status==1);
	UART_TX_Status = 1;
	UART_TX_Buffer[0]=FRAMEHEAD;
	UART_TX_Buffer[1]=0x03;
	UART_TX_Buffer[2]=flag;
	UART_TX_Buffer[3]=(unsigned char)(value>>8);
	UART_TX_Buffer[4]=(unsigned char)value;
	UART_TX_Buffer[5]=UART_TX_Buffer[2]+UART_TX_Buffer[3]+UART_TX_Buffer[4];
	UART_TX_Buffer[6]=FRAMETAIL;
	UART_SendBuf(UART_TX_Buffer,2);
	UART_TX_Status = 0;
}
/**
*����:UART����һ���ֽ�����
*����:flag:��־��
	  value:��Χ0~256
*����:��
**/
void UART_SendByte(unsigned char flag,unsigned char value)
{
	unsigned char UART_TX_Buffer[6];
	while(UART_TX_Status==1);
	UART_TX_Status = 1;
	UART_TX_Buffer[0]=FRAMEHEAD;
	UART_TX_Buffer[1]=0x02;
	UART_TX_Buffer[2]=flag;
	UART_TX_Buffer[3]=value;
	UART_TX_Buffer[4]=UART_TX_Buffer[2]+UART_TX_Buffer[3];
	UART_TX_Buffer[5]=FRAMETAIL;
	UART_SendBuf(UART_TX_Buffer,1);
	UART_TX_Status =0;
}
