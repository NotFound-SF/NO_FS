


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include"bsp_led.h"

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
*                                             BSP_LED_Init()
*
* Description : ��ʼ��LED�˿�����Ϊ�������ģʽ
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

void  BSP_LED_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	
	RCC_AHB1PeriphClockCmd(LED12_GPIO_CLK, ENABLE);    
	RCC_AHB1PeriphClockCmd(LED3_GPIO_CLK, ENABLE);  
	
	gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = LED1_PIN|LED2_PIN;
	GPIO_Init(LED12_GPIO_PORT, &gpio_init);

	gpio_init.GPIO_Pin   = LED3_PIN;
	GPIO_Init(LED3_GPIO_PORT, &gpio_init);
	
	BSP_LED_Off(0);                                       /* ������ȷ������LED���ر�                          */                                    
}

/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : ����LED
*
* Argument(s) : led    led��ʾΪҪ����led�ı��[0-8]:
*
*                      0   ��������LED
*                      1   ����LED1
*                      2   ����LED2
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_LED_On (uint8_t led)
{
	switch (led) {
		
		case 0:
			GPIO_ResetBits(LED12_GPIO_PORT, LED1_PIN|LED2_PIN);
			GPIO_ResetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		case 1:
			GPIO_ResetBits(LED12_GPIO_PORT, LED1_PIN);
			break;
		
		case 2:
			GPIO_ResetBits(LED12_GPIO_PORT, LED2_PIN);
			break;
		
		case 3:
			GPIO_ResetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		default:
			break;
	}
}


/*
*********************************************************************************************************
*                                             BSP_LED_Off()
*
* Description : �ر�LED
*
* Argument(s) : led    led��ʾΪҪ�ر�led�ı��[0-8]:
*
*                      0   �ر�����LED
*                      1   �ر�LED1
*                      2   �ر�LED2
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Off(uint8_t led)
{
	switch (led) {
		case 0:
			GPIO_SetBits(LED12_GPIO_PORT, LED1_PIN|LED2_PIN);
			GPIO_SetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		case 1:
			GPIO_SetBits(LED12_GPIO_PORT, LED1_PIN);
			break;
		
		case 2:
			GPIO_SetBits(LED12_GPIO_PORT, LED2_PIN);
			break;
		
		case 3:
			GPIO_SetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		default:
			break;
	}
	
}


/*
*********************************************************************************************************
*                                             BSP_LED_Toggle()
*
* Description : �ı�LED״̬
*
* Argument(s) : led    led��ʾΪҪ�ı�״̬led�ı��[0-8]:
*
*                      0   �ı�����LED
*                      1   �ı�LED1
*                      2   �ı�LED2
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_LED_Toggle(uint8_t led)
{
	uint32_t pins_12, pins_3;
	
	pins_12 = GPIO_ReadOutputData(LED12_GPIO_PORT);                               /* �����˿ڵ�ǰ�������                      */
	pins_3  = GPIO_ReadOutputData(LED3_GPIO_PORT); 
	
	switch (led) {
		case 0:
			GPIO_ResetBits(LED12_GPIO_PORT, pins_12 & (LED1_PIN|LED2_PIN));       /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			GPIO_SetBits(LED12_GPIO_PORT, (~pins_12) & (LED1_PIN|LED2_PIN));      /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			
			GPIO_ResetBits(LED3_GPIO_PORT, pins_3 & LED3_PIN);                    /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			GPIO_SetBits(LED3_GPIO_PORT, (~pins_3) & LED3_PIN);                   /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			break;
		
		case 1:
			GPIO_ResetBits(LED12_GPIO_PORT, pins_12 & LED1_PIN);                  /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			GPIO_SetBits(LED12_GPIO_PORT, (~pins_12) & LED1_PIN); 
			break;
		
		case 2:
			GPIO_ResetBits(LED12_GPIO_PORT, pins_12 & LED2_PIN);                  /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			GPIO_SetBits(LED12_GPIO_PORT, (~pins_12) & LED2_PIN); 
			break;
		
		case 3:
			GPIO_ResetBits(LED3_GPIO_PORT, pins_3 & LED3_PIN);                    /* ֻ�޸�LED���ڹܽţ����޸��޹عܽ�          */
			GPIO_SetBits(LED3_GPIO_PORT, (~pins_3) & LED3_PIN); 
			break;
		
		default:
			break;
	}
}
	

