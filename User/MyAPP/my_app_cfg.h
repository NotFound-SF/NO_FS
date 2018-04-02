
#ifndef         __LOCAL_SETTING_TASK_H
#define         __LOCAL_SETTING_TASK_H

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f4xx.h"
#include "info_manager.h"

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define          LOCAL_SETTING_TASK_PRIO                  6u

#define          APP_TASK_TOUCH_PRIO                      7u

#define          APP_TASK_GUI_DEMO_PRIO                   8u

#define          APP_TASK_SENSOR_PRIO                      2u                    // ȷ�����ȼ����

#define          APP_TASK_FIRE_PRIO                        3u                    // ȷ�����ȼ��ϸ�

#define	         APP_TASK_WIFI_PRIO                        4u

#define          APP_TASK_LED_PRIO                        12u

#define          APP_TASK_BUZZER_PRIO                      5u

#define          APP_TASK_MOTOR_PRIO                      11u 

#define          APP_TASK_COLLECTION_PRIO                 9u                    // �������ɼ�����

#define          APP_TASK_HANDLE_PRIO                     10u                    // �����Զ����صƴ�����

/*
*********************************************************************************************************
*                                            TASK STACK SIZES   32bit
*********************************************************************************************************
*/

#define          LOCAL_SETTING_TASK_STK_SIZE              256u              // ������get_info() / set_info()������Ҫ�ܴ�Ķ�ջ

#define          APP_TASK_TOUCH_STK_SIZE                  128u

#define          APP_TASK_GUI_DEMO_STK_SIZE               512u

#define          APP_TASK_MOTOR_STK_SIZE                  128u
  
#define          APP_TASK_LED_STK_SIZE                    64u

#define          APP_TASK_BUZZER_STK_SIZE                 128u

#define          APP_TASK_WIFI_STK_SIZE                   128u              

#define          APP_TASK_SENSOR_STK_SIZE                 128u

#define          APP_TASK_COLLECTION_STK_SIZE             128u

#define          APP_TASK_HANDLE_STK_SIZE                 128u

#define          APP_TASK_FIRE_STK_SIZE                   128u






/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/





#endif         /* __LOCAL_SETTING_TASK_H */




