#ifndef   __BSP_RAIN_H
#define   __BSP_RAIN_H

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "stm32f4xx.h"


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


// ʹ��PF9��Ϊ���������Խ�������ģʽ
#define      RAIN_PIN             	      GPIO_Pin_2

#define      RAIN_GPIO_PORT               GPIOE
#define      RAIN_GPIO_CLK                RCC_AHB1Periph_GPIOE                // �˿�ʱ��                  

#define      RAIN_PIN_RD()	             (RAIN_GPIO_PORT->IDR & RAIN_PIN) 	  //��ȡIO��״̬����0���0		



typedef enum {
	Raining = 0x00,           // ��ʾ������  
	No_Rain = 0x01,           // ��ʾδ����
}Rain_Status;
      





/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void          BSP_Rain_Init                (void);

Rain_Status   BSP_Rain_State              (void);


#endif    //__BSP_RAIN_H

