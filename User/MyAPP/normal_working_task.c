/*
*********************************************************************************************************
*                                      该文件使单片机工作在常规模式
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include "DIALOG.h"
#include "includes.h"
#include "bsp_power_switch.h" 
#include "bsp_light.h"
#include "bsp_mq2.h"
#include "bsp_rain.h"
#include "my_app_cfg.h"
#include  "user_info.h"
#include  "stdlib.h"
#include  "string.h"
#include  "bsp_ga6.h"
#include  "bsp_buzzer.h"


extern  OS_TCB   AppTaskStartTCB;                                           //开始任务任务控制块


/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   AppTaskGUIDemoTCB;

static  OS_TCB   AppTaskTouchTCB;

static  OS_TCB   AppTaskLedTCB;

static  OS_TCB   AppTaskSensorTCB;   

static  OS_TCB   AppTaskMotorTCB;

static  OS_TCB   AppTaskWifiTCB;

static  OS_TCB   AppTaskHandleTCB;

static  OS_TCB   AppTaskCollectionTCB;

static  OS_TCB   AppTaskFireTCB;

static  OS_TCB   AppTaskBuzzerTCB;




/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  *AppTaskGUIDemoStk;                       // 该空间在堆上分配

static  CPU_STK  *AppTaskTouchStk;                         // 该空间在堆上分配

static  CPU_STK  *AppTaskWifiStk;                          // 该空间在堆上分配           

static  CPU_STK  AppTaskLedStk[APP_TASK_LED_STK_SIZE];

static  CPU_STK  AppTaskMotorStk[APP_TASK_MOTOR_STK_SIZE];

static  CPU_STK  AppTaskSensorStk[APP_TASK_SENSOR_STK_SIZE];

static  CPU_STK  AppTaskHandleStk[APP_TASK_HANDLE_STK_SIZE];

static  CPU_STK  AppTaskFireStk[APP_TASK_FIRE_STK_SIZE];

static  CPU_STK  AppTaskBuzzerStk[APP_TASK_BUZZER_STK_SIZE];

static  CPU_STK  AppTaskCollectionStk[APP_TASK_COLLECTION_STK_SIZE];






/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskSensor     (void *p_arg);

static  void  AppTaskLed        (void *p_arg);

static  void  AppTaskTouch      (void *p_arg);

static  void  AppTaskMotor      (void *p_arg);

static  void  AppTaskWifi       (void *p_arg);

static  void  AppTaskHandle     (void *p_arg);

static  void  AppTaskFire       (void *p_arg);

static  void  AppTaskCollectin  (void *p_arg);

static  void  AppTaskBuzzer     (void *p_arg);



/*
*********************************************************************************************************
*                                       LOCAL DEFINES
*********************************************************************************************************
*/

#define     DEVICE_ID                     12345         // 硬件设备唯一ID
#define     ALIVE_TIME_OUT                2000          // 单位为时钟节拍周期
 





/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/


// 所有的状态0x0F表示该状态成立，0x00表示状态不成立

typedef struct {
	uint32_t user_id;                      // 用户ID
	uint32_t device_id;                    // 设备ID
	uint8_t  operation;                    // 操作方式，0x0F表示设置，0x00表示请求
	uint8_t  warning_flag;                 // 警告标志，0x0F表示发生火灾，0x00表示未发生
	uint8_t  led_switch;                   // led灯开关标志，0x0F表示开灯，0x00表示关灯
	uint8_t  led_auto;                     // led灯光控标志，0x0F表示光控，0x00表示手控
	uint8_t  power_switch;                 // 电源开关，0x0F表示打开继电器，0x00表示关闭继电器
	uint8_t  rain_status;                  // 下雨状态，0x0F表示下雨，0x00表示天晴
	uint8_t  window;                       // 开关窗角度
	uint8_t  window_auto;                  // 下雨自动关窗，0x0F表示自动关窗，0x00表示不自动关窗
	float    temperature;                  // 温度
	float    current;                      // 电流
} Local_status;




static       OS_SEM               StatusSemLock;          // 用于独占访问状态全局变量  

static       Local_status         local_status;           // 全局状态

static       uint8_t              server_flag = 0x00;     // 服务器的连接状态         

static       InfoStruct           *user_info;             // 使用该内存区域存放Flash中的用户数据

static       Light_Level          light_state;            // 供给传感器采集任务和处理任务共享，光照强度水平

static       uint8_t              fire_flag = 0;          // 如果火灾被处理就会被置位一



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        NormalWorkingTaskCreate()
*
* Description : 创建常规模式任务
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


CPU_BOOLEAN  NormalWorkingTaskCreate (void)
{
	OS_ERR      err;
	
	// malloc申请内存空间
	
	user_info = (InfoStruct*)malloc(sizeof(InfoStruct));
	AppTaskGUIDemoStk = (CPU_STK*)malloc(APP_TASK_GUI_DEMO_STK_SIZE * sizeof(CPU_STK));
	AppTaskTouchStk = (CPU_STK*)malloc(APP_TASK_TOUCH_STK_SIZE * sizeof(CPU_STK));
	AppTaskWifiStk = (CPU_STK*)malloc(APP_TASK_WIFI_STK_SIZE * sizeof(CPU_STK));
	
	
	// 创建独占信号量
	
	OSSemCreate((OS_SEM    *)&StatusSemLock,                                 //创建互斥信号量
                (CPU_CHAR  *)"Status SemLock", 
	            (OS_SEM_CTR ) 1,                                             // 互斥信号量                             
	            (OS_ERR    *)&err); 
	
	// 做简单的初始化
	memset(&local_status, 0x00, sizeof(Local_status));
	
	// 先将Flash中的数据读入内存
	get_user_info(user_info, &AppTaskStartTCB); 
	
	
	//创建应用任务,emwin的官方示例函数 GUIDEMO_Main
		
	OSTaskCreate((OS_TCB     *)&AppTaskGUIDemoTCB,                                        
				(CPU_CHAR   *)"GUI Normal", 									                     
				(OS_TASK_PTR ) MainTask,									                        
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
	
	//创建触摸屏检测线程

	OSTaskCreate((OS_TCB     *)&AppTaskTouchTCB,              /* Create the LED1 task                                 */
				(CPU_CHAR   *)"App Task Touch",
				(OS_TASK_PTR ) AppTaskTouch,
				(void       *) 0,
				(OS_PRIO     ) APP_TASK_TOUCH_PRIO,
				(CPU_STK    *)&AppTaskTouchStk[0],
				(CPU_STK_SIZE) APP_TASK_TOUCH_STK_SIZE / 10,
				(CPU_STK_SIZE) APP_TASK_TOUCH_STK_SIZE,
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err==OS_ERR_NONE) {
//			BSP_UART_Printf(BSP_UART_ID_1, "AppTaskTouch OK");
	}
	

	//创建子进程 SENSOR子进程

	OSTaskCreate((OS_TCB     *)&AppTaskSensorTCB,              /* Create the Sensor task                                 */
				(CPU_CHAR   *)"App Task SENSOR",
				(OS_TASK_PTR ) AppTaskSensor,
				(void       *) 0,
				(OS_PRIO     ) APP_TASK_SENSOR_PRIO,
				(CPU_STK    *)&AppTaskSensorStk[0],
				(CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE / 10,
				(CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE,
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskSensor OK");
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
	
	
	 //创建蜂鸣器子进程
	
    OSTaskCreate((OS_TCB     *)&AppTaskBuzzerTCB,              /* Create the LED task                                 */
	  		    (CPU_CHAR   *)"App Task Buzzer",
			    (OS_TASK_PTR ) AppTaskBuzzer,
			    (void       *) 0,
			    (OS_PRIO     ) APP_TASK_BUZZER_PRIO,
			    (CPU_STK    *)&AppTaskBuzzerStk[0],
			    (CPU_STK_SIZE) APP_TASK_BUZZER_STK_SIZE / 10,
			    (CPU_STK_SIZE) APP_TASK_BUZZER_STK_SIZE,
			    (OS_MSG_QTY  ) 0u,
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
	
	//创建传感器数据采集任务
	
	OSTaskCreate((OS_TCB     *)&AppTaskCollectionTCB,                                        
				(CPU_CHAR   *)"Collection", 									                     
				(OS_TASK_PTR ) AppTaskCollectin,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_COLLECTION_PRIO,					
				(CPU_STK    *)&AppTaskCollectionStk[0],						
				(CPU_STK_SIZE) APP_TASK_COLLECTION_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_COLLECTION_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
	
	//创建开关灯窗的处理任务任务
	
	OSTaskCreate((OS_TCB     *)&AppTaskHandleTCB,                                        
				(CPU_CHAR   *)"Handle", 									                     
				(OS_TASK_PTR ) AppTaskHandle,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_HANDLE_PRIO,					
				(CPU_STK    *)&AppTaskHandleStk[0],						
				(CPU_STK_SIZE) APP_TASK_HANDLE_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_HANDLE_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
	
	
	//创建处理火灾的任务
	
	OSTaskCreate((OS_TCB     *)&AppTaskFireTCB,                                        
				(CPU_CHAR   *)"Fire", 									                     
				(OS_TASK_PTR ) AppTaskFire,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_FIRE_PRIO,					
				(CPU_STK    *)&AppTaskFireStk[0],						
				(CPU_STK_SIZE) APP_TASK_FIRE_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_FIRE_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
		
	//创建步进电机任务
	
	OSTaskCreate((OS_TCB     *)&AppTaskMotorTCB,                                        
				(CPU_CHAR   *)"Motor", 									                     
				(OS_TASK_PTR ) AppTaskMotor,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_MOTOR_PRIO,					
				(CPU_STK    *)&AppTaskMotorStk[0],						
				(CPU_STK_SIZE) APP_TASK_MOTOR_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_MOTOR_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskMotor OK");
    }
	
	return DEF_OK;
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
	uint8_t  ssid[31], pwd[31];
	uint8_t  wifi_status = 0x00;
	uint8_t  dat_len = 0;
	uint32_t temp_id;
	OS_ERR   err;
	Local_status request;                     // 保存APP端发送的数据
	
	(void)p_arg;
	
	// 设置为连接模式
	
	BSP_ESP8266_Client_Init();
	
	dat_len = wifi_getSSID(user_info, ssid);   // 获取热点名字
	
	if (0 == dat_len) {                         // 永久阻塞,因为不会有任务向该线程发送信号量
		OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &err);
	}
	
	wifi_getPWD(user_info, pwd);               // 获取热点密码
	
	// 开始连接
	wifi_status = BSP_ESP8266_WIFI_connect((char*)ssid, (char*)pwd);
	
	while (0x00 == wifi_status) {
		wifi_status = BSP_ESP8266_WIFI_connect((char*)ssid, (char*)pwd);
		
		OSTimeDlyHMSM( 0, 0, 4, 0,              // 线程休眠一段时间
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
	// 连接到服务器
	server_flag = BSP_ESP8266_connect_server();
	
	while (0x00 == server_flag) {
		server_flag = BSP_ESP8266_connect_server();
		
		OSTimeDlyHMSM( 0, 0, 6, 0,              // 线程休眠一段时间
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
	// 表明连接到服务器成功
	
	while(DEF_ON) {
		dat_len = BSP_ESP8266_Client_Read((uint8_t*)(&request), ALIVE_TIME_OUT);    // 读取APP端的数据
		
		// 表示真实的受到服务器请求
		
		if (sizeof(request) == dat_len) {                
			
			if (0x0F == request.operation) {                                       // 表示APP要设置硬件端的数据
			
				// 设置本地状态时必须加锁
				
				OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
				local_status.window = request.window;
				local_status.led_auto = request.led_auto;
				local_status.led_switch = request.led_switch;
				local_status.window_auto = request.window_auto;
				local_status.power_switch = request.power_switch;
				local_status.warning_flag = request.warning_flag;
				OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
			
			} else if (0x00 == request.operation) {                               // 表示APP要请求数据
			
				// 回传本地数据
				
				OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
				request.window = local_status.window;
				request.led_auto = local_status.led_auto;
				request.led_switch = local_status.led_switch;
				request.window_auto = local_status.window_auto;
				request.current     = local_status.current;
				request.rain_status = local_status.rain_status;
				request.temperature = local_status.temperature;
				request.power_switch = local_status.power_switch;
				request.warning_flag = local_status.warning_flag;
				OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
				
				// 根据接受到的数据翻转ID
				
				temp_id = request.device_id;
				request.device_id = request.user_id;
				request.user_id = temp_id;
				
				// 发送request到服务器
				
				BSP_ESP8266_Client_Write((uint8_t*)(&request), sizeof(request));
			}			
			
		} else if (0 == dat_len) {                                              // 表示超时返回
			
			// 发送存活信号
			
			request.user_id = DEVICE_ID;
			request.device_id = 0x00;                                           // 表示为脉搏信号
			
			BSP_ESP8266_Client_Write((uint8_t*)(&request), sizeof(request));   
		}

	}
	
}





/*
*********************************************************************************************************
*                                          SENSOR TASK
*
* Description : 该任务负责读取各个传感器数据，必须保证该任务优先级最高，只有这样才能保证任务能都产生正常的时序
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskSensor (void *p_arg)
{
	OS_ERR     err;

	(void)p_arg;	

	while (DEF_ON) {
					
		OSTimeDlyHMSM( 0, 0, 1, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );

		if(DEF_OK == BSP_18B20_GetTemp()) {
			//BSP_UART_Printf(BSP_UART_ID_1, "Temp: %.4f\n", BSP_18B20_TempTran(temp));
		}
	}
}





/*
*********************************************************************************************************
*                                          Touch TASK
*
* Description : 触摸屏更新任务
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/
static  void  AppTaskTouch (void *p_arg)
{
	OS_ERR  err;
	
	(void)p_arg;

	while(DEF_ON) {
		OSTimeDlyHMSM( 0, 0, 0, 10,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
		
		 GUI_TOUCH_Exec();                     // 更细触摸屏数据
	}
	
}



/*
*********************************************************************************************************
*                                         LED TASK
*
* Description : 用来监视系统工作正常
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
		
		OSTimeDlyHMSM( 0, 0, 0, 500,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}




/*
*********************************************************************************************************
*                                          Collectin TASK
*
* Description : 传感器采集任务，负责采集天气，电流，光照，烟雾
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/


static  void  AppTaskCollectin  (void *p_arg)
{
	OS_ERR          err;
	Local_status    temp_state;                     // 将采集的数据先暂存在该变量中，以减小枷锁粒度
	
	(void) p_arg;
		

	while(DEF_ON) {
		
		// 更新下雨状态
		
		if (Raining == BSP_Rain_State())
			temp_state.rain_status = 0x0F;
		else 
			temp_state.rain_status = 0x00;
		
		// 更新烟雾报警器状态
		
		if (MQ2_High_Level == BSP_MQ2_State()) 
			temp_state.warning_flag = 0x0F;
		else 
			temp_state.warning_flag = 0x00;
		
		// 更新光照强度状态
		
		light_state = BSP_Light_State();
		
		// 获取电流强度
		
		temp_state.current = BSP_ACS_GetU_Real();
		
		// 获取温度，从缓冲区获取，所以非常快速
		temp_state.temperature = BSP_18B20_TempTran(BSP_18B20_GetTempFast());
		
		
		// 将临时变量中的数据拷贝到全局状态变量
		
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
		local_status.current = temp_state.current;
		local_status.rain_status = temp_state.rain_status;
		local_status.temperature = temp_state.temperature;
		
		// 只允许传感器触发报警，而不允许传感器撤销报警，撤销报警只能由LCD和APP操作
		
		if (0x0F == temp_state.warning_flag)                            
			local_status.warning_flag = temp_state.warning_flag;
		
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
		
		// 没间隔一段时间获取一次传感器数据
		
		OSTimeDlyHMSM( 0, 0, 0, 500,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}



/*
*********************************************************************************************************
*                                          Handle TASK
*
* Description : 负责处理开关灯，开关窗操作
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskHandle     (void *p_arg)
{
	OS_ERR          err;
	Local_status    temp_state;                     // 获取全局的状态信息，减少加锁粒度
	
	(void) p_arg;
		
	/* 显示测试 */
	

	while(DEF_ON) {
		
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
		temp_state.window = local_status.window;
		temp_state.led_auto = local_status.led_auto;
		temp_state.led_switch = local_status.led_switch;
		temp_state.window_auto = local_status.window_auto;
		temp_state.current     = local_status.current;
		temp_state.rain_status = local_status.rain_status;
		temp_state.temperature = local_status.temperature;
		temp_state.power_switch = local_status.power_switch;
		temp_state.warning_flag = local_status.warning_flag;
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
		
		// 处理自动照明操作
		
		if (0x0F == temp_state.led_switch) {                              // 如果照明总开关为开
			
			if (0x0F == temp_state.led_auto) {
				if (Light_High_Level == light_state)
					BSP_LIGHT_Off();
				else 
					BSP_LIGHT_On();
					
			} else {                                                      // 开灯
                BSP_LIGHT_On();                 				
			}
			
		} else {                                                          // 关灯
 			BSP_LIGHT_Off();
		}
		
		// 处理自动关窗操作
		
		if (0x0F == temp_state.window_auto) {
			
			if (0x0F == temp_state.rain_status) {
				OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
				local_status.window = 0x00;
				OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
			}
			
		}
		
		// 处理电源总开关操作
		
		if (0x0F == temp_state.power_switch) {
			BSP_POWER_SWITCH_On();
			
		} else {
			BSP_POWER_SWITCH_Off();
		}
		
		// 处理火灾报警操作，当发生火灾报警就想火灾报警任务发生信号量
		
		if (0x0F == temp_state.warning_flag && 0x00 == fire_flag) {
			fire_flag = 0x01;                                                    // 非常重要，防止传感器任务重复发送信号量 
			OSTaskSemPost(&AppTaskFireTCB, OS_OPT_POST_NONE, &err);              // 会发生任务调度
		}
		
		
		OSTimeDlyHMSM( 0, 0, 0, 100,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}



/*
*********************************************************************************************************
*                                          Handle TASK
*
* Description : 负责处理开关灯，开关窗操作
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskFire       (void *p_arg)
{
	OS_ERR        err;
	uint8_t       phone[16];
	uint8_t       p_lenth, index, warning;
	
	(void) p_arg;
		
	/* 显示测试 */
	

	while(DEF_ON) {
		
		// 阻塞等待内建信号量
		
		OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &err);
		
		// 开始处理火灾报警
		
		// 鸣响蜂鸣器
		OSTaskSemPost(&AppTaskBuzzerTCB, OS_OPT_POST_NONE, &err);         // 会发生任务调度
		
		// 开窗，应该还有断气，但是没有硬件外设
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
		local_status.window = 100;                                           
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
		
		// 发送短信
//		for (index = 0; index < 32; index++) {
//			p_lenth = phone_get(user_info, phone, index);
//			
//			if (p_lenth > 0) {
//				if (REG_NORMAL == BSP_GA6_GetRegSt()) {                       // 模块注册成功
//					BSP_GA6_SenTextMSG((char*)phone, "Fire Alarm");
//				}
//			}
//		}

		BSP_UART_Printf(BSP_UART_ID_1, "发送短信\r\n");
		
		// 保证能取消酱爆，以及警报不会被重复触发
		
		do {
			OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
			warning = local_status.warning_flag;
			OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
			
			OSTimeDlyHMSM( 0, 0, 0, 10,
						   OS_OPT_TIME_HMSM_STRICT,
                           &err );
			
		} while (0x0F == warning);
		
		fire_flag = 0x00;                                                     // 允许再次被触发
		
	}
	
}





/*
*********************************************************************************************************
*                                          Motor TASK
*
* Description : 步进电机任务
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskMotor  (void *p_arg)
{
	OS_ERR      err;
	uint8_t     window_current = 0;           // 当前窗户的角度
	uint8_t     window_target = 0;            // 目标窗户角度
	
	(void) p_arg;
		
	/* 显示测试 */

	while(DEF_ON) {
		
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
		window_target = local_status.window;                              // 取得目标开度
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
		
		if (window_target != window_current) {
			if (window_target > window_current) {
				BSP_Turn_Motor(5, MOTOR_DIR_RIGHT);                       // 会自动休眠 
				window_current++;
			} else {
				BSP_Turn_Motor(5, MOTOR_DIR_LIFT);                        // 会自动休眠
				window_current--;
			}
		} else {
			OSTimeDlyHMSM( 0, 0, 0, 6,                                    // 休眠一段时间
						   OS_OPT_TIME_HMSM_STRICT,
						   &err );
		}
	}
	
}




/*
*********************************************************************************************************
*                                          BUZZER TASK
*
* Description : 该任务负债在发送火灾时冥想蜂鸣器
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskBuzzer (void *p_arg)
{
	OS_ERR     err;
	uint8_t    buzzer_flag;

	(void)p_arg;	

	while (DEF_ON) {
		
		// 等待火灾报警任务发送信号量
		
		OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &err);
				
		do {
			OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // 占用信号量
			buzzer_flag = local_status.warning_flag;
			OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // 释放信号量
			
			BSP_BUZZER_Func();
		} while (0x0F == buzzer_flag);
		
		// 关闭蜂鸣器
	}
}






/******************************************************************************************************************************************************
*																		LCD主图形界面相关代码
*******************************************************************************************************************************************************/


/*********************************************************************
*                          Defines
**********************************************************************/

#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_CHECKBOX_0 (GUI_ID_USER + 0x01)
#define ID_CHECKBOX_1 (GUI_ID_USER + 0x02)
#define ID_CHECKBOX_2 (GUI_ID_USER + 0x03)
#define ID_CHECKBOX_3 (GUI_ID_USER + 0x04)
#define ID_BUTTON_0 (GUI_ID_USER + 0x05)
#define ID_TEXT_0 (GUI_ID_USER + 0x06)
#define ID_TEXT_1 (GUI_ID_USER + 0x07)
#define ID_TEXT_2 (GUI_ID_USER + 0x08)
#define ID_TEXT_3 (GUI_ID_USER + 0x09)
#define ID_SLIDER_0 (GUI_ID_USER + 0x0A)



/*********************************************************************
*       _aDialogCreate
**********************************************************************/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 0, 0, 320, 240, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_0, 227, 59, 80, 20, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_1, 227, 98, 80, 20, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_2, 23, 98, 80, 20, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_3, 23, 59, 80, 20, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 96, 93, 112, 33, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Window Control", ID_TEXT_0, 115, 148, 123, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Temp", ID_TEXT_1, 23, 18, 128, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "CUR", ID_TEXT_2, 225, 18, 85, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Rain", ID_TEXT_3, 133, 49, 80, 20, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_0, 50, 178, 220, 20, 0, 0x0, 0 },
};



// 测试代码
static  char textBuff[20];
static  Local_status  temp_state_UI;
static OS_ERR  err_UI;




/*********************************************************************
*       回调函数
*********************************************************************/

static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;                      // 窗口句柄
  int     NCode;
  int     Id;
  uint8_t val_s = 0;
  static  uint8_t button_state = 0;   // 表示按键状态
  static  uint8_t light_state = 0;
  static  uint8_t light_auto_state = 0;
  static  uint8_t window_auto_state = 0;
  static  uint8_t power_state = 0;

  switch (pMsg->MsgId) {
    // 初始对话框
    case WM_INIT_DIALOG:
        // 初始化电源控制开关
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_0);
        CHECKBOX_SetText(hItem, "Power");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // 初始化下雨自动关窗开关
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_1);
        CHECKBOX_SetText(hItem, "W_Auto");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // 初始化自动关灯开关
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_2);
        CHECKBOX_SetText(hItem, "L_Auto");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // 初始化照明开关
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_3);
        CHECKBOX_SetText(hItem, "Light");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // 初始化一键报警按键
        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
        BUTTON_SetText(hItem, "FIRE  PRESS");
        BUTTON_SetFont(hItem, GUI_FONT_13HB_1);
        BUTTON_SetTextColor(hItem, BUTTON_CI_PRESSED, 0x6543FE);      // 设置按下时的字体颜色
        BUTTON_SetTextColor(hItem, BUTTON_CI_UNPRESSED, 0x0000FF);    // 设置为按下时的字体颜色

        // 初始化窗口控制提醒文本
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
        TEXT_SetFont(hItem, GUI_FONT_13B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00808040));

        // 初始化温度显示文本
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
        TEXT_SetFont(hItem, GUI_FONT_16B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x008B377A));

        // 初始化电流显示文本
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
        TEXT_SetFont(hItem, GUI_FONT_16B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x008B377A));

        // 初始化天气状态
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
        TEXT_SetFont(hItem, GUI_FONT_16B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x008B377A));

         // 初始化拖动条，设置范围和当前值
        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
        SLIDER_SetRange(hItem, 0, 100);
        SLIDER_SetValue(hItem, 50);
        break;    // WM_INIT_DIALOG:

    // 设置背景颜色
    case WM_PAINT:
        GUI_SetBkColor(0x00D7EBFA);
        GUI_Clear();
        break;

    // 定时溢出，用于根据单片机状态定期刷新界面
    case WM_TIMER:
		// 获取本地状态，将其拷贝到临时变量
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
		temp_state_UI.window = local_status.window;
		temp_state_UI.led_auto = local_status.led_auto;
		temp_state_UI.led_switch = local_status.led_switch;
		temp_state_UI.window_auto = local_status.window_auto;
		temp_state_UI.current     = local_status.current;
		temp_state_UI.rain_status = local_status.rain_status;
		temp_state_UI.temperature = local_status.temperature;
		temp_state_UI.power_switch = local_status.power_switch;
		temp_state_UI.warning_flag = local_status.warning_flag;
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
	
		// 更新温度,显示到小数点后一位
		sprintf(textBuff, "TEMP: %.1f `C", temp_state_UI.temperature);
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
        TEXT_SetText(hItem, (const char *)textBuff);
	
		// 更新电流，显示到小数点后一位
		sprintf(textBuff, "CUR: %.1f A", temp_state_UI.current);
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
        TEXT_SetText(hItem, (const char *)textBuff);
        
		// 更新天气状况
		if (temp_state_UI.rain_status == 0x0F) 
			sprintf(textBuff, "W: Raining");
		else 
			sprintf(textBuff, "W: No Rain");
		
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
        TEXT_SetText(hItem, (const char *)textBuff);
		
	
		// 更新照明开关light_switch
	    hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_3);
		if (0x0F == temp_state_UI.led_switch) {
			CHECKBOX_SetState(hItem, 1);
			light_state = 1;
		}
		else {
			CHECKBOX_SetState(hItem, 0);
			light_state = 0;
		}
		
		// 更新照明自动开关light_auto_switch
		hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_2);
		if (0x0F == temp_state_UI.led_auto) {
			CHECKBOX_SetState(hItem, 1);
			light_auto_state = 1;
		}
		else  {
			CHECKBOX_SetState(hItem, 0);
			light_auto_state = 0;
		}
			
		// 更新自动关窗window_auto_switch
		hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_1);
		if (0x0F == temp_state_UI.window_auto) {
			CHECKBOX_SetState(hItem, 1);
			window_auto_state = 1;
		}
		else {
			CHECKBOX_SetState(hItem, 0);
			window_auto_state = 0;
		}
		
		// 更新电源开关power_switch
		hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_0);
		if (0x0F == temp_state_UI.power_switch) {
			CHECKBOX_SetState(hItem, 1);
			power_state = 1;
		}
		else {
			CHECKBOX_SetState(hItem, 0);
			power_state = 0;
		}
		
		// 更新窗帘开度window,,此处更新不会触发回调函数
        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
        SLIDER_SetValue(hItem, temp_state_UI.window);
		
		// 更新报警器按键
		hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
		if (0x0F == temp_state_UI.warning_flag) {
			button_state = 0x01;
            BUTTON_SetPressed(hItem, 1);
			BUTTON_SetText(hItem, "ALARM CANCEL");
		} else {
			button_state = 0x00;
            BUTTON_SetPressed(hItem, 0);
			BUTTON_SetText(hItem, "FIRE  PRESS");
		}
	
		WM_RestartTimer(pMsg->Data.v, 500);             // 初始化定时器初值500ms
        break;

		
    // 监听视图被触发
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);
        NCode = pMsg->Data.v;

        switch(Id) {
            case ID_CHECKBOX_0:                          // power
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:        // 被点击时，非选中

                        break;

                    case WM_NOTIFICATION_RELEASED:       // 点击释放时，非取消选中
						power_state++;
						power_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
						if (0 == power_state) {
							local_status.power_switch = 0x00;
						} else {
							local_status.power_switch = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量

                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // 状态发生改变时
						
                        break;
                }
                break; // ID_CHECKBOX_0

            case ID_CHECKBOX_1:                          // w_auto
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:        // 被点击时，非选中

                        break;

                    case WM_NOTIFICATION_RELEASED:       // 点击释放时，非取消选中
						window_auto_state++;
						window_auto_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
						if (0 == window_auto_state) {
							local_status.window_auto = 0x00;
						} else {
							local_status.window_auto = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // 状态发生改变时
						
                        break;

                }
                break; // ID_CHECKBOX_1

            case ID_CHECKBOX_2:                          // l_auto
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:        // 被点击时，非选中

                        break;

                    case WM_NOTIFICATION_RELEASED:       // 点击释放时，非取消选中
						light_auto_state++;
						light_auto_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
						if (0 == light_auto_state) {
							local_status.led_auto = 0x00;
						} else {
							local_status.led_auto = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // 状态发生改变时
						
                        break;
                }
                break; // ID_CHECKBOX_2

            case ID_CHECKBOX_3:                         // light
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:       // 被点击时，非选中

                        break;

                    case WM_NOTIFICATION_RELEASED:      // 点击释放时，非取消选中
						light_state++;
						light_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
						if (0 == light_state) {
							local_status.led_switch = 0x00;
						} else {
							local_status.led_switch = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED: // 状态发生改变时
						
                        break;
                }
                break; // ID_CHECKBOX_3

			case ID_BUTTON_0:
				switch(NCode) {
					case WM_NOTIFICATION_CLICKED:   // 按键被按下时

						break;

					case WM_NOTIFICATION_RELEASED:  // 按键释放时
						button_state++;
						button_state &= 0x01;
						hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
						BUTTON_SetPressed(hItem, button_state);
						if (0 == button_state) {
							OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
							local_status.warning_flag = 0x00;
							OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
							BUTTON_SetText(hItem, "FIRE  PRESS");
						} else {
							OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
							local_status.warning_flag = 0x0F;
							OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
							BUTTON_SetText(hItem, "ALARM CANCEL");
						}
			
						break;
				}
				break; // ID_BUTTON_0

            case ID_SLIDER_0:
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:       // 拖动条被点击时

                        break;

                    case WM_NOTIFICATION_RELEASED:      // 拖动条被释放时
                        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
                        val_s = SLIDER_GetValue(hItem);
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // 占用信号量
						local_status.window = val_s;
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // 释放信号量
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // 拖动条数据发生改变时

                        break;

                }
                break; // ID_SLIDER_0
        }
        break; // WM_NOTIFY_PARENT

    default:
        WM_DefaultProc(pMsg);
        break;
  }
}


/*********************************************************************
*       CreateFramewin
**********************************************************************/
WM_HWIN CreateFramewin(void);
WM_HWIN CreateFramewin(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}


/*********************************************************************
*       MainTask UI线程入口函数
**********************************************************************/

void MainTask(void)
{
    WM_HWIN   hDlg;                                 // 窗口句柄
	
    // 窗口使能使用内存设备，防止闪烁，放在GUI_Init()之前可以保证所有
    // 窗口都能使用动态内存设备
    WM_SetCreateFlags(WM_CF_MEMDEV);

    GUI_Init();
	

    // 创建一个窗口并且返回一个句柄,用于为其创建一个定时器
    hDlg = CreateFramewin();

    // 为窗口创建一个定时器，定时器溢出就会触发，用于更新界面的View
    WM_CreateTimer(WM_GetClientWindow(hDlg), 0, 1000, 0);

    while (1) {
        GUI_Delay(10);
    }

}





