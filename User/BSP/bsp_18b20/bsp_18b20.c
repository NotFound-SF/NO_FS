
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_18b20.h"


/*
*********************************************************************************************************
*                                            LOCAL DATA
*********************************************************************************************************
*/

static      __IO           uint16_t tempBuf;                                      //�����ȡ�����¶�   
static      OS_MUTEX       temp_mutex;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void            BSP_18B20_GPIO_Init(void);
static  void            BSP_18B20_WriteByte(uint8_t data);
static  CPU_BOOLEAN     BSP_18B20_GetAck(void);
static  uint8_t         BSP_18B20_ReadByte(void);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        BSP_18B20_Init()
*
* Description : ��ʼ�� GPIOӳ�䵽DS18B20
*
* Argument(s) : none
*
* Return(s)   : DEF_OK      ��ʼ���ɹ�    
*               DEF_FAIL    ��ʼ��ʧ�ܣ�δ��⵽DS18B20
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_18B20_Init(void)
{
	OS_ERR      err;
	CPU_BOOLEAN status;
	
	BSP_18B20_GPIO_Init();                                //��ʼ���˿�
	
	// �����ı�ȫ�ֻ������Ļ����ź���
	
	OSMutexCreate(&temp_mutex, "temp mutex", &err);
	
	// ȷ���ý�������ʱ���ռ�ö�ʱ��
	
	Timing_Lock();   
	
	if (0 == BSP_18B20_GetAck()) {
		status = DEF_OK;                                  //��ʾ��������
		BSP_18B20_WriteByte(0xCC);                        //����ROM
		
		// �������üĴ�������ȷ��9Bit��0.5C'
		
		BSP_18B20_WriteByte(0x4E);                        //�����ݴ���ָ��
		BSP_18B20_WriteByte(0xFF);                        //TH
		BSP_18B20_WriteByte(0xFF);                        //TL
		BSP_18B20_WriteByte(ACCURACY);                        //config�Ĵ���
		
		BSP_18B20_WriteByte(0x44);                        //����һ���¶�ת��
	} else {
		status = DEF_FAIL;                                //��ʾ����������
	}
	
	// �ͷŶ�ʱ��
	Timing_Unlock();                                  //��������ʱ������ͷŶ�ʱ��
	
	return status;
}



/*
*********************************************************************************************************
*                                        BSP_18B20_GetTemp()
*
* Description : ��ȡDS18B20���¶�ֵ�������нϳ���ת����ʱ
*
* Argument(s) : temp        ��ȡ�¶�ֵ
*
* Return(s)   : DEF_OK      ��ȡ�¶ȳɹ�    
*               DEF_FAIL    ��ȡ�¶�ʧ��
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_18B20_GetTemp(void)
{
	OS_ERR        err;
	uint16_t      temp;
	CPU_BOOLEAN   status;
	
	// ȷ���ý�������ʱ���ռ�ö�ʱ��
	
	Timing_Lock();  
	
	// ��ȡ�¶�ǰ��ת��һ��
	
	if (0 == BSP_18B20_GetAck()) {
		BSP_18B20_WriteByte(0xCC);                        //����ROM
		BSP_18B20_WriteByte(0x44);                        //����һ���¶�ת��
		Timing_Delay_Ms(CONVERT_T);                       //ת��������Ҫʱ��
	} 
	
	// �������¶ȶ�ȡ����
	if (0 == BSP_18B20_GetAck()) {
		status = DEF_OK;
		BSP_18B20_WriteByte(0xCC);                       //����ROM	
		BSP_18B20_WriteByte(0xBE);                       //���Ͷ��¶�����
		temp  = BSP_18B20_ReadByte();                    //���¶ȵ��ֽ�
		temp |= (uint16_t)BSP_18B20_ReadByte()<<8;       //���¶ȸ��ֽ�
	} else {
		status = DEF_FAIL;
	}
	
	// �ͷŶ�ʱ��
	Timing_Unlock();                                     //��������ʱ������ͷŶ�ʱ��
	
	// �޸��˻�������ֵ
	OSMutexPend(&temp_mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
	tempBuf = temp;
	OSMutexPost(&temp_mutex, OS_OPT_POST_NO_SCHED, &err);
	
	return status;
}


/*
*********************************************************************************************************
*                                        BSP_18B20_GetTempFast()
*
* Description : ��ȡDS18B20���¶�ֵ�����Ƕ�ȡ���¶Ȳ��������µ��¶ȣ����ǻ��������¶�ֵ(�ϴζ�ȡ���¶�ֵ)
*               ���Բ������ͨ��ʱ�����ֱ�ӷ��ػ���������ֵ��
*
* Argument(s) : none
*
* Return(s)   : 16λ18B20��ʽ��ԭʼ����
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint16_t   BSP_18B20_GetTempFast(void)
{
	OS_ERR   err;
	uint16_t temp;
	
	OSMutexPend(&temp_mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
	temp = tempBuf;
	OSMutexPost(&temp_mutex, OS_OPT_POST_NO_SCHED, &err);
	
	return temp;
}


/*
*********************************************************************************************************
*                                        BSP_18B20_TempTran()
*
* Description : ��18B20��������ת��Ϊ������
*
* Argument(s) : temp        DS18B20��ʽ�¶�ֵ
*
* Return(s)   : �������͵��¶�ֵ
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

float BSP_18B20_TempTran(uint16_t temp)
{
	float  dat;
	
	// ��ȡ�¶ȵ�ʵ����ֵ������������λ
	
	dat = (temp >> 4)&0x7F;                         //��ȡ��������
	dat += (float)(temp&0x0F) / 16;                 //��ȡС������
	
	// �ж��¶ȵķ���
	
	if (0 != (temp&0xF800)) {                       //�жϷ���Ϊ��ȫΪ1��ʾ�����¶�ֵ
		return -dat;
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
*                                        BSP_18B20_GPIO_Init()
*
* Description : ��ʼ�� DS18B20��GPIO����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_18B20_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_18B20_GPIO_Init (void)
{
	GPIO_InitTypeDef  gpioInit;
	
	RCC_AHB1PeriphClockCmd(DS18B20_GPIO_CLK, ENABLE);        //����ʱ��
	
	gpioInit.GPIO_Pin   = DS18B20_PIN;
	gpioInit.GPIO_Mode  = GPIO_Mode_OUT;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_OType = GPIO_OType_OD;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(DS18B20_GPIO_PORT, &gpioInit);                 //��ʼ������
}


/*
*********************************************************************************************************
*                                        BSP_18B20_GetAck()
*
* Description : ��ȡ������DS18B20������Ϣ
*
* Argument(s) : none
*
* Return(s)   : 0      �⵽DS18B20     
*               1      δ��⵽DS18B20
*
* Caller(s)   : BSP_18B20_Init(), BSP_18B20_GetTemp()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static CPU_BOOLEAN BSP_18B20_GetAck(void)
{
	CPU_BOOLEAN status;
	                                
	// ������������λ���弴480~960us�͵�ƽ
	
	DS18B20_PIN_OUT();                                // ȷ���������ģʽ
	DS18B20_PIN_WR_0();                               // ��������Ϊ�͵�ƽ
	Timing_Delay_Us(500);
	
	// �����ͷ����ߵȴ�18B20Ӧ��
	
	DS18B20_PIN_IN();                                 //����Ϊ����ģʽ�ͷ�����
	Timing_Delay_Us(60);                              
	status = DS18B20_PIN_RD();                        //��ȡ����״̬
	
	//�ȴ�18B20Ӧ�����
	
	Timing_Delay_Us(240);                             	
	
	return status;                                   
}


/*
*********************************************************************************************************
*                                        BSP_18B20_WriteByte()
*
* Description : ������DS18B20д��һ���ֽ�
*
* Argument(s) : data    Ҫд������ݻ�������
*
* Return(s)   : none
*
* Caller(s)   : BSP_18B20_Init(), BSP_18B20_GetTemp()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_18B20_WriteByte(uint8_t data)
{
	uint8_t mask;
	
	DS18B20_PIN_OUT();                                    // ȷ���������ģʽ
	
	for(mask = 0x01; mask != 0; mask <<= 1) {
		// ��֤ÿ�ο�ʼ���Ǹߵ�ƽ
		DS18B20_PIN_WR_1();     
		Timing_Delay_Us(1);                               //����bit֮��ļ�����뱣֤����1us
		
		// ��������2us�ĵ͵�ƽ
		
		DS18B20_PIN_WR_0();                               // ��������Ϊ�͵�ƽ
		Timing_Delay_Us(2);
		
		// ��������bitֵ������һ��
		
		if(mask & data) 
			DS18B20_PIN_WR_1();                           //д1
		else 
			DS18B20_PIN_WR_0();                           //д0
		
		Timing_Delay_Us(58);                              //�ӳ�58us
	}
	
	DS18B20_PIN_WR_1();                                   //ÿ��д�걣������Ϊ�ߵ�ƽ
	
}


/*
*********************************************************************************************************
*                                        BSP_18B20_ReadByte()
*
* Description : ��DS18B20��ȡһ���ֽ�
*
* Argument(s) : none
*
* Return(s)   : ��ȡ������
*
* Caller(s)   : BSP_18B20_GetTemp()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static uint8_t  BSP_18B20_ReadByte(void)
{
	uint8_t mask;
	uint8_t data = 0x00;                                        // ����ʼ���ͻᵼ�¶�ȡ������
	
	for(mask = 0x01; mask != 0; mask <<= 1) { 
		
		// ������ȡÿ��bit����ʼ�ź�
		
		DS18B20_PIN_OUT();                                  // ȷ���������ģʽ
		DS18B20_PIN_WR_0();                   
		Timing_Delay_Us(2);   
		
		//�����ȡ״̬
		
		DS18B20_PIN_IN();   
		Timing_Delay_Us(3);                                //������15us���ڲ���   
		
		if (DS18B20_PIN_RD())                                 
			data |= mask;
		else	
			data  &= ~mask;
		
		Timing_Delay_Us(56);                               //ȷ����������Ϊ60us
	}
	
	return data;
}


