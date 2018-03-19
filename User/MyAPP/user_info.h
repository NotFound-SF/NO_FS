
#ifndef    __USER_INFO_H_
#define    __USER_INFO_H_


#include "info_manager.h"
#include "flash_driver.h"

/*
*********************************************************************************************************
*                                           Flash ʹ�ù滮
*********************************************************************************************************
*/

#define          USER_INFO_BASE                            0                              // ��ȡ�û����ݵ��׵�ַ
#define          USER_INFO_SIZE                            (sizeof(InfoStruct))                      // ��ȡ�û�����ռ�õĿռ�
#define          USER_INFO_BUF_SIZE                        (sizeof(FLASH_DRIVER_MSG)+USER_INFO_SIZE) // ����Flash��Ҫ���ڴ�ռ�


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  get_user_info(InfoStruct *dat, OS_TCB *task_tcb);

void  save_user_info(InfoStruct *dat, OS_TCB *task_tcb);


#endif      // __USER_INFO_H_



