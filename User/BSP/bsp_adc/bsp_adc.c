
/*
*********************************************************************************************************
*        ��������Ҫ����Ե�����������MQ-2���崫����������ǿ��������Ҫ�õ�AD�Ĵ�������д
*        ���Ը�����ֻ����һ��ADC���ҿ���3��ͨ��������������ֻ����һ��,�ڵ��øú�����ADC
*        �Ϳ�ʼĬĬ���ٺ�̨ת���������Զ����£�ֻ�ṩ��ȡ�ӿڣ����Բ���Ҫ�ź���������
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_adc.h"

/*
*********************************************************************************************************
*                                              LOCAL DATA
*********************************************************************************************************
*/

static   __IO   uint16_t  ADC_Value[DAC_CH_NUM];                           //���ADC��ȡ�����ݣ���DMA����               

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static           void            BSP_ADC_DMA_Config(void);
static           void            BSP_ADC_GPIO_Config(void);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_ADC_Init()
*
* Description : ����DMA������IO���ţ���DMA���ӵ��ڴ���ADC����������ͨ��
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   BSP_ADC_Init(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	// ����ADC����ʱ��
	
	ADC_RCC_CMD(ADC_CLK, ENABLE);
	
	// ��ʼDMA��GPIO
	
	BSP_ADC_DMA_Config();
	BSP_ADC_GPIO_Config();
	
		
	// ADC Common �ṹ�� ���� ��ʼ��
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                       // ����ADCģʽ
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;                    // ʱ��Ϊfpclk x��Ƶ
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;        // ��ֹDMAֱ�ӷ���ģʽ	
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;  // ����ʱ����	ʵ����˫��ģʽ
	
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	// ADC_InitTypeDef�ṹ���ʼ��
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                         // 12λ�ֱ���
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                                   // ��ͨ���ɼ�����Ҫ	
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                             // ����ת����ֻ��Ҫ����һ�ξͿ�ʼ����ת��
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;    // ��ʹ���ⲿ����
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;          // ��Ϊ��ʹ���ⲿ����������Ч
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                         // �����Ҷ���	
	ADC_InitStructure.ADC_NbrOfConversion = DAC_CH_NUM;                            // ת��ͨ�� 3��     
	
	ADC_Init(ADC_PORT, &ADC_InitStructure);
	//---------------------------------------------------------------------------
	
	// ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ56��ʱ������                    
	
	ADC_RegularChannelConfig(ADC_PORT, DAC_CH1, 1, ADC_SampleTime_112Cycles);      //��һ��ת��
	ADC_RegularChannelConfig(ADC_PORT, DAC_CH2, 2, ADC_SampleTime_112Cycles);      //�ڶ���ת��
	ADC_RegularChannelConfig(ADC_PORT, DAC_CH3, 3, ADC_SampleTime_112Cycles);      //������ת��

	
	ADC_DMARequestAfterLastTransferCmd(ADC_PORT, ENABLE);
    ADC_DMACmd(ADC_PORT, ENABLE);                                                  //ʹ��ADC DMA����
	ADC_Cmd(ADC_PORT, ENABLE);                                                     //ʹ��ADC
	ADC_SoftwareStartConv(ADC_PORT);                                               //�������ת��
}



/*
*********************************************************************************************************
*                                        BSP_ADC_GetDat()
*
* Description : ����ͨ����ŷ��ض�Ӧͨ��������
*
* Argument(s) : CHx  BSP_ADC_CH1      ͨ��1
*                    BSP_ADC_CH2      ͨ��2
*                    BSP_ADC_CH3      ͨ��3
*
* Return(s)   : ADC��������,δ��������
*
* Caller(s)   : Application   
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint16_t BSP_ADC_GetDat(BSP_ADC_CHx CHx)
{
	uint16_t dat;
	 
	switch (CHx) {
		
		case BSP_ADC_CH1:
			dat = ADC_Value[0];
			break;
		
		case BSP_ADC_CH2:
			dat = ADC_Value[1];
			break;
	
		case BSP_ADC_CH3:
			dat = ADC_Value[2];
			break;
	}
	
	return dat;
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
*                                        BSP_ADC_DMA_Config()
*
* Description : ����DMA����DMA���ӵ��ڴ���ADC
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_ADC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_ADC_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(ADC_DMA_CLK, ENABLE);      			                     //����DMAʱ��

	DMA_InitStructure.DMA_Channel = ADC_DMA_CHANNEL;
	DMA_InitStructure.DMA_BufferSize = DAC_CH_NUM;            		                 //��������С
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;    			             //�ر�FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC_Value;                     //�ڴ����ַ
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                      //�����ݷ���
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                          //�ڴ��ַ��
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                  //ѭ��ģʽ���ܿ���˫����ģʽ
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC_DR_ADDR;   
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;      //�����ֽڴ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;                               //�������ȼ�
	
	
	DMA_Init(ADC_DMA_STREAM, &DMA_InitStructure);
	DMA_Cmd(ADC_DMA_STREAM, ENABLE);
}



/*
*********************************************************************************************************
*                                        ADC_DMA_Config()
*
* Description : ��GPIOӳ�䵽ADC
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_ADC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_ADC_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// ʹ�� GPIO ʱ��
	RCC_AHB1PeriphClockCmd(DAC_CH1_GPIO_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(DAC_CH2_GPIO_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(DAC_CH3_GPIO_RCC, ENABLE);
		
	// ���� IO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;                    //������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = DAC_CH1_PIN;
	GPIO_Init(DAC_CH1_GPIO_PORT, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = DAC_CH2_PIN;
	GPIO_Init(DAC_CH2_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = DAC_CH3_PIN;
	GPIO_Init(DAC_CH3_GPIO_PORT, &GPIO_InitStructure);
}












