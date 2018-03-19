
#ifndef    __USER_INFO_H_
#define    __USER_INFO_H_


#include "info_manager.h"
#include "flash_driver.h"

/*
*********************************************************************************************************
*                                           Flash 使用规划
*********************************************************************************************************
*/

#define          USER_INFO_BASE                            0                              // 存取用户数据的首地址
#define          USER_INFO_SIZE                            (sizeof(InfoStruct))                      // 存取用户数据占用的空间
#define          USER_INFO_BUF_SIZE                        (sizeof(FLASH_DRIVER_MSG)+USER_INFO_SIZE) // 访问Flash需要的内存空间


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  get_user_info(InfoStruct *dat, OS_TCB *task_tcb);

void  save_user_info(InfoStruct *dat, OS_TCB *task_tcb);


#endif      // __USER_INFO_H_



