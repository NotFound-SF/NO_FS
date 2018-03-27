
/*
*********************************************************************************************************
*                         该文件创建一个任务专门负责app对单片机wifi，电话号码信息
*                        的设置，即将wifi模块设置为服务端解析app传过来的指令
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "info_manager.h"     
#include "my_app_cfg.h"
#include  "user_info.h"
#include  "stdlib.h"

/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   LocalSettingTaskTCB;

static  OS_TCB   AppTaskGUIDemoTCB;

static  OS_TCB   AppTaskWifiTCB;

static  OS_TCB   AppTaskLedTCB;

/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  *LocalSettingTaskStk;

static  CPU_STK  *AppTaskGUIDemoStk;

static  CPU_STK  *AppTaskLedStk;

static  CPU_STK  *AppTaskWifiStk;




/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskWifi      (void *p_arg);

static  void  AppTaskLed       (void *p_arg);

static  void  LocalSettingTask (void *p_arg);


/*
*********************************************************************************************************
*                                       LOCAL DEFINES
*********************************************************************************************************
*/

#define    SETTING_TASK_Q_SIZE     4                   // 设置线程的消息队列长度 

// APP --> MCU
#define    ADD_PHONE_NUM           0x63
#define    ADD_WIFI_INFO           0x60
#define    DEL_PHONE_NUM           0x61
#define    CLAEN_PHONE_NUM         0x62
#define    REQUEST                 0x12


typedef enum{
	Save_Option = 0x00,
	Get_Option = 0x01
}Option_Type;



/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_MEM                  MyPartition;

static  InfoStruct              *user_info;                                    // 使用该内存区域存放Flash中的用户数据

static  CPU_INT08U              *MyPartitionStorage;                           // 用于消息队列传递消息    

   


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        LocalSettingTaskCreate()
*
* Description : 创建本地设置任务
*
* Argument(s) : none
*
* Return(s)   : DEF_FAIL 表示创建任务失败
*             : DEF_OK   表示创建任务成功
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  LocalSettingTaskCreate (void)
{
	OS_ERR      err;
	
	// malloc申请内存空间
	
	user_info = (InfoStruct*)malloc(sizeof(InfoStruct));
	MyPartitionStorage = (CPU_INT08U*)malloc(SETTING_TASK_Q_SIZE*USER_INFO_SIZE*sizeof(CPU_INT08U));
	
	LocalSettingTaskStk = (CPU_STK*)malloc(LOCAL_SETTING_TASK_STK_SIZE * sizeof(CPU_STK));
	AppTaskGUIDemoStk = (CPU_STK*)malloc(APP_TASK_GUI_DEMO_STK_SIZE * sizeof(CPU_STK));
	AppTaskLedStk = (CPU_STK*)malloc(APP_TASK_LED_STK_SIZE * sizeof(CPU_STK));
	AppTaskWifiStk = (CPU_STK*)malloc(APP_TASK_WIFI_STK_SIZE * sizeof(CPU_STK));

	// 创建内存管理分区空间
	OSMemCreate(&MyPartition, 
				 "MyPartition", 
				 MyPartitionStorage, 
				 SETTING_TASK_Q_SIZE, 
				 USER_INFO_SIZE, 
				 &err);
	
	
	//创建应用任务,emwin的官方示例函数 GUIDEMO_Main
		
	OSTaskCreate((OS_TCB     *)&AppTaskGUIDemoTCB,                                        
				(CPU_CHAR   *)"GUI Demo Test", 									                     
				(OS_TASK_PTR ) SettingMainTask,									                        
				(void       *) 0,																
				(OS_PRIO     ) APP_TASK_GUI_DEMO_PRIO,					
				(CPU_STK    *)&AppTaskGUIDemoStk[0],						
				(CPU_STK_SIZE) APP_TASK_GUI_DEMO_STK_SIZE / 10,				
				(CPU_STK_SIZE) APP_TASK_GUI_DEMO_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//			BSP_UART_Printf(BSP_UART_ID_1, "MainTask OK");
	}
		
		
	 //创建子进程 LED子进程
	
    OSTaskCreate((OS_TCB     *)&AppTaskLedTCB,              /* Create the LED task                                 */
	  		    (CPU_CHAR   *)"App Task Led",
			    (OS_TASK_PTR ) AppTaskLed,
			    (void       *) 0,
			    (OS_PRIO     ) APP_TASK_LED_PRIO,
			    (CPU_STK    *)&AppTaskLedStk[0],
			    (CPU_STK_SIZE) APP_TASK_LED_STK_SIZE / 10,
			    (CPU_STK_SIZE) APP_TASK_LED_STK_SIZE,
			    (OS_MSG_QTY  ) 0u,
			    (OS_TICK     ) 0u,
			    (void       *) 0,
			    (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			    (OS_ERR     *)&err);
			 
    if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskLed OK");
    }
		
		
	//创建WIFI收发任务
	
	OSTaskCreate((OS_TCB     *)&AppTaskWifiTCB,                                        
				(CPU_CHAR   *)"Wifi", 									                     
				(OS_TASK_PTR ) AppTaskWifi,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_WIFI_PRIO,					
				(CPU_STK    *)&AppTaskWifiStk[0],						
				(CPU_STK_SIZE) APP_TASK_WIFI_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_WIFI_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	

	// 创建管理flash的线程
	
	OSTaskCreate((OS_TCB    *)&LocalSettingTaskTCB,              /* Create the Sensor task                                 */
				(CPU_CHAR   *)"Local setting Task",
				(OS_TASK_PTR ) LocalSettingTask,
				(void       *) 0,
				(OS_PRIO     ) LOCAL_SETTING_TASK_PRIO,
				(CPU_STK    *)&LocalSettingTaskStk[0],
				(CPU_STK_SIZE) LOCAL_SETTING_TASK_STK_SIZE / 10,
				(CPU_STK_SIZE) LOCAL_SETTING_TASK_STK_SIZE,
				(OS_MSG_QTY  ) SETTING_TASK_Q_SIZE,             // 内建消息队列长度
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err == OS_ERR_NONE) {
		
	}
	
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
*                                          Wifi TASK
*
* Description : 负责Wifi收发
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/
static  void  AppTaskWifi (void *p_arg)
{
	uint8_t len = 0;
	uint8_t data[60];
	uint8_t id = 0;
	uint8_t *msg;
	uint8_t *source = (uint8_t*)user_info;
	OS_ERR  err;
	uint16_t index;
	Option_Type opt;
	
	
	(void)p_arg;
	
	// 现将Flash中的数据读入内存
	get_user_info(user_info, &AppTaskWifiTCB); 
	
	
	// 将wifi模块设置为服务器模式
	BSP_ESP8266_Server_Init();
	
	while(DEF_ON) {

		len = BSP_ESP8266_Server_Read(data, &id, 0);              // 阻塞等待重wifi模块读取数据    
		
		if (len > 0) {
			// 解析APP端的数据信息
			switch (data[0]) {                                        // 首字节表示操作命令
				case ADD_PHONE_NUM:
					opt = Save_Option;                                // 表示一个写操作
					phone_add(user_info, data+2, data[1]);
					break;
				
				case ADD_WIFI_INFO:
					opt = Save_Option;                                // 表示一个写操作
					wifi_setSSID(user_info, data+3, data[1]);        // 添加wifi热点名
					wifi_setPWD(user_info, data+3+data[1], data[2]); // 添加wifi密码信息
					break;
				
				case DEL_PHONE_NUM:
					opt = Save_Option;                                // 表示一个写操作
					phone_del(user_info, data[1]);
					break;
				
				case CLAEN_PHONE_NUM:
					opt = Save_Option;                                // 表示一个写操作
					phone_clean(user_info);
					break;
				
				case REQUEST:                                         // 表示请求信息
					opt = Get_Option;                                 // 表示回复操作
					BSP_ESP8266_Server_Write(source, sizeof(InfoStruct), id);  
					break;
			}
			
			// 根据APP 请求类型做响应设置的操作
			
			if (Save_Option == opt)  {
				
				// 申请内存空间
		
				msg = (uint8_t*)OSMemGet(&MyPartition, &err);
				
				// 如果申请空间成功就执行设置操作
				
				if (err == OS_ERR_NONE) {
					
					// 拷贝数据到申请的内存空间
					
					for (index = 0; index < USER_INFO_SIZE; index++) {
						*(msg+index) = *(source+index);
					}
					
					// 向设置任务发布信号量
					
					OSTaskQPost((OS_TCB      *) &LocalSettingTaskTCB,           // 设置任务标号
								(void        *) msg,                            // 消息内容
								(OS_MSG_SIZE  ) USER_INFO_SIZE,                 // 消息长度
								(OS_OPT       ) OS_OPT_POST_FIFO,               // FIFO队列
								(OS_ERR      *) &err);
				} else {
					// 申请不到空间不做任何操作
				}
			} 

		}
		
	}
	
}



/*
*********************************************************************************************************
*                                          TASK
*
* Description : 
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskLed   (void *p_arg)
{
	OS_ERR          err;
	
	(void) p_arg;
		
	/* 显示测试 */
	

	while(DEF_ON) {
				
		BSP_LED_Toggle(3);
		
		OSTimeDlyHMSM( 0, 0, 0, 200,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}




/*
*********************************************************************************************************
*                                        LocalSettingTask()
*
* Description : 该任务专门负责解析APP端发送的数据以完成对单片机的设置
*
* Argument(s) : p_arg is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : NONE
*
* Caller(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  LocalSettingTask (void *p_arg)
{
	OS_ERR            err;
	OS_MSG_SIZE       msg_size;
	InfoStruct       *info_dat;
	
	(void)p_arg;
	
	while (DEF_ON) {
		
		// 永久的阻塞等待消息队列
		info_dat = OSTaskQPend((OS_TICK      ) 0,
							   (OS_OPT       ) OS_OPT_PEND_BLOCKING,
							   (OS_MSG_SIZE *) &msg_size,
							   (CPU_TS      *) NULL,
							   (OS_ERR      *) &err);
		
		// 将空间中的数据写入Flash,阻塞等待至写完成
		save_user_info(info_dat, &LocalSettingTaskTCB);
		
		// 释放消息队列的空间
		OSMemPut(&MyPartition,
				 info_dat, 
				 &err);
	}
	
}



