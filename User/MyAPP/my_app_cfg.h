
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

#define          LOCAL_SETTING_TASK_PRIO                  8u

#define          NORMAL_WORKING_TASK_PRIO                 16u

#define          APP_TASK_TOUCH_PRIO                      17u

#define          APP_TASK_GUI_DEMO_PRIO                   18u

#define          APP_TASK_SENSOR_PRIO                      2u                    // 确保优先级最高

#define          APP_TASK_FIRE_PRIO                        4u                    // 确保优先级较高

#define	         APP_TASK_WIFI_PRIO                        6u

#define          APP_TASK_LED_PRIO                        20u

#define          APP_TASK_MOTOR_PRIO                      17u 

#define          APP_TASK_COLLECTION_PRIO                 17u                    // 传感器采集任务

#define          APP_TASK_HANDLE_PRIO                     17u                    // 控制自动开关灯窗操作

/*
*********************************************************************************************************
*                                            TASK STACK SIZES   32bit
*********************************************************************************************************
*/

#define          LOCAL_SETTING_TASK_STK_SIZE              256u              // 调用了get_info() / set_info()函数需要很大的堆栈

#define          APP_TASK_TOUCH_STK_SIZE                  128u

#define          APP_TASK_GUI_DEMO_STK_SIZE               512u

#define          APP_TASK_MOTOR_STK_SIZE                  128u
  
#define          APP_TASK_LED_STK_SIZE                    64u

#define          APP_TASK_WIFI_STK_SIZE                   256u              // 调用了get_info() / set_info()函数需要很大的堆栈

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




