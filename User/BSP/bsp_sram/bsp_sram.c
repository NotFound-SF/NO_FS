
/*
*********************************************************************************************************
*      ��Ϊֻ�Ӵ������ͺŵ�SRAM,���Ըÿ�����ֻ��֤������ IS62WV51216BLL 55ns��
*********************************************************************************************************
*/


#include "bsp_sram.h"


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void         BSP_SRAM_CTRL_GPIO_Init (void);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAl FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         BSP_SRAM_Init()
*
* Description :  ��ʼ��SRAM,���а�����ʼ��FSCM
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_SRAM_Init(void)
{
	FSMC_NORSRAMInitTypeDef        sramInit;
	FSMC_NORSRAMTimingInitTypeDef  sramTimigRd;
	
	// ��ʼGPIOͨ�����źͿ���SRAM�Ķ��ؿ�������
	
	BSP_SRAM_CTRL_GPIO_Init();                                                
	
	FSMC_NORSRAMDeInit(BSP_SRAM_Bank);
	
	// ��ʱ������
	
	sramTimigRd.FSMC_AccessMode            = FSMC_AccessMode_A;                //ģʽA�μ��ο��ֲ�
	sramTimigRd.FSMC_DataSetupTime         = 0x08;                             // x/55ns = 168/1000 ns ,x = 9.24
	sramTimigRd.FSMC_AddressSetupTime      = 0x03;                             //��ַ����ʱ��
	sramTimigRd.FSMC_CLKDivision           = 0x00;                             //SRAM�������첽ģʽ��λ������                      
	sramTimigRd.FSMC_DataLatency           = 0x00;                             //��ʾ�����ӳ����ڣ�SRAM�������첽ģʽ��λ������ 
	sramTimigRd.FSMC_AddressHoldTime       = 0x00;                             //ʹ����ģʽD,ģʽA��λ������
	sramTimigRd.FSMC_BusTurnAroundDuration = 0x01;                             //SRAM ��λ������
	
	sramInit.FSMC_MemoryType            = FSMC_MemoryType_SRAM;                //�洢������
	sramInit.FSMC_Bank                  = BSP_SRAM_Bank;                       //SRAM���ڿ�
	sramInit.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;         //�����õ�ַ��������������
	sramInit.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;          //����д����
	sramInit.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;           //��дʱ����Էֱ�����
	sramInit.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;            //�ⲿ�洢�����ݿ��
	
	sramInit.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;        //��������ͬ���洢��
	sramInit.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;       //��SRAM�޵ȴ�����
	sramInit.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;         //�ȴ��ź�Ϊ�ߵ�ƽ��Ч��SRAM����
	sramInit.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;             //����NOR
	sramInit.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	sramInit.FSMC_WrapMode              = FSMC_WrapMode_Disable;               //����ͻ��ģʽ����Ч
	sramInit.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;             //����ͬ��ģʽ����Ч
	
	sramInit.FSMC_ReadWriteTimingStruct = &sramTimigRd;
	sramInit.FSMC_WriteTimingStruct     = &sramTimigRd;
	
	FSMC_NORSRAMInit(&sramInit);
	FSMC_NORSRAMCmd(BSP_SRAM_Bank, ENABLE);
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
*                                         BSP_SRAM_CTRL_GPIO_Init()
*
* Description :  ��ʼ��SRAM��������
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_SRAM_GPIO_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/
static  void  BSP_SRAM_CTRL_GPIO_Init (void)
{
	GPIO_InitTypeDef gpioInit;
	                                                  
	// �����������Ŷ˿�ʱ��
	
	RCC_AHB1PeriphClockCmd(SRAM_GPIO_PORT_RCC_NCE, ENABLE);
	RCC_AHB1PeriphClockCmd(SRAM_GPIO_PORT_RCC_NLB, ENABLE);
	RCC_AHB1PeriphClockCmd(SRAM_GPIO_PORT_RCC_NUB, ENABLE);
																			   
	// ���ÿ�������
	
	GPIO_PinAFConfig(SRAM_GPIO_PORT_NCE, SRAM_PIN_SOURCE_NCE, GPIO_AF_FSMC);
	GPIO_PinAFConfig(SRAM_GPIO_PORT_NLB, SRAM_PIN_SOURCE_NLB, GPIO_AF_FSMC);
	GPIO_PinAFConfig(SRAM_GPIO_PORT_NUB, SRAM_PIN_SOURCE_NUB, GPIO_AF_FSMC);
		                                                                      
	 // ���ÿ�������ģʽ
	
	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_UP;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// ��ʼ��NCE
	
	gpioInit.GPIO_Pin   = SRAM_PIN_NCE;                                        
	GPIO_Init(SRAM_GPIO_PORT_NCE, &gpioInit);
	
	 // ��ʼ��NLB
	
	gpioInit.GPIO_Pin   = SRAM_PIN_NLB;                                       
	GPIO_Init(SRAM_GPIO_PORT_NLB, &gpioInit);
	
	// ��ʼ��NUB
	
	gpioInit.GPIO_Pin   = SRAM_PIN_NUB;                                        
	GPIO_Init(SRAM_GPIO_PORT_NUB, &gpioInit);
}



