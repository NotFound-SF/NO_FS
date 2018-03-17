


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include"bsp_key.h"


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_KEY_Init()
*
* Description : ��ʼ�������˿ڣ�ʹ������Ϊ����ģʽ
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : main().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_KEY_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	
	RCC_AHB1PeriphClockCmd(KEY1_GPIO_CLK, ENABLE);    
	
	gpio_init.GPIO_Mode  = GPIO_Mode_IN;
	gpio_init.GPIO_OType = GPIO_OType_OD;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = KEY1_PIN;
	GPIO_Init(KEY1_GPIO_PORT, &gpio_init);                                    
}

/*
*********************************************************************************************************
*                                             BSP_KEY_In_Long()
*
* Description : ���ڿ���ʱ���һ������������
*
* Argument(s) : none
*
* Return(s)   : 0:��ʾû�а������� 1:��ʾ����������������
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
Key_Status  BSP_KEY_In_Long (void)
{
	OS_ERR      err;
	uint8_t     times = 0;
	Key_Status  status = KEY_UP;                   // ��ʾ����δ������
	
	while (1) {
		switch (status) {
			case KEY_UP:
				times = 0;                  
				if(0 == KEY1_PIN_RD()) {
					status = KEY_HOLD;
					OSTimeDlyHMSM( 0, 0, 0, 6,      // ��ʱ6ms��������
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
				} else 
					return KEY_UP;
				break;
			
			case KEY_HOLD:
				if (0 == KEY1_PIN_RD()) {
					OSTimeDlyHMSM( 0, 0, 0, 200,     // ��ʱ600
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
					times++;
					if (times >= 8) 
						status = KEY_DOWN;           // ���ڵı�����						
				} else 	
					status = KEY_UP;                 // ��ʾ�ⲻ��һ��������
				break;
				
			case KEY_DOWN:
				return KEY_DOWN;
		}
		
	}	
	
}


