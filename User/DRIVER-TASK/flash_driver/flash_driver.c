
/*
*********************************************************************************************************
*                          ���ļ�����һ���߳�ר�Ÿ���flash��д�����������߳�Ҫ����
*                        flash����������̷߳�����Ϣ����Ϣ��ʽ�μ�ͷ�ļ�
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
* Description : ��ʼ����flashоƬ���Ҵ�������flash��д�������߳�.
*
* Argument(s) : none
*
* Return(s)   : DEF_FAIL ��ʾ��������ʧ��
*             : DEF_OK   ��ʾ��������ɹ�
*
* Caller(s)   : BSP_Init();
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_FLASH_Drivet_Init (void)
{
	OS_ERR      err;
	// flash�ײ��ʼ��
	BSP_FLASH_Init();
	
	
	// ��������flash���߳�
	
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
* Description : �ú����������̵߳��ã����������������������Ϣ����ִ����Ӧ�Ķ�д����,��������ִ����Ϻ󷽲ŷ���
*
* Argument(s) : request �ò���Ϊָ�룬ָ��һƬ�������ڴ�����
*
* Return(s)   : DEF_FAIL ��ʾ��������ʧ��
*             : DEF_OK   ��ʾ��������ɹ�
*
* Caller(s)   : �����߳�;
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   BSP_FLASH_Drivet_Request (FLASH_DRIVER_MSG *request)
{
	OS_ERR   err;
	
	// �������̵߳��ڽ���Ϣ���з�����Ϣ
	OSTaskQPost((OS_TCB      *) &DriverTaskFlashTCB,            // ����������
				(void        *) request,                        // ��Ϣ����
				(OS_MSG_SIZE  ) sizeof(FLASH_DRIVER_MSG),       // ��Ϣ����
				(OS_OPT       ) OS_OPT_POST_FIFO,               // FIFO����
				(OS_ERR      *) &err);
	
	// �����ȴ���������ִ����ôζ�д�������������ź���ʵ��
	OSTaskSemPend((OS_TICK      ) 0,                            // ��������
				  (OS_OPT       ) OS_OPT_PEND_BLOCKING,         // �����ȴ�
				  (CPU_TS      *) NULL,                         // ����Ҫʱ���
				  (OS_ERR      *) &err);                        // �����־
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
* Description : ������ר�Ÿ������flash��д����
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
		
		// �����ȴ������̷߳�����������ִ����Ӧ�Ĳ���
		request = OSTaskQPend((OS_TICK      ) 0,
							  (OS_OPT       ) OS_OPT_PEND_BLOCKING,
							  (OS_MSG_SIZE *) &msg_size,
							  (CPU_TS      *) NULL,
							  (OS_ERR      *) &err);
		
		if (OS_ERR_NONE != err)
			continue;
		
		// ���ݽ��յĵ�������ִ����Ӧ�Ĳ���
		dat_len = request->len;
		
		if (FLASH_OPTION_R == request -> option) {                     // ��ʾ������
			BSP_FLASH_BufferRead(((uint8_t*)request)+sizeof(FLASH_DRIVER_MSG),
								 request->addr, dat_len);
		} else if (FLASH_OPTION_W == request -> option) {              // ��ʾд����
			BSP_FLASH_SectorErase(request->addr/4096);                 // ���ǰ��Ҫ��д����ִ�в�������
			BSP_FLASH_BufferWrite(((uint8_t*)request)+sizeof(FLASH_DRIVER_MSG),
								  request->addr, dat_len);
		}
		
		
		// �Ӵ�������������
		OSTaskSemPost(request->task_tcb,
					  OS_OPT_POST_NONE,                                // ��ʵ������Ҳ������Ϊwhile��ʼ���������
					  &err);
	}
	
}




