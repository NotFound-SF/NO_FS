
#include "bsp_fsmc.h"




/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void      BSP_SRAM_ADDR_GPIO_Init   (void);
static  void      BSP_SRAM_DAT_GPIO_Init    (void);
static  void      BSP_SRAM_RW_GPIO_Init     (void);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                         BSP_FSMC_COMMON_Init()
*
* Description :  ��ʼ��FSMC�Ĺ������ţ��������ݣ���ַ����д���ţ�����FSMCʱ��
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

void BSP_FSMC_COMMON_Init(void)
{
	// ����FSMC����ʱ��
	
	FSMC_PERIPH_RCC_CMD(FSMC_PERIPH_RCC, ENABLE);    

	// ��ʼ����������
	
	BSP_SRAM_RW_GPIO_Init();
	BSP_SRAM_DAT_GPIO_Init();
	BSP_SRAM_ADDR_GPIO_Init();
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
*                                         BSP_SRAM_ADDR_GPIO_Init()
*
* Description :  ��ʼ��SRAM�ĵ�ַ����
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

static void  BSP_SRAM_ADDR_GPIO_Init (void)
{
	GPIO_InitTypeDef gpioInit;
	
	// ��������ʱ��
	
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_A0_9, ENABLE);                   
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_A10_15, ENABLE);
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_A16_18, ENABLE);
	
	// ��������A0-A9
	
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A0, GPIO_AF_FSMC);   
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A3, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A6, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A0_9, FSMC_PIN_SOURCE_A9, GPIO_AF_FSMC);
	                                                                           
	// ��������A10-A15
	
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A10_15, FSMC_PIN_SOURCE_A10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A10_15, FSMC_PIN_SOURCE_A11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A10_15, FSMC_PIN_SOURCE_A12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A10_15, FSMC_PIN_SOURCE_A13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A10_15, FSMC_PIN_SOURCE_A14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A10_15, FSMC_PIN_SOURCE_A15, GPIO_AF_FSMC);
	                                                                           
	// ��������A16-A18																		   
																			   
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A16_18, FSMC_PIN_SOURCE_A16, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A16_18, FSMC_PIN_SOURCE_A17, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_A16_18, FSMC_PIN_SOURCE_A18, GPIO_AF_FSMC);
     
    // ����GPIOģʽ
	
 	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// ��ʼ��A0-A9ģʽ
	
	gpioInit.GPIO_Pin   = FSMC_PINS_A0_9;                                     
	GPIO_Init(FSMC_GPIO_PORT_A0_9, &gpioInit);
	
	// ��ʼ��A10-A15ģʽ
	
	gpioInit.GPIO_Pin   = FSMC_PINS_A10_15;                                    
	GPIO_Init(FSMC_GPIO_PORT_A10_15, &gpioInit);
	
	// ��ʼ��A16-A18ģʽ
	
	gpioInit.GPIO_Pin   = FSMC_PINS_A16_18;                                    
	GPIO_Init(FSMC_GPIO_PORT_A16_18, &gpioInit);
}



/*
*********************************************************************************************************
*                                         BSP_SRAM_DAT_GPIO_Init()
*
* Description :  ��ʼ��SRAM����������
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

static void  BSP_SRAM_DAT_GPIO_Init (void)
{
	                     
	GPIO_InitTypeDef gpioInit;
	
	// ����GPIO����ʱ��
	
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_D0_3, ENABLE);                  
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_D4_12, ENABLE);
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_D13_15, ENABLE);
	             
	// ��������D0-D3
	
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D0_3, FSMC_PIN_SOURCE_D0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D0_3, FSMC_PIN_SOURCE_D1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D0_3, FSMC_PIN_SOURCE_D2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D0_3, FSMC_PIN_SOURCE_D3, GPIO_AF_FSMC);
		
	// ��������D4-D12
	
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D6, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D4_12, FSMC_PIN_SOURCE_D12, GPIO_AF_FSMC);
				
	// ��������D4-D12
	
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D13_15, FSMC_PIN_SOURCE_D13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D13_15, FSMC_PIN_SOURCE_D14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_D13_15, FSMC_PIN_SOURCE_D15, GPIO_AF_FSMC);
	
	// ����GPIOģʽ
	
	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// ��ʼ��D0-D3ģʽ
	
	gpioInit.GPIO_Pin   = FSMC_PINS_D0_3;                                      
	GPIO_Init(FSMC_GPIO_PORT_D0_3, &gpioInit);
	
	// ��ʼ��D4-D12ģʽ
	
	gpioInit.GPIO_Pin   = FSMC_PINS_D4_12;                                     
	GPIO_Init(FSMC_GPIO_PORT_D4_12, &gpioInit);
	
	// ��ʼ��D13-D15ģʽ
	
	gpioInit.GPIO_Pin   = FSMC_PINS_D13_15;                                    
	GPIO_Init(FSMC_GPIO_PORT_D13_15, &gpioInit);
}



/*
*********************************************************************************************************
*                                         BSP_SRAM_RW_GPIO_Init()
*
* Description :  ��ʼ��SRAM�Ķ�д����
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

static void BSP_SRAM_RW_GPIO_Init(void)
{
	GPIO_InitTypeDef gpioInit;
	
	// ������д��������ʱ��
	
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_NWE, ENABLE);
	RCC_AHB1PeriphClockCmd(FSMC_GPIO_PORT_RCC_NOE, ENABLE);
	
	// �����Ÿ��õ�FSMC
	
	GPIO_PinAFConfig(FSMC_GPIO_PORT_NWE, FSMC_PIN_SOURCE_NWE, GPIO_AF_FSMC);
	GPIO_PinAFConfig(FSMC_GPIO_PORT_NOE, FSMC_PIN_SOURCE_NOE, GPIO_AF_FSMC);
	
	// ����GPIOģʽ
	
	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// ��ʼ��NWE
	
	gpioInit.GPIO_Pin   = FSMC_PIN_NWE;                                        
	GPIO_Init(FSMC_GPIO_PORT_NWE, &gpioInit);
	
	 // ��ʼ��NOE
	
	gpioInit.GPIO_Pin   = FSMC_PIN_NOE;                                       
	GPIO_Init(FSMC_GPIO_PORT_NOE, &gpioInit);
	
}



