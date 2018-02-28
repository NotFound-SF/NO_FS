
/*
*********************************************************************************************************
*                          该文件创建一个线程专门负责flash读写，所以其他线程要操作
*                        flash都必须向该线程发送消息，消息格式参见头文件
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "flash_driver.h"
#include "bsp_w25q128.h"


/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   DriverTaskFlashTCB;

/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  DriverTaskFlashStk[DRIVER_TASK_FLASH_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void     FlashDriverTask (void *p_arg);




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_FLASH_Drivet_Init()
*
* Description : 初始板载flash芯片并且创建管理flash读写的驱动线程.
*
* Argument(s) : none
*
* Return(s)   : DEF_FAIL 表示创建任务失败
*             : DEF_OK   表示创建任务成功
*
* Caller(s)   : BSP_Init();
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_FLASH_Drivet_Init (void)
{
	OS_ERR      err;
	// flash底层初始化
	BSP_FLASH_Init();
	
	
	// 创建管理flash的线程
	
	OSTaskCreate((OS_TCB    *)&DriverTaskFlashTCB,              /* Create the Sensor task                                 */
				(CPU_CHAR   *)"Flash Driver Task",
				(OS_TASK_PTR ) FlashDriverTask,
				(void       *) 0,
				(OS_PRIO     ) DRIVER_TASK_FLASH_PRIO,
				(CPU_STK    *)&DriverTaskFlashStk[0],
				(CPU_STK_SIZE) DRIVER_TASK_FLASH_STK_SIZE / 10,
				(CPU_STK_SIZE) DRIVER_TASK_FLASH_STK_SIZE,
				(OS_MSG_QTY  ) FLASH_DRIVER_Q_SIZE,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err == OS_ERR_NONE) {
		return DEF_OK;
	}
	
	return DEF_FAIL;
}



/*
*********************************************************************************************************
*                                        BSP_FLASH_Drivet_Request()
*
* Description : 该函数由其他线程调用，用于请求驱动任务根据消息内容执行相应的读写操作,驱动任务执行完毕后方才返回
*
* Argument(s) : request 该参数为指针，指向一片连续的内存区域
*
* Return(s)   : DEF_FAIL 表示创建任务失败
*             : DEF_OK   表示创建任务成功
*
* Caller(s)   : 其他线程;
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   BSP_FLASH_Drivet_Request (FLASH_DRIVER_MSG *request)
{
	OS_ERR   err;
	
	// 向驱动线程的内建消息队列发送消息
	OSTaskQPost((OS_TCB      *) &DriverTaskFlashTCB,            // 驱动任务标号
				(void        *) request,                        // 消息内容
				(OS_MSG_SIZE  ) sizeof(FLASH_DRIVER_MSG),       // 消息长度
				(OS_OPT       ) OS_OPT_POST_FIFO,               // FIFO队列
				(OS_ERR      *) &err);
	
	// 阻塞等待驱动任务执行完该次读写操作，由任务信号量实现
	OSTaskSemPend((OS_TICK      ) 0,                            // 永久阻塞
				  (OS_OPT       ) OS_OPT_PEND_BLOCKING,         // 阻塞等待
				  (CPU_TS      *) NULL,                         // 不需要时间戳
				  (OS_ERR      *) &err);                        // 错误标志
}




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        FlashDriverTask()
*
* Description : 该任务专门负责管理flash读写操作
*
* Argument(s) : p_arg is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : NONE
*
* Caller(s)   : BSP_FLASH_Drivet_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  FlashDriverTask (void *p_arg)
{
	OS_ERR            err;
	uint16_t          dat_len;
	OS_MSG_SIZE       msg_size;
	FLASH_DRIVER_MSG *request;
	
	(void)p_arg;
	
	while (DEF_ON) {
		
		// 阻塞等待其他线程发来的请求，以执行相应的操作
		request = OSTaskQPend((OS_TICK      ) 0,
							  (OS_OPT       ) OS_OPT_PEND_BLOCKING,
							  (OS_MSG_SIZE *) &msg_size,
							  (CPU_TS      *) NULL,
							  (OS_ERR      *) &err);
		
		if (OS_ERR_NONE != err)
			continue;
		
		// 根据接收的到的请求执行相应的操作
		dat_len = request->len;
		
		if (FLASH_OPTION_R == request -> option) {                     // 表示读操作
			BSP_FLASH_BufferRead(((uint8_t*)request)+sizeof(FLASH_DRIVER_MSG),
								 request->addr, dat_len);
		} else if (FLASH_OPTION_W == request -> option) {              // 表示写操作
			BSP_FLASH_SectorErase(request->addr/4096);                 // 编程前需要对写区域执行擦除操作
			BSP_FLASH_BufferWrite(((uint8_t*)request)+sizeof(FLASH_DRIVER_MSG),
								  request->addr, dat_len);
		}
		
		
		// 接触请求任务阻塞
		OSTaskSemPost(request->task_tcb,
					  OS_OPT_POST_NONE,                                // 其实不调度也可以因为while开始会任务调度
					  &err);
	}
	
}




