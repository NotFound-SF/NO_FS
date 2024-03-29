


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_buzzer.h"
#include "includes.h"

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_BUZZER_Init()
*
* Description : 初始化蜂鸣器端口引脚为推完输出模式
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BUZZER_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	  
	RCC_AHB1PeriphClockCmd(BUZZER_GPIO_CLK, ENABLE);  
	
	gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = BUZZER_PIN;
	GPIO_Init(BUZZER_GPIO_PORT, &gpio_init);
	
	BSP_BUZZER_Off();                                       /* 开机后确保蜂鸣器不发声音                          */                                    
}

/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : 开启蜂鸣器，即设施响应的端口为高电平
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BUZZER_On (void)
{
	GPIO_SetBits(BUZZER_GPIO_PORT, BUZZER_PIN);
}


/*
*********************************************************************************************************
*                                             BSP_LED_Off()
*
* Description : 关闭蜂鸣器，即将引脚设置为低电平
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BUZZER_Off(void)
{
	GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_PIN);
}


/*
*********************************************************************************************************
*                                             BSP_LED_Off()
*
* Description : 关闭蜂鸣器，即将引脚设置为低电平
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BUZZER_Func(void)
{
	OS_ERR err;
	
	BSP_BUZZER_On();
	OSTimeDlyHMSM( 0, 0, 0, 160,
				   OS_OPT_TIME_HMSM_STRICT,
				   &err );
	BSP_BUZZER_Off();
	OSTimeDlyHMSM( 0, 0, 0, 80,
				   OS_OPT_TIME_HMSM_STRICT,
				   &err );
	BSP_BUZZER_On();
	OSTimeDlyHMSM( 0, 0, 0, 160,
				   OS_OPT_TIME_HMSM_STRICT,
				   &err );
	BSP_BUZZER_Off();	
	OSTimeDlyHMSM( 0, 0, 0, 80,
				   OS_OPT_TIME_HMSM_STRICT,
				   &err );

	BSP_BUZZER_On();
	OSTimeDlyHMSM( 0, 0, 0, 160,
				   OS_OPT_TIME_HMSM_STRICT,
				   &err );
	BSP_BUZZER_Off();
	OSTimeDlyHMSM( 0, 0, 0, 400,
				   OS_OPT_TIME_HMSM_STRICT,
				   &err );
}



