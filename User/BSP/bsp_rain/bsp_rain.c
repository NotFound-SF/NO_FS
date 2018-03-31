

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include"bsp_rain.h"


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_Rain_Init()
*
* Description : 初始化下雨传感器检测端口，使其设置为输入模式
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

void  BSP_Rain_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	
	RCC_AHB1PeriphClockCmd(RAIN_GPIO_CLK, ENABLE);    
	
	gpio_init.GPIO_Mode  = GPIO_Mode_IN;
	gpio_init.GPIO_OType = GPIO_OType_OD;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = RAIN_PIN;
	GPIO_Init(RAIN_GPIO_PORT, &gpio_init);                                    
}

/*
*********************************************************************************************************
*                                             BSP_Rain_State()
*
* Description : 获取雨滴传感器状态
*
* Argument(s) : none
*
* Return(s)   : Raining表示下雨, No_Rain表示未下雨
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
Rain_Status  BSP_Rain_State (void)
{
	uint8_t     count;
	OS_ERR      err;                 
	
	if (0 == RAIN_PIN_RD()) {
		
		// 休眠一段时间，防止误判
		
		for (count = 0; count < 10; count++) {
			OSTimeDlyHMSM( 0, 0, 0, 4,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
			if (0 != RAIN_PIN_RD()) 
				break;
		}	
	}

	if (10 == count)
		return Raining;
	
	return No_Rain;
}


