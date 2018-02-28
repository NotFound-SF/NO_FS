
/*
*********************************************************************************************************
*              ���ļ����ھ�ȷ������ʱ��ʹ��Ӳ����ʱ��10��Ϊʱ��Դ,��Ϊ��ʱ��10����APB2(84Mhz)���ߣ�����ʱ��
*              ����ȷĿ���Ǳ�֤�Ǳ�׼�ӿ�ʱ�����ȷ��,��������DS18B20Ϊ�˱�֤��ʱ�ľ�ȷ�ԣ���ȷ�����̵����ȼ�
*              �㹻�ߣ���������������̺�ʱ��
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_timing.h"

/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct{
	OS_SEM   SemLock;                                  //��IIC��ռ�ź���                                
    OS_SEM   SemWait;                                  //IIC�ȴ��ź���     
}BSP_TIMING_STATUS;



/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_TIMING_STATUS      TimingStatus;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void         NVIC_Configuration   (void);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_Timing_Init()
*
* Description : ��ʼ��ʱ����
*
* Argument(s) : none
*
* Return(s)   : DEF_OK     ��ʼ���ɹ�
*               DEF_FAIL   ��ʼ��ʧ��
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_Timing_Init (void)
{
	OS_ERR err;
	TIM_TimeBaseInitTypeDef baseInit;
	
	TIMING_PERIPH_RCC_CMD(TIMING_PERIPH_RCC, ENABLE);                       //������ʱ��RCCʱ��
	                                                                        //������ռ�ź���
	OSSemCreate((OS_SEM    *)&(TimingStatus.SemLock),                      
                (CPU_CHAR  *)"Timing SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	                                                                        //���������ź���
	OSSemCreate((OS_SEM    *)&(TimingStatus.SemWait),                      
                (CPU_CHAR  *)"Timing SemWait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
	baseInit.TIM_Period            = 0xFFFF;                                //��������
	baseInit.TIM_Prescaler         = 0xFFFF;                                //��84Mʱ�ӷ�Ƶ
	baseInit.TIM_CounterMode       = TIM_CounterMode_Up;                    //��������
	baseInit.TIM_ClockDivision     = TIM_CKD_DIV1;                          //�Զ�ʱ��10����
	baseInit.TIM_RepetitionCounter = 0x00;                                  //ֻ���ø߼���ʱ��1��8
	                               
	NVIC_Configuration();                                                   //���ӵ�NVIC
	TIM_TimeBaseInit(TIMING_TYPE, &baseInit);                               //������������һ��ǰ�����һʹ�ܾͻ��������ж�
	TIM_ClearITPendingBit(TIMING_TYPE, TIM_IT_Update);
	TIM_ITConfig(TIMING_TYPE, TIM_IT_Update, ENABLE);                       //���������ж�
	
	TIMING_TYPE -> CR1 |= DEF_BIT_03;                                      //����Ϊ������ģʽ                                         
	
	return DEF_OK;
}


/*
*********************************************************************************************************
*                                        Timing_Lock()
*
* Description : ���̶�ռ�ö�ʱ������һ��������ʱ��ͷ���ã���ֹ��������ռ�ö�ʱ��������ʱ����ȷ
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
void Timing_Lock(void)
{	
	OS_ERR err;
	
	OSSemPend((OS_SEM *)&(TimingStatus.SemLock),                            //�����ö˿�
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
}



/*
*********************************************************************************************************
*                                        Timing_Unlock()
*
* Description : ����ȡ����ռ�ö�ʱ������һ��������ʱ���β���ã���ֹ������������
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

void Timing_Unlock(void)
{
	OS_ERR err;
	
	OSSemPost((OS_SEM *)&(TimingStatus.SemLock),                           //�ͷŸö�ʱ��
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);                                            
}


/*
*********************************************************************************************************
*                                        Timing_Delay_Us()
*
* Description : us����ʱ����
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

void Timing_Delay_Us(uint16_t count)     //2.166
{
	OS_ERR err;
	
	if (0 == count) {                                          //����Ҫ��ʱ
		return;
	}
	
	TIMING_TYPE -> PSC   = TIMING_PERIOD_US;                   //��Ƶ��0.25us
	
	if(count <= TIMING_NO_SCHED_MAX_US) {
		TIMING_TYPE->DIER  &= ~0x01;                                 //�ȹر��жϣ���ֹ�ж���ǰ�ͷ��ź���
		TIMING_TYPE -> ARR  = (count << TIMING_BASE_SHIFT)-1;
		TIMING_TYPE -> CR1 |= 0x01;                                  //ʹ�ܼ�����,���ڴ˴���ֹ��ʱ̫�����������ж�
		while(0 == (TIMING_TYPE->SR&TIMING_UPDATE_MASK)) {           //��ȡ��־λ
		                                                             //�ȴ���ʱ����
		}
		TIMING_TYPE->SR &= ~TIMING_UPDATE_MASK;                      //����жϱ�־	
		TIMING_TYPE->DIER |= 0x01;                                   //�����жϣ�Ϊ������ĳ�ʱ����ʱ׼��
		
	} else {                                                         //ֻ��ʱ�����ϴ�ʱ������
		TIMING_TYPE -> ARR   = ((count-6) << TIMING_BASE_SHIFT)-2;   //��Ҫ����
		TIMING_TYPE -> CR1  |= 0x01;                                 //����������
		OSSemPend((OS_SEM *)&(TimingStatus.SemWait),                 //��������ʱ����
				  (OS_TICK ) 0,
				  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
				  (CPU_TS *) 0,
				  (OS_ERR *) &err);
	}
}


/*
*********************************************************************************************************
*                                        Timing_Delay_Ms()
*
* Description : ms����ʱ����
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

void Timing_Delay_Ms(uint16_t count)
{	
	OS_ERR err;
	
	if (0 == count) {
		return;                                                //����ʱֱ�ӷ���
	}
	
	TIMING_TYPE -> PSC   = TIMING_PERIOD_MS;                   //ʱ�����ڷ�Ƶ��0.25ms
	TIMING_TYPE -> ARR   = ((count << TIMING_BASE_SHIFT) - 1); //��������1��׼ȷ
	TIMING_TYPE -> CR1  |= 0x01;                               //ʹ�ܼ�����
	
	OSSemPend((OS_SEM *)&(TimingStatus.SemWait),               //��������ʱ����
		  (OS_TICK ) 0,
		  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
		  (CPU_TS *) 0,
		  (OS_ERR *) &err);
}


/*
*********************************************************************************************************
*                                        Timing_Handler() 
*
* Description : �����жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : TIM2_IRQHandler()
*
* Note(s)     : none.
*********************************************************************************************************
*/


void  Timing_Handler (void)
{	
	OS_ERR   err;
		                           
	if(TIMING_TYPE->SR & TIMING_UPDATE_MASK) {                        //��ȡ״̬
		TIMING_TYPE->SR &= ~TIMING_UPDATE_MASK;                       //����жϱ�־	
		TIMING_TYPE->CR1 &= ~0x01;                                    //ʧ�ܼ�����
		
		OSSemPost((OS_SEM *)&(TimingStatus.SemWait),                  //ȡ���ȴ�
	              (OS_OPT  ) OS_OPT_POST_1,
	              (OS_ERR *) &err);
	}
	                                                                  //���жϽ�������һ��Ҫ��֤���̵���
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
*                                        NVIC_Configuration()
*
* Description : ����ʱ�����ӵ���Ӧ���ж�
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_Timing_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  NVIC_Configuration (void)
{
	NVIC_InitTypeDef NvicInit;
	
	NvicInit.NVIC_IRQChannel = BSP_TIMING_IRQ;
	NvicInit.NVIC_IRQChannelSubPriority = BSP_TIMING_NVIC_SUB_PRIO;
	NvicInit.NVIC_IRQChannelPreemptionPriority = BSP_TIMING_NVIC_PRE_PRIO;
	NvicInit.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NvicInit);
}





