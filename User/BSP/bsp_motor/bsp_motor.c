


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_motor.h"


/*
*********************************************************************************************************
*                                           LOCAL DATA
*********************************************************************************************************
*/

static  OS_SEM    SemLock;                                        //ȷ��ֻ��һ�����̵��ò������,�������Ų������ת��

static  const uint8_t BeatCode[8] = {                             //����������Ķ�Ӧ��IO���ƴ���
     0xE, 0xC, 0xD, 0x9, 0xB, 0x3, 0x7, 0x6
};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void            BSP_MOTOR_GPIO_Init(void);
static  void            BSP_MOTOR_GPIO_OUT(uint8_t dat);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        BSP_MOTOR_Init()
*
* Description : ��ʼ���������
*
* Argument(s) : none
*
* Return(s)   : DEF_FAIL
*             ��DEF_OK
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_MOTOR_Init(void)
{
	CPU_ERR  err;
	
	// ���������ź���
	
	OSSemCreate((OS_SEM    *)& SemLock,
                (CPU_CHAR  *)  "Motor SemLock", 
	            (OS_SEM_CTR )  1,
	            (OS_ERR    *)  &err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	BSP_MOTOR_GPIO_Init();
	BSP_MOTOR_GPIO_OUT(0x00);                        // ȷ�����������ͨ��          
	
	return DEF_OK;
}


/*
*********************************************************************************************************
*                                        BSP_Turn_Motor()
*
* Description : ��һ������ͽǶ�ת�������
*
* Argument(s) : angle   ת���ĽǶ�
*               dir     MOTOR_DIR_RIGHT ���ת����ת 
*                       MOTOR_DIR_RIGHT ���ת����ת 
*
* Return(s)   : none    
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Turn_Motor(uint16_t angle, MOTOR_DIR_X dir)
{
	CPU_ERR  err;
	uint8_t  index = 0;                                          //�����������
	uint32_t beats = (angle * 4076) / 360;                       //������

	if(0 == angle) {
		BSP_MOTOR_GPIO_OUT(0x00);                                // ȷ�����������ͨ��
		return;
	}

	//�������õȴ���ֱ�������߳��ͷŲ������
	 
	OSSemPend((OS_SEM *)& SemLock,                               //�����ö˿�
			  (OS_TICK )  0,
			  (OS_OPT  )  OS_OPT_PEND_BLOCKING,
			  (CPU_TS *)  0,
			  (OS_ERR *)& err);


	for ( ; beats>0; beats--) {
		
		if (MOTOR_DIR_LIFT == dir) {
			index++; 
		} else {
			index--;
		}
		
		index = index & 0x07;                                    // �������
		
		BSP_MOTOR_GPIO_OUT( BeatCode[index] );
		
		// ÿ��������ʱ2ms
		OSTimeDly((OS_TICK )  3, 
				  (OS_OPT  )  OS_OPT_TIME_DLY, 
				  (OS_ERR *)& err);
	}

	BSP_MOTOR_GPIO_OUT(0x00);                                   // ȷ�����������ͨ��   
	
	//ת�������ͷ��ź���

	OSSemPost((OS_SEM *)& SemLock,
			  (OS_OPT  )  OS_OPT_POST_1,
			  (OS_ERR *)& err);
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
*                                        BSP_MOTOR_GPIO_Init()
*
* Description : ��ʼ�� ���������GPIO����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_MOTOR_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_MOTOR_GPIO_Init (void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
    // ����GPIOʱ�� 
	
	RCC_AHB1PeriphClockCmd(MOTOR_IN1_GPIO_CLK, ENABLE);            // ����IN1����ʱ��
	RCC_AHB1PeriphClockCmd(MOTOR_IN2_GPIO_CLK, ENABLE);            // ����IN2����ʱ��
	RCC_AHB1PeriphClockCmd(MOTOR_IN3_GPIO_CLK, ENABLE);            // ����IN3����ʱ��
	RCC_AHB1PeriphClockCmd(MOTOR_IN4_GPIO_CLK, ENABLE);            // ����IN4����ʱ��
 
	
	// ��ʼ�� IN1�� IN2�� IN3�� IN4
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	
	GPIO_InitStructure.GPIO_Pin = MOTOR_IN1_PIN;                    // CS����
	GPIO_Init(MOTOR_IN1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = MOTOR_IN2_PIN;                    // CS����
	GPIO_Init(MOTOR_IN2_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MOTOR_IN3_PIN;                    // CS����
	GPIO_Init(MOTOR_IN3_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MOTOR_IN4_PIN;                    // CS����
	GPIO_Init(MOTOR_IN4_PORT, &GPIO_InitStructure);
}




/*
*********************************************************************************************************
*                                        BSP_MOTOR_GPIO_OUT()
*
* Description : ʹ����������������ĸ��������Ӧ�ĵ�ƽ
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_Turn_Motor()
*
* Note(s)     : none.
*********************************************************************************************************
*/


static  void   BSP_MOTOR_GPIO_OUT(uint8_t dat)
{
	if (dat & DEF_BIT_00) 
		MOTOR_IN1_HIGH();
	else
		MOTOR_IN1_LOW();
	
	if (dat & DEF_BIT_01) 
		MOTOR_IN2_HIGH();
	else
		MOTOR_IN2_LOW();
	
	if (dat & DEF_BIT_02) 
		MOTOR_IN3_HIGH();
	else
		MOTOR_IN3_LOW();
	
	if (dat & DEF_BIT_03) 
		MOTOR_IN4_HIGH();
	else
		MOTOR_IN4_LOW();
		
}	



