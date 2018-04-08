#ifndef   __BSP_LIGHT_H
#define   __BSP_LIGHT_H

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


#define      HIGH_LIGHT_VALUE                600

#define      LIGHT_PIN             	         GPIO_Pin_2
#define      LIGHT_GPIO_CLK                  RCC_AHB1Periph_GPIOC              /* ¶Ë¿ÚÊ±ÖÓ                  */
#define      LIGHT_GPIO_PORT                 GPIOC




typedef enum {
	Light_High_Level = 0x00,
	Light_Normal_Level = 0x01
} Light_Level;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         BSP_LIGHT_Init   (void);

void         BSP_LIGHT_On     (void);

void         BSP_LIGHT_Off    (void);

void         BSP_LIGHT_Toggle (void);

Light_Level  BSP_Light_State  (void);




#endif    //__BSP_LIGHT_H
