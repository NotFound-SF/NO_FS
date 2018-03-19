/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_spi.h"

/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/


// ��д����ѡ���־,��дͬʱ���У�ֻ����ֻд

typedef enum {
	SPIxReadWrite = 0x00,
	SPIxReadOnly  = 0x01,
	SPIxWriteOnly = 0x02
}Read_Write_Flag;


// SPI״̬ȫ�ֱ���

typedef struct {
	OS_SEM          SemLock;                                       // ��SPI�˿ڶ�ռ�ź���                  
    OS_SEM          SemWait;                                       // SPI���ȴ��ź���                                 
	CPU_INT16S      RdBufLen;                                      // SPI��Ҫ��ȡ�����ݳ���                           
	CPU_INT16S      WrBufLen;                                      // SPI��Ҫд������ݳ���                           
	CPU_INT08U      *RdBufPtr;                                     // SPI����������ͷָ��                             
	CPU_INT08U      *WrBufPtr;                                     // SPI��д��������ͷָ��                             
	Read_Write_Flag Operation;                                     // ��д������־flag     
}BSP_SPIx_DEV_STATUS; 


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_SPIx_DEV_STATUS         BSP_SPIx_DevStatus;                // ���ڱ���SPI����״̬��Ҳ�����жϹ�ͨ����Ŧ

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static      void         BSP_NVIC_Configuration(void);
static      void         BSP_GPIO_Init(void);

/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_SPI_Init()
*
* Description : ��ʼ�� SPI���ţ�ӳ�䵽SPI���裬����PSI����Ӧ��ģʽ
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_SPIx_Init(void)
{
	OS_ERR err;
	
	SPI_InitTypeDef  SPI_InitStructure;
	
	// ����SPIʱ��
	
	BSP_SPIx_RCC_CMD(BSP_SPIx_RCC, ENABLE);
	
	// ����SPI��ص�����
	
	BSP_GPIO_Init();
	
	// ��SPI���ӵ��ж�
	
	BSP_NVIC_Configuration();
	
	// ������Ӧ���ź���
	
	OSSemCreate((OS_SEM    *)& (BSP_SPIx_DevStatus.SemLock),            // �˿������ź���
                (CPU_CHAR  *)  "SPI Lock", 
	            (OS_SEM_CTR )  1,
	            (OS_ERR    *)& err);
				
	OSSemCreate((OS_SEM    *)& (BSP_SPIx_DevStatus.SemWait),            // �˿ڵȴ��ź���
                (CPU_CHAR  *)  "SPI Wait", 
	            (OS_SEM_CTR )  0,
	            (OS_ERR    *)& err);
	
	
	// ����SPIģʽ
	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       // ��������ģʽ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                         // ʱ�Ӽ���
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                        // ʱ����λ
	SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;                          // �����ΪƬѡ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  // ��λ����
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   // ���ݿ��8bit
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  // ������ȫ˫��ģʽ
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;  // ʱ�ӷ�����
	SPI_InitStructure.SPI_CRCPolynomial = 7;                            // CRCУ��
	
	SPI_Init(BSP_SPIx_PORT, &SPI_InitStructure);                       
	BSP_SPIx_CS_DISABLE();                                              // ȷ��SPIƬѡ��ʹ��
	SPI_Cmd(BSP_SPIx_PORT, ENABLE);                                     // ʹ��SPI	
}



/*
*********************************************************************************************************
*                                        BSP_SPI_ReadWrite()
*
* Description : ��SPI���߷���BuffLen���ֽڣ����Ҷ�ȡBuffLen���ֽڵ�ReadBuff������
*
* Argument(s) : WrBuff   �����ͻ�����
*               ReadBuff �����ջ�����
*               BuffLen  : ��������ջ��峤��       
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : �ú�����ȫ˫����������дͬʱ����.
*********************************************************************************************************
*/

void BSP_SPI_ReadWrite(uint8_t *WrBuff, uint8_t *ReadBuff, uint16_t BuffLen) 
{
	OS_ERR err;
	
	// �������
	
	if (0 == BuffLen) 
		return;
	
	// ����SPI�˿�
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemLock),
	          (OS_TICK ) 0,                                                    // ���õȴ�                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // �����ȴ�                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// ��ʼ��ױ̨����
	
	BSP_SPIx_DevStatus.RdBufLen = BuffLen;                                  
	BSP_SPIx_DevStatus.RdBufPtr = ReadBuff;
	BSP_SPIx_DevStatus.WrBufPtr = WrBuff;                                   
	BSP_SPIx_DevStatus.WrBufLen = BuffLen;
	BSP_SPIx_DevStatus.Operation = SPIxReadWrite;
	
	// ������д�жϿ���˳��ܹؼ�����Ϊ����ʱTXE����λ�ģ���RXDN��0��

	BSP_SPIx_PORT->CR2 |= DEF_BIT_06;                                         // ���������ж�
	BSP_SPIx_PORT->CR2 |= DEF_BIT_07;                                         // ���������жϻ��Զ������жϿ�ʼ���͵�һ������
	
	// �ȴ���д����
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemWait),
	          (OS_TICK ) 0,                                                    // ���õȴ�                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // �����ȴ�                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// �رն�д�ж�
	
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_06;                                         // �رս����ж�
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                         // �رշ����ж�    
	
	
	// �ͷŶ˿�
	
	OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemLock),
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}



/*
*********************************************************************************************************
*                                        BSP_SPI_ReadOnly()
*
* Description : ��ȡBuffLen���ֽڵ�ReadBuff��������������SPI���߷���BuffLen����Ч�ֽ�
*
* Argument(s) : ReadBuff �����ջ�����
*               BuffLen  : ��������ջ��峤��       
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : �ú�����ȫ˫����������дͬʱ����.
*********************************************************************************************************
*/

void BSP_SPI_ReadOnly(uint8_t *ReadBuff, uint16_t BuffLen) 
{
	OS_ERR err;
	
	// �������
	
	if (0 == BuffLen) 
		return;
	
	// �����˿�
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemLock),
	          (OS_TICK ) 0,                                                    // ���õȴ�                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // �����ȴ�                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// ��ʼ��ױ̨����
	
	BSP_SPIx_DevStatus.RdBufLen = BuffLen;                                    //������ֻ��ֻд��Ҫ������BufLen����ͬ�ĳ���
	BSP_SPIx_DevStatus.WrBufLen = BuffLen;
	BSP_SPIx_DevStatus.RdBufPtr = ReadBuff;
	BSP_SPIx_DevStatus.Operation = SPIxReadOnly;
	
	// ������д�ж�
	
	BSP_SPIx_PORT->CR2 |= DEF_BIT_06;                                         // ���������ж�
	BSP_SPIx_PORT->CR2 |= DEF_BIT_07;                                         // ���������жϻ��Զ������жϿ�ʼ���͵�һ������
	
	// �ȴ���ȡ����
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemWait),
	          (OS_TICK ) 0,                                                    // ���õȴ�                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // �����ȴ�                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// �رն�д�ж�
	
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_06;                                         // �رս����ж�
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                         // �رշ����ж�  
	
	// �ͷŶ˿�
	
	OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemLock),
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}



/*
*********************************************************************************************************
*                                        BSP_SPI_WriteOnly()
*
* Description : ��SPI���߷���BuffLen���ֽ�,���Һ��Խ��յ�������
*
* Argument(s) : WriteBuff�����ջ�����
*               BuffLen  : ��������ջ��峤��       
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : �ú�����ȫ˫����������дͬʱ����.
*********************************************************************************************************
*/

void BSP_SPI_WriteOnly(uint8_t *WriteBuff, uint16_t BuffLen) 
{
	OS_ERR err;
	
	// �������
	
	if (0 == BuffLen) 
		return;
	
	// �����˿�
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemLock),
	          (OS_TICK ) 0,                                                    // ���õȴ�                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // �����ȴ�                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// ��ʼ��ױ̨����
	
	BSP_SPIx_DevStatus.RdBufLen = BuffLen;                                    //������ֻ��ֻд��Ҫ������BufLen����ͬ�ĳ���
	BSP_SPIx_DevStatus.WrBufLen = BuffLen;
	BSP_SPIx_DevStatus.WrBufPtr = WriteBuff;
	BSP_SPIx_DevStatus.Operation = SPIxWriteOnly;
	
	
	// ������д�ж�
	
	BSP_SPIx_PORT->CR2 |= DEF_BIT_06;                                         // ���������ж�
	BSP_SPIx_PORT->CR2 |= DEF_BIT_07;                                         // ���������жϻ��Զ������жϿ�ʼ���͵�һ������
	
	// �ȴ���ȡ����
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemWait),
	          (OS_TICK ) 0,                                                    // ������õȴ�                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // �����ȴ�                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	
	// �رն�д�ж�
	
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_06;                                         // �رս����ж�
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                         // �رշ����ж�  
	
	// �ͷŶ˿�
	
	OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemLock),
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}


/*
*********************************************************************************************************
*                                        BSP_SPI_WriteByte()
*
* Description : дһ���ֽ�
*
* Argument(s) : dat Ҫд�������      
*
* Return(s)   : ��ȡ��������
*
* Caller(s)   : Application
*
* Note(s)     : none 
*********************************************************************************************************
*/

uint8_t BSP_SPI_WriteByte(uint8_t dat)
{
	uint8_t readDat;
	
	BSP_SPI_ReadWrite(&dat, &readDat, 1);
	
	return readDat;
}




/*
*********************************************************************************************************
*                                        BSP_SPI_ReadByte()
*
* Description : ��ȡһ���ֽ�
*
* Argument(s) : none      
*
* Return(s)   : ��ȡ��������
*
* Caller(s)   : Application
*
* Note(s)     : none 
*********************************************************************************************************
*/

uint8_t BSP_SPI_ReadByte(void)
{	
	uint8_t res;
	
	BSP_SPI_ReadOnly(&res, 1);
	
	return res;
}



/*
*********************************************************************************************************
*                                        BSP_SPIx_IRQHandler()
*
* Description : SPI�жϴ�����
*
* Argument(s) : none      
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

#define  BSP_SPIx_RXNE_MASK       0x01                  		 // ���ջ������ǿձ�־λ����
#define  BSP_SPIx_TXE_MASK        0x02                           // ���ͻ������ձ�־λ����

// ����Ϊȫ��Ϊ�����Ч�ʣ�����ÿ�η���ռ䣬���ж�ʹ��
static __IO uint16_t SPIx_CR2_Temp;                              // �ݴ�CR2�е�����
static __IO uint16_t SPIx_SR_Temp;                               // ���ж����ݴ�״̬�Ĵ���
static __IO uint16_t SPIx_DR_NULL;                               // ������Ҫ��������ʱ�洢���ռĴ���ֵ�ﵽ���־λĿ��
static      OS_ERR   err;                                        // �ź��������־


void BSP_SPIx_IRQHandler(void)
{
	// ��ȡ״̬�Ĵ������жϼĴ���
	
	SPIx_CR2_Temp = BSP_SPIx_PORT->CR2;
	SPIx_SR_Temp  = BSP_SPIx_PORT->SR;
	
	// ��ʾ���ͻ�����Ϊ��
	
	if ((SPIx_SR_Temp & BSP_SPIx_TXE_MASK) && (SPIx_CR2_Temp & DEF_BIT_07)) {

		switch (BSP_SPIx_DevStatus.Operation) {
			
			// ��Ϊֻд����Ϊ��дʱ��Ҫ����������䵽SPI���ݼĴ���
			
			case SPIxReadWrite:
			case SPIxWriteOnly:
				if (BSP_SPIx_DevStatus.WrBufLen> 0) {                                   
					BSP_SPIx_PORT -> DR = *(BSP_SPIx_DevStatus.WrBufPtr);               // ����������䵽SPI���ݼĴ���
					BSP_SPIx_DevStatus.WrBufPtr++;                                      // ����ָ���һ       	
				}					
				break;
				
			// ��Ϊֻ��ʱ����Ҫ�򻺳�����������ֵ�ﵽ���־λĿ��
				
			case SPIxReadOnly:
				if (BSP_SPIx_DevStatus.WrBufLen> 0) {         
					BSP_SPIx_PORT -> DR = BSP_SPI_NOUSE;                                // ֻ��ʱֻ��Ҫ��������ֵ
				}
				break;
		}		
		
		BSP_SPIx_DevStatus.WrBufLen--;                                                  // ���峤�ȼ�һ
		if (0 == BSP_SPIx_DevStatus.WrBufLen) {       
			BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                          // �ط����ж�
		}		
	}
	
	// ��ʾ���ջ������ǿ�
	
	if ((SPIx_SR_Temp & BSP_SPIx_RXNE_MASK) && (SPIx_CR2_Temp & DEF_BIT_06) ) {                                        
		
		switch (BSP_SPIx_DevStatus.Operation) {
			
			// ��Ϊֻд���߶�дͬʱ����ʱ
			
			case SPIxReadWrite:
			case SPIxReadOnly:
				if (BSP_SPIx_DevStatus.RdBufLen > 0) {
					*BSP_SPIx_DevStatus.RdBufPtr = BSP_SPIx_PORT -> DR;                   // ������д�뻺����
					BSP_SPIx_DevStatus.RdBufPtr++;
				}
				break;
				
			case SPIxWriteOnly:
				SPIx_DR_NULL = BSP_SPIx_PORT -> DR;                                   // �ն�������ñ�׼                    
				break;
		}		
		
		BSP_SPIx_DevStatus.RdBufLen--;
		// SPIͨѶ������
		
		if (0 == BSP_SPIx_DevStatus.RdBufLen) {                               
			OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemWait),
					  (OS_OPT  )  OS_OPT_POST_1,
					  (OS_ERR *)  &err);
		}
	}
	
}


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        BSP_NVIC_Configuration()
*
* Description : ��ʼ�� SPI ���ӵ�NVIC
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_SPIx_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_NVIC_Configuration(void)
{
	NVIC_InitTypeDef nvicInit;
	
	nvicInit.NVIC_IRQChannel                   = BSP_SPIx_IRQ;
	nvicInit.NVIC_IRQChannelPreemptionPriority = BSP_SPIx_NVIC_PRE_PRIO;         /* �������ȼ�                      */
	nvicInit.NVIC_IRQChannelSubPriority        = BSP_SPIx_NVIC_SUB_PRIO;         /* �����ȼ�                        */
	nvicInit.NVIC_IRQChannelCmd                = ENABLE;                         /* ʹ���ж�                        */
	
	NVIC_Init(&nvicInit);
}




/*
*********************************************************************************************************
*                                        BSP_GPIO_Init()
*
* Description : ��ʼ�� SPI���ţ�ӳ�䵽SPI����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_SPI_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
    // ����GPIOʱ�� 
	
	RCC_AHB1PeriphClockCmd(BSP_SPIx_CS_GPIO_CLK, ENABLE);            // ����Ƭѡ����ʱ��
	RCC_AHB1PeriphClockCmd(BSP_SPIx_CLK_GPIO_CLK, ENABLE);           // ����SCL����ʱ��
	RCC_AHB1PeriphClockCmd(BSP_SPIx_MOSI_GPIO_CLK, ENABLE);          // ����MOSI����ʱ��
	RCC_AHB1PeriphClockCmd(BSP_SPIx_MISO_GPIO_CLK, ENABLE);          // ����MISO����ʱ��
	
	// ���Ÿ�������
	
	GPIO_PinAFConfig(BSP_SPIx_CLK_PORT, BSP_SPIx_CLK_SOURCE, BSP_SPIx_GPIO_AF);
	GPIO_PinAFConfig(BSP_SPIx_MISO_PORT, BSP_SPIx_MISO_SOURCE, BSP_SPIx_GPIO_AF);
	GPIO_PinAFConfig(BSP_SPIx_MOSI_PORT, BSP_SPIx_MOSI_SOURCE, BSP_SPIx_GPIO_AF);

	// ��������
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_CLK_PIN;                 // ����CLk����
	GPIO_Init(BSP_SPIx_CLK_PORT, &GPIO_InitStructure);
	 
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_MISO_PIN;                // ����MISO����
	GPIO_Init(BSP_SPIx_MISO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_MOSI_PIN;                // ����MOSI����
	GPIO_Init(BSP_SPIx_MOSI_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;                  // ����CS����
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_CS_PIN;
	GPIO_Init(BSP_SPIx_CS_PORT, &GPIO_InitStructure);
}




