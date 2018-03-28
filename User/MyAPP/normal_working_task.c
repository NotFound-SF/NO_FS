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

#include "my_app_cfg.h"
#include  "user_info.h"
#include  "stdlib.h"
#include  "string.h"

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




/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskSensor (void *p_arg);

static  void  AppTaskLed    (void *p_arg);

static  void  AppTaskTouch  (void *p_arg);

static  void  AppTaskMotor  (void *p_arg);

static  void  AppTaskWifi   (void *p_arg);




/*
*********************************************************************************************************
*                                       LOCAL DEFINES
*********************************************************************************************************
*/

#define     DEVICE_ID                     12345         // 硬件设备唯一ID
#define     ALIVE_TIME_OUT                2000          // 单位为时钟节拍周期
 

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




/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/




static       OS_SEM               StatusSemLock;          // 用于独占访问状态全局变量  

static       Local_status         local_status;           // 全局状态

static       uint8_t              server_flag = 0x00;     // 服务器的连接状态         

static       InfoStruct           *user_info;             // 使用该内存区域存放Flash中的用户数据




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
				(OS_MSG_QTY  ) 5u,
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
				(OS_MSG_QTY  ) 5u,
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
	
#if 0		
	
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
	

	
	
	
#endif
	
	
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
	
	// 先将Flash中的数据读入内存
	
	get_user_info(user_info, &AppTaskWifiTCB); 
	
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
		
//		BSP_UART_Printf(BSP_UART_ID_1,"CH1: %d\r\n", BSP_ADC_GetDat(BSP_ADC_CH1));
//		BSP_UART_Printf(BSP_UART_ID_1,"CH2: %d\r\n", BSP_ADC_GetDat(BSP_ADC_CH2));
//		BSP_UART_Printf(BSP_UART_ID_1,"CH3: %d\r\n\r\n", BSP_ADC_GetDat(BSP_ADC_CH3));
		
		
//		BSP_UART_Printf(BSP_UART_ID_1, "i: %f\r\n", BSP_ACS_GetS_Real());
		
		BSP_LED_Toggle(3);
		
		OSTimeDlyHMSM( 0, 0, 0, 200,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
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
	
	(void) p_arg;
		
	/* 显示测试 */

	while(DEF_ON) {
//		BSP_Turn_Motor(360, MOTOR_DIR_RIGHT);
		
		OSTimeDlyHMSM( 0, 0, 6, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
}







