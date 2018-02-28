
#ifndef     __BSP_TIMING_H
#define     __BSP_TIMING_H

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "stm32f4xx.h"
#include "includes.h"


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define         BSP_TIMING_NVIC_SUB_PRIO                   1                                //�ö�ʱ�������ж��붨ʱ��1���� 
#define         BSP_TIMING_NVIC_PRE_PRIO                   1                                //�ö�ʱ�������ж��붨ʱ��1����


#define         TIMING_TYPE                                TIM2
#define         TIMING_PERIPH_RCC                          RCC_APB1Periph_TIM2
#define         TIMING_PERIPH_RCC_CMD                      RCC_APB1PeriphClockCmd

#define         TIMING_PERIOD_US                           (uint16_t)0x14                   //����ʱ��Ϊ84/(20+1)==4Mhz
#define         TIMING_PERIOD_MS                           (uint16_t)0x5207                 //���൱������ʱ��Ϊ4Khz
#define         TIMING_BASE_SHIFT                          0x02                             //�����ó˷�����ʹ����λ
#define         TIMING_UPDATE_MASK                         (uint16_t)0x01
#define         TIMING_NO_SCHED_MAX_US                     0x10                             //���볬����ֵ��������ȣ�16us��

#define         BSP_TIMING_IRQ                             TIM2_IRQn                        //�ö�ʱ�������ж��붨ʱ��1����
#define         Timing_Handler                             TIM2_IRQHandler                  //�жϴ�����λ��


CPU_BOOLEAN  BSP_Timing_Init (void);
void         Timing_Unlock   (void);
void         Timing_Lock     (void);
void         Timing_Delay_Ms (uint16_t count);
void         Timing_Delay_Us (uint16_t count);




#endif     /* __BSP_TIMING_H */





