
/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_light.h"
#include "bsp_adc.h"


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_LIGHT_Init()
*
* Description : 初始化LED端口引脚为推完输出模式
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

void  BSP_LIGHT_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	
	RCC_AHB1PeriphClockCmd(LIGHT_GPIO_CLK, ENABLE);    

	gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = LIGHT_PIN;
	GPIO_Init(LIGHT_GPIO_PORT, &gpio_init);
	
	BSP_LIGHT_Off();                                       /* 开机后确保所有LED都关闭                          */                                    
}



/*
*********************************************************************************************************
*                                             BSP_LIGHT_On()
*
* Description : 开启LED
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

void  BSP_LIGHT_On (void)
{
	GPIO_SetBits(LIGHT_GPIO_PORT, LIGHT_PIN);
}


/*
*********************************************************************************************************
*                                             BSP_LIGHT_Off()
*
* Description : 关闭LED
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

void  BSP_LIGHT_Off(void)
{
	GPIO_ResetBits(LIGHT_GPIO_PORT, LIGHT_PIN);
}




/*
*********************************************************************************************************
*                                             BSP_Light_State()
*
* Description : 获获取光照水平
*
* Argument(s) : none
*
* Return(s)   : High_Level, Normal_Level
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
Light_Level  BSP_Light_State (void)
{
	uint8_t   count = 0;
	OS_ERR    err;
	
	if (BSP_ADC_GetDat(BSP_ADC_CH2) < HIGH_LIGHT_VALUE) {
		for (count = 0; count < 10; count++) {
			
			OSTimeDlyHMSM( 0, 0, 0, 4,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
			
			if (BSP_ADC_GetDat(BSP_ADC_CH2) > HIGH_LIGHT_VALUE) 
				break;			
		}
	}
	
	if (10 == count)
		return Light_High_Level;

	return Light_Normal_Level;	
}

