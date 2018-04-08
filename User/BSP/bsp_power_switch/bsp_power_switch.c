

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include"bsp_power_switch.h"

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

typedef enum {
	Power_OFF = 0x00,
	Power_ON = 0x01
} Power_State;


static Power_State  power_state = Power_ON;             // 必须设置为该状态，因为初始化函数，调用了OFF函数


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_POWER_SWITCH_Init()
*
* Description : 初始化继电器控制端口引脚为推完输出模式
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

void  BSP_POWER_SWITCH_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	
	RCC_AHB1PeriphClockCmd(POWER_SWITCH_CLK, ENABLE);     
	
	gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_OD;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = POWER_SWITCH_PIN;
	GPIO_Init(POWER_SWITCH_GPIO_PORT, &gpio_init);
	
	BSP_POWER_SWITCH_Off();                                       /* 开机后确保所有电源关闭                          */                                    
}

/*
*********************************************************************************************************
*                                             BSP_POWER_SWITCH_On()
*
* Description : 开启继电器开关
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

void  BSP_POWER_SWITCH_On (void)
{
	if (Power_ON != power_state) {
		GPIO_ResetBits(POWER_SWITCH_GPIO_PORT, POWER_SWITCH_PIN);
		power_state = Power_ON;
	}
}


/*
*********************************************************************************************************
*                                             BSP_POWER_SWITCH_Off()
*
* Description : 关闭继电器开关
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

void  BSP_POWER_SWITCH_Off(void)
{
	if (Power_OFF != power_state) {
		GPIO_SetBits(POWER_SWITCH_GPIO_PORT, POWER_SWITCH_PIN);
		power_state = Power_OFF;
	}
}


