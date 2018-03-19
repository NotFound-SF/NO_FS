
/*
*********************************************************************************************************
*                          该文件提供重flash存取用户信息的方法
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "user_info.h"


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        save_user_info()
*
* Description : 将内存结构中的数据写入到flash中，写结束才返回
*
* Argument(s) : dat:重Flash中读取的数据将被放入Dat
*             : task_tcb：调用该函数的任务TCB针对，用于阻塞该任务
*
* Return(s)   : none
*
* Caller(s)   : APP
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  save_user_info(InfoStruct *dat, OS_TCB *task_tcb)
{
	uint8_t            buf[USER_INFO_BUF_SIZE];               // 用于存放头信息和数据的空间 
	uint16_t           index;
	uint8_t           *dest = buf+sizeof(FLASH_DRIVER_MSG);   // 偏移过头信息
	uint8_t           *source = (uint8_t *)dat;
	FLASH_DRIVER_MSG  *request = (FLASH_DRIVER_MSG *)buf;
	
	// 初始化请求的数据头信息
	request ->option   = FLASH_OPTION_W;                       // 写操作
	request ->task_tcb = task_tcb;                             // 调用者的TCB
	request ->addr     = USER_INFO_BASE;                       // 要读取的flash地址
	request ->len      = USER_INFO_SIZE;                       // 读取的字节长度
	
	// 填充真实消息体
	for (index = 0; index < sizeof(InfoStruct); index++) {
		*(dest+index) = *(source+index);                       
	}
	
	BSP_FLASH_Drivet_Request(request);                         // 向Flash任务发起请求,该任务会被阻塞
}



/*
*********************************************************************************************************
*                                        get_user_info()
*
* Description : 将内flash中的用户数据读入内存，读取结束后才返回
*
* Argument(s) : dat:重Flash中读取的数据将被放入Dat
*             : task_tcb：调用该函数的任务TCB针对，用于阻塞该任务
*
* Return(s)   : none
*
* Caller(s)   : APP
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  get_user_info(InfoStruct *dat, OS_TCB *task_tcb)
{
	uint16_t           index;
	uint8_t            buf[USER_INFO_BUF_SIZE];                // 用于存放头信息和数据的空间 
	uint8_t           *dest = (uint8_t *)dat;                  // 目标地址
	uint8_t           *source = buf+sizeof(FLASH_DRIVER_MSG);  // 偏移过头信息，的原地址
	FLASH_DRIVER_MSG  *request = (FLASH_DRIVER_MSG *)buf;
	
	// 初始化请求的数据头信息
	request ->option   = FLASH_OPTION_R;                       // 读操作
	request ->task_tcb = task_tcb;                             // 调用者的TCB
	request ->addr     = USER_INFO_BASE;                       // 要读取的flash地址
	request ->len      = USER_INFO_SIZE;                       // 读取的字节长度
	
	BSP_FLASH_Drivet_Request(request);                         // 向Flash任务发起请求,该任务会被阻塞
	
	// 填充真实消息体
	for (index = 0; index < sizeof(InfoStruct); index++) {
		*(dest+index) = *(source+index);                       
	}
	
}








