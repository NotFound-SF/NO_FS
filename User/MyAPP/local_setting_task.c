
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

#include "my_app_cfg.h"

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

static  CPU_STK  LocalSettingTaskStk[LOCAL_SETTING_TASK_STK_SIZE];

static  CPU_STK  AppTaskGUIDemoStk[APP_TASK_GUI_DEMO_STK_SIZE];

static  CPU_STK  AppTaskLedStk[APP_TASK_LED_STK_SIZE];

static  CPU_STK  AppTaskWifiStk[APP_TASK_WIFI_STK_SIZE];



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
			    (OS_MSG_QTY  ) 5u,
			    (OS_TICK     ) 0u,
			    (void       *) 0,
			    (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			    (OS_ERR     *)&err);
			 
    if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskLed OK");
    }
		
		
	
	//创建WIFI任务
	
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
				(OS_MSG_QTY  ) 0,                                // 无内建消息队列
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

static uint8_t buf[180];

static  void  LocalSettingTask (void *p_arg)
{
	int i;
	OS_ERR            err;
	FLASH_DRIVER_MSG  *request = (FLASH_DRIVER_MSG *)buf;
	
	(void)p_arg;
	
	request ->option   = FLASH_OPTION_R;                       // 读操作
	request ->task_tcb = &LocalSettingTaskTCB;                 // 任务自身TCB
	request ->addr     = 0x00;                                 // 要读取的flash地址
	request ->len      = 160;                                  // 读取的字节长度
	
	while (DEF_ON) {
//		
//		BSP_FLASH_Drivet_Request(request);                     // 向Flash任务发起请求
//		
//		for (i = 0; i < 160; i++) {
//			BSP_UART_Printf(BSP_UART_ID_1, "%d\t", buf[sizeof(FLASH_DRIVER_MSG)+i]);
//		}
		
	
		OSTimeDlyHMSM( 0, 0, 6, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
}



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
	int len = 0;
	uint8_t data[60];
	uint8_t id = 0;
	OS_ERR  err;
	
	(void)p_arg;


	// 将wifi模块设置为服务器模式
	BSP_ESP8266_Server_Init();
	
	while(DEF_ON) {
		len = BSP_ESP8266_Server_Read(data, &id, 0);              // 阻塞等待重wifi模块读取数据        
		
		BSP_UART_Printf(BSP_UART_ID_1, "len: %d\tid: %d\t:%s\r\n", len, id, data);
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






