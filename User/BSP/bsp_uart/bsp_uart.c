/*
*********************************************************************************************************
*        ����������ʹ�����ź��������Բ�����ISR���ø��ļ�����������SemLockĬ�ϳ�ʼ��Ϊ1���Զ�д����
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_uart.h"


/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct {
	OS_SEM     SemLock;                                       /* ��UART��ռ�ź�������дͬʱռ��                    */
    OS_SEM     SemRdWait;                                     /* UART���ȴ��ź���                                 */
	OS_SEM     SemTxWait;                                     /* UART���͵ȴ��ź���                               */  
	CPU_INT16U BufLen;                                        /* UART������ȡ�����ݳ���                           */
	CPU_INT08U *BufPtr;                                       /* UART����������ͷָ��                             */
}BSP_UART_DEV_STATUS; 


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_UART_DEV_STATUS  BSP_UART_DevTbl[BSP_UART_NBR_MAX];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void        BSP_UARTx_Handler  (CPU_INT08U uart_id);

static  void        NVIC_Configuration(CPU_INT08U uart_id);
static  void        BSP_UART_GPIO_Init(CPU_INT08U uart_id);
static  void        BSP_UART_SEM_Creat(CPU_INT08U uart_id);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        BSP_UART_Init()
*
* Description : Initialize the UART.
*
* Argument(s) : uart_id    BSP_UART_ID_x  x = [1,6]
*
*               baud       ������
*
* Return(s)   : NONE
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_UART_Init (CPU_INT08U uart_id, CPU_INT32U uart_baud)
{
	USART_TypeDef     *uartPort;                                             /* UART�˿ں�                         */
	USART_InitTypeDef uartInit;
	
	BSP_UART_SEM_Creat(uart_id);                                             /* ������Ӧ���ź���                    */
	BSP_UART_GPIO_Init(uart_id);                                             /* ��GPIO���ӵ�UART                   */
	NVIC_Configuration(uart_id);                                             /* ���ӵ�NVIC                         */
	
	uartInit.USART_BaudRate            = uart_baud;                          /* ������                             */
	uartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     /* ��ʹ��Ӳ��������                    */
	uartInit.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;      /* ���շ�                             */ 
	uartInit.USART_StopBits            = USART_StopBits_1;                   /* ֹͣλ1λ                          */
	uartInit.USART_WordLength          = USART_WordLength_8b;                /* ���ݿ��8λ                        */
	uartInit.USART_Parity              = USART_Parity_No;                    /* ��ʹ��У��                         */
	
	switch (uart_id) {
		case BSP_UART_ID_1:
			UART1_CLK_CMD(UART1_DEV_CLK, ENABLE);
			uartPort = USART1;
	    	break;
		
		case BSP_UART_ID_2:
			UART2_CLK_CMD(UART2_DEV_CLK, ENABLE);
			uartPort = USART2;
	    	break;
		
		case BSP_UART_ID_3:
			UART3_CLK_CMD(UART3_DEV_CLK, ENABLE);
			uartPort = USART3;
	    	break;
		
		case BSP_UART_ID_4:
			UART4_CLK_CMD(UART4_DEV_CLK, ENABLE);
			uartPort = UART4;
	    	break;
		
		case BSP_UART_ID_5:
			UART5_CLK_CMD(UART5_DEV_CLK, ENABLE);
			uartPort = UART5;
	    	break;
		
		case BSP_UART_ID_6:
			UART6_CLK_CMD(UART6_DEV_CLK, ENABLE);
			uartPort = USART6;
		
		default:
			break;
	}
	
	USART_ITConfig(uartPort, USART_IT_IDLE, DISABLE);                            /* �رտ����жϣ�����ʱ�ж�                  */
	USART_ITConfig(uartPort, USART_IT_RXNE, DISABLE);                            /* �رս����ж�                             */
	USART_ITConfig(uartPort, USART_IT_TXE, DISABLE);                             /* �رշ����ж�                             */
	
	USART_Init(uartPort, &uartInit);
	USART_Cmd(uartPort, ENABLE); 
}


/*
*********************************************************************************************************
*                                        BSP_UART_RdByte()
*
* Description : ��ȡ�˿�Ϊuart_id��UATR���ݣ���ȡһ���ֽ�.
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*
*             : timeOut     ��ȡ��ĵȴ�ʱ��
*                                     
*
* Return(s)   : ���ض�ȡ��������
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

CPU_INT08U  BSP_UART_RdByte (CPU_INT08U uart_id, OS_TICK timeOut)
{
	CPU_INT08U data;
	
	BSP_UART_RdData(uart_id, &data, 1,timeOut);
	
	return data;
}


/*
*********************************************************************************************************
*                                        BSP_UART_WrByte()
*
* Description : ��˿�Ϊuart_id��UATRдһ���ֽ�����
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                                     
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

void  BSP_UART_WrByte (CPU_INT08U uart_id, CPU_INT08U data)
{
	BSP_UART_WrData(uart_id, &data, 1);
}



/*
*********************************************************************************************************
*                                        BSP_UART_WrStr()
*
* Description : ��˿�Ϊuart_id��UATR����һ���ַ���
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                                     
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

void  BSP_UART_WrStr (CPU_INT08U uart_id, CPU_CHAR *data)
{
	CPU_INT08U len = 0;
	
	while(0 != *(data+len)) {
		len++;
	}
	
	BSP_UART_WrData(uart_id, (CPU_INT08U*)data, len);
}



/*
*********************************************************************************************************
*                                        BSP_UART_RdData()
*
* Description : ��ȡ�˿�Ϊuart_id��UATR���ݣ���ȡ����Ϊlen.
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                             
*               data        ָ����ջ�������ͷָ��      
*                            
*               len         ��Ҫ���յ����ݳ���    
*
*               timeOut     ��ʾ�����ڸ�ʱ���ڶ�ȡ�����ݣ��������������ϵķ��Ͳ��ҽ�������ʱҲ�ᳬʱ�˳������Ե�����
*                           Ҫȷ����ָ��ʱ�����ܶ�ȡ���������ݣ�
*
* Return(s)   : ����ʵ�ʶ�ȡ�����ݳ���
*
* Caller(s)   : Application
*
* Note(s)     : �ú�������ʱ��⣬����ȡ����ʵ�����ݳ��ȿ��ܱ�lenС
*********************************************************************************************************
*/

static __IO uint8_t   ReadNoUse;                                               /* ������ÿ�ζ�ȡ����ǰ��ȡ��Ӧ�ļĴ������Դﵽ�����־λ��Ŀ�� */

CPU_INT16U  BSP_UART_RdData (CPU_INT08U uart_id, CPU_INT08U *data, CPU_INT16U len, OS_TICK timeOut)
{
	
	OS_ERR         err;
	USART_TypeDef *uartPort;
	CPU_INT16U     rdLen = 0;
	BSP_UART_DEV_STATUS *p_uart_dev_status;
	
	// ������Ч�Լ��
	
	if (uart_id >= BSP_UART_NBR_MAX)
		return 0;
	
	if (0 == len)                                          
		return 0;
	
	p_uart_dev_status = BSP_UART_DevTbl+uart_id;                               /* ���ӵ���ǰ�˿�ʹ�õ��豸״̬�ṹ��        */
																			   /* �������ź�����ֹ�ṹ�������ݱ����        */
	OSSemPend((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_TICK ) BSP_UAER_LOCK_TIME,                                                             
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 /* �����ȴ�                                */
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return 0;                    
	
	p_uart_dev_status->BufLen = len;                                           /* ׼����ȡ�����ݳ���                       */
	p_uart_dev_status->BufPtr = data;                                          /* ��ȡ�����ݵĴ洢������                   */
	                                                                         
	switch (uart_id) {                                                         /* ƥ��˿ں�                              */
		case BSP_UART_ID_1:
			uartPort = USART1;
			break;
		
		case BSP_UART_ID_2:
			uartPort = USART2;
			break;
		
		case BSP_UART_ID_3:
			uartPort = USART3;
			break;
		
		case BSP_UART_ID_4:
			uartPort = UART4;
			break;
		
		case BSP_UART_ID_5:
			uartPort = UART5;
			break;
		
		case BSP_UART_ID_6:
			uartPort = USART6;
			break;
		
		default :
			break;
	}
	
         
	// �ڶ�ȡ����ǰ��������ж���Ӧ�ı�־��������δ���ö�����ʱ������������������
	// ��ô��ʱ�����߲�������������ʽ�ٵ��ö�ȡ����ʱ���������أ���Ϊ�����жϱ�־
	// ��λ������һ�����жϾͻᵼ�º����������أ�������������б�־�ķ���
	                           
	ReadNoUse = uartPort -> SR;	                                              
	ReadNoUse = uartPort -> DR;                    
	
	// ���������ж�������ж�
	
	uartPort -> CR1 |= UART_IDEL_RXN_MASK;            
	
	// �ȴ������жϷ���
	OSSemPend((OS_SEM *) &(p_uart_dev_status->SemRdWait),                       /* �ȴ��������                            */
	          (OS_TICK ) timeOut,                                               /* ʹ����������Զ����                       */
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                  /* �����ȴ�                                */
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
			  
	// �رտ����ж�������ж�
	
	uartPort -> CR1 &= ~UART_IDEL_RXN_MASK; 

	// �����ȡ�������ݳ���
	
	rdLen = len-(p_uart_dev_status->BufLen);                                    /* �������ͷŶ˿�ǰ����                     */
	
	// ����ǵȴ���ʱ���ؾ���Ҫȷ��SemRdWait�ź�����ֵΪ0����Ϊ���ȴ���ʱ����ʱ�����ڹر��ж�֮ǰ
	// һ�����ݵ��ﵼ�¿����ж�ִ�У���ô�жϾͻᵼ��SemRdWait��һ����ô�´ε��ö����ݺ���ʱ�ͻ�
	// �������أ��Ⲣ�������������ģ���Ȼ������������ĸ����൱С���������ǻ���Ӧ��ȷ���ڹر��жϺ�
	// ������ź�����
	
	if (OS_ERR_TIMEOUT == err && 0 != rdLen ) {                                /* ���ٽ�����жϲſ��� */                                                                    
		OSSemSet((OS_SEM   *) &(p_uart_dev_status->SemRdWait),
				 (OS_SEM_CTR) 0,
		         (OS_ERR   *) &err);
	}
	
	
	                                                                            /* �ͷŸö˿�                              */
	OSSemPost((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &err);
	
	return rdLen;                                                              /* ���ض�ȡ����ʵ���ֽ���                   */
}


/*
*********************************************************************************************************
*                                        BSP_UART_WrData()
*
* Description : ��ȡ�˿�Ϊuart_id��UATR���ݣ���ȡ����Ϊlen.
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                             
*               data        ָ���ͻ�������ͷָ��      
*                            
*               len         ��Ҫ���͵����ݳ���               
*
* Return(s)   : ʵ��д�������ݳ���
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

CPU_INT16U  BSP_UART_WrData (CPU_INT08U uart_id, CPU_INT08U *data, CPU_INT16U len)
{
	OS_ERR  err;
	USART_TypeDef *uartPort;
	CPU_INT16U rdLen = 0;
	BSP_UART_DEV_STATUS *p_uart_dev_status;
	
	if(0 == len)
		return 0;
	if(uart_id >= BSP_UART_NBR_MAX)
		return 0;
	p_uart_dev_status = BSP_UART_DevTbl+uart_id;
	
	OSSemPend((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_TICK ) BSP_UAER_LOCK_TIME,                                   /* ������õȴ����ܵ����������ù���          */                           
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 /* �����ȴ�                                */
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return 0;     
	
	switch (uart_id) {                                                         /* ƥ��˿ں�                              */
		case BSP_UART_ID_1:
			uartPort = USART1;
			break;
		
		case BSP_UART_ID_2:
			uartPort = USART2;
			break;
		
		case BSP_UART_ID_3:
			uartPort = USART3;
			break;
		
		case BSP_UART_ID_4:
			uartPort = UART4;
			break;
		
		case BSP_UART_ID_5:
			uartPort = UART5;
			break;
		
		case BSP_UART_ID_6:
			uartPort = USART6;
			break;
		
		default :
			break;
		
	}
	
	USART_SendData(uartPort, *data);                                           /* �������ɴ�����һ�η����ж�                 */
	p_uart_dev_status->BufLen = len-1;                                         /* ׼��д�������ݳ���                        */
	p_uart_dev_status->BufPtr = data+1;                                        /* ���ݵĴ洢������                          */ 
	USART_ITConfig(uartPort, USART_IT_TXE, ENABLE);                            /* ������������ж��ж�                      */
	 
	OSSemPend((OS_SEM *)&(p_uart_dev_status->SemTxWait),                       /* �ȴ��������                              */
	          (OS_TICK )BSP_UAER_LOCK_TIME,
	          (OS_OPT  )OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
//	USART_ITConfig(uartPort, USART_IT_TXE, DISABLE);                           /* ���ж��йرո��ж�                         */
	rdLen = len-(p_uart_dev_status->BufLen);                  
	
	OSSemPost((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &err);
	
	return rdLen;
}



/*
*********************************************************************************************************
*                                           BSP_UART_Printf()
*
* Description : Print formatted data to the output serial port.
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*               format      String that contains the text to be written.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function output a maximum of BSP_SER_PRINTF_STR_BUF_SIZE number of bytes to the
*                   serial port.  The calling function hence has to make sure the formatted string will
*                   be able fit into this string buffer or hence the output string will be truncated.
*********************************************************************************************************
*/
void  BSP_UART_Printf (CPU_INT08U uart_id, CPU_CHAR  *format, ...)
{
    CPU_CHAR  buf_str[BSP_SER_PRINTF_STR_BUF_SIZE + 1u];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

    BSP_UART_WrStr(uart_id, buf_str);
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
*                                        NVIC_Configuration()
*
* Description : ��ʼ�� UART ���ӵ�NVIC
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UART_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  NVIC_Configuration(CPU_INT08U uart_id)
{
	NVIC_InitTypeDef nvicInit;
  
	switch (uart_id) {                                                           /* �����ж�Դ                      */
		case BSP_UART_ID_1:
			nvicInit.NVIC_IRQChannel = BSP_UART1_IRQ;
			break;
		
		case BSP_UART_ID_2:
			nvicInit.NVIC_IRQChannel = BSP_UART2_IRQ;
			break;
		
		case BSP_UART_ID_3:
			nvicInit.NVIC_IRQChannel = BSP_UART3_IRQ;
			break;
				
		case BSP_UART_ID_4:
			nvicInit.NVIC_IRQChannel = BSP_UART4_IRQ;
			break;
		
		case BSP_UART_ID_5:
			nvicInit.NVIC_IRQChannel = BSP_UART5_IRQ;
			break;
		
		case BSP_UART_ID_6:
			nvicInit.NVIC_IRQChannel = BSP_UART6_IRQ;
			break;
		
		default:
			break;
	}         

	nvicInit.NVIC_IRQChannelPreemptionPriority = BSP_UART_NVIC_PRE_PRIO;         /* �������ȼ�                      */
	nvicInit.NVIC_IRQChannelSubPriority        = BSP_UART_NVIC_SUB_PRIO;         /* �����ȼ�                        */
	nvicInit.NVIC_IRQChannelCmd                = ENABLE;                         /* ʹ���ж�                        */
	
	NVIC_Init(&nvicInit);
}



/*
*********************************************************************************************************
*                                        BSP_UART_GPIO_Init()
*
* Description : ��ʼ�� GPIOӳ�䵽UART
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UART_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_UART_GPIO_Init (CPU_INT08U uart_id)
{
	GPIO_InitTypeDef gpioInit_TX, gpioInit_RX;
	GPIO_TypeDef    *gpioPortTX, *gpioPortRX;
	
	switch (uart_id) {                                      
		case BSP_UART_ID_1:
			gpioInit_TX.GPIO_Pin = UART1_TX_PIN;                                /* ��������                                 */
			gpioInit_RX.GPIO_Pin = UART1_RX_PIN;
			gpioPortTX = UART1_GPIO_PORT_TX;                                    /* ���ӵ��˿�                               */
			gpioPortRX = UART1_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART1_GPIO_CLK_TX, ENABLE);                  /* ����GPIO�˿�ʱ��                         */
			RCC_AHB1PeriphClockCmd(UART1_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);  /* GPIO���ܸ���                             */
			GPIO_PinAFConfig(gpioPortRX, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);
			break;
		
		case BSP_UART_ID_2:
			gpioInit_TX.GPIO_Pin = UART2_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART2_RX_PIN;
		    gpioPortTX = UART2_GPIO_PORT_TX;
			gpioPortRX = UART2_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART2_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART2_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART2_TX_PIN_SOURCE, GPIO_AF_USART2);
			GPIO_PinAFConfig(gpioPortRX, UART2_RX_PIN_SOURCE, GPIO_AF_USART2);
			break;
		
		case BSP_UART_ID_3:
			gpioInit_TX.GPIO_Pin = UART3_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART3_RX_PIN;
			gpioPortTX = UART3_GPIO_PORT_TX;
			gpioPortRX = UART3_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART3_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART3_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART3_TX_PIN_SOURCE, GPIO_AF_USART3);
			GPIO_PinAFConfig(gpioPortRX, UART3_RX_PIN_SOURCE, GPIO_AF_USART3);
			break;
				
		case BSP_UART_ID_4:
			gpioInit_TX.GPIO_Pin = UART4_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART4_RX_PIN;
			gpioPortTX = UART4_GPIO_PORT_TX;
			gpioPortRX = UART4_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART4_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART4_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART4_TX_PIN_SOURCE, GPIO_AF_UART4);
			GPIO_PinAFConfig(gpioPortRX, UART4_RX_PIN_SOURCE, GPIO_AF_UART4);
			break;
		
		case BSP_UART_ID_5:
			gpioInit_TX.GPIO_Pin = UART5_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART5_RX_PIN;
			gpioPortTX = UART5_GPIO_PORT_TX;
			gpioPortRX = UART5_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART5_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART5_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART5_TX_PIN_SOURCE, GPIO_AF_UART5);
			GPIO_PinAFConfig(gpioPortRX, UART5_RX_PIN_SOURCE, GPIO_AF_UART5);
			break;
		
		case BSP_UART_ID_6:
			gpioInit_TX.GPIO_Pin = UART6_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART6_RX_PIN;
			gpioPortTX = UART6_GPIO_PORT_TX;
			gpioPortRX = UART6_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART6_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART6_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART6_TX_PIN_SOURCE, GPIO_AF_USART6);
			GPIO_PinAFConfig(gpioPortRX, UART6_RX_PIN_SOURCE, GPIO_AF_USART6);
			break;
		
		default:
			break;
	}    
	
	gpioInit_TX.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_TX.GPIO_OType = GPIO_OType_PP;
	gpioInit_TX.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit_TX.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortTX, &gpioInit_TX);
	
	gpioInit_RX.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_RX.GPIO_OType = GPIO_OType_OD;                    
	gpioInit_RX.GPIO_PuPd  = GPIO_PuPd_UP;   // 2018/4/9 ԭ��ΪUP
	gpioInit_RX.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortRX, &gpioInit_RX);
}


/*
*********************************************************************************************************
*                                        BSP_UART_SEM_Creat()
*
* Description : ����UART�����ź���
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UART_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_UART_SEM_Creat (CPU_INT08U uart_id)
{
	OS_ERR err;
	
	if(uart_id >= BSP_UART_NBR_MAX)
		return;
	
	OSSemCreate((OS_SEM    *)&((BSP_UART_DevTbl+uart_id)->SemLock),
                (CPU_CHAR  *)"UART Lock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	OSSemCreate((OS_SEM    *)&((BSP_UART_DevTbl+uart_id)->SemTxWait),
                (CPU_CHAR  *)"UART Tx Wait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
	
	OSSemCreate((OS_SEM    *)&((BSP_UART_DevTbl+uart_id)->SemRdWait),
                (CPU_CHAR  *)"UART Rd Wait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
}



/*
*********************************************************************************************************
*                                        BSP_UARTx_Handler()
*
* Description : �жϴ�����
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UARTx_Handler
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_UARTx_Handler (CPU_INT08U uart_id)
{
	OS_ERR err;
	USART_TypeDef *uartPort;
	BSP_UART_DEV_STATUS *p_uart_dev_status;
	
	p_uart_dev_status = BSP_UART_DevTbl+uart_id;                               /* ���ӵ���ǰ�˿�ʹ�õ��豸״̬�ṹ��        */
	
	switch (uart_id) {                                                         /* ƥ��˿ں�                              */
		case BSP_UART_ID_1:
			uartPort = USART1;
			break;
		
		case BSP_UART_ID_2:
			uartPort = USART2;
			break;
		
		case BSP_UART_ID_3:
			uartPort = USART3;
			break;
		
		case BSP_UART_ID_4:
			uartPort = UART4;
			break;
		
		case BSP_UART_ID_5:
			uartPort = UART5;
			break;
		
		case BSP_UART_ID_6:
			uartPort = USART6;
			break;
		
		default :
			break;
	}
	
	if (SET == USART_GetITStatus(uartPort, USART_IT_RXNE)) {                  /* ���ջ������ǿ�                            */
		if(0 != p_uart_dev_status->BufLen) {                                  /* ��ʾ����δ���                            */
			*(p_uart_dev_status->BufPtr) = USART_ReceiveData(uartPort);       /* ��д���ջ�����                            */
			p_uart_dev_status->BufPtr++;
			p_uart_dev_status->BufLen--;                                         
		} 
		
		USART_ClearITPendingBit(uartPort, USART_IT_RXNE);                     /* �����־λ                                */                                  
	}
	
	if (SET == USART_GetITStatus(uartPort, USART_IT_IDLE)) {                  /* ��ʾһ֡���ݷ��ͽ���                      */
		USART_ReceiveData(uartPort);                                          /* �����־λ                               */ 
		OSSemPost((OS_SEM *)&(p_uart_dev_status->SemRdWait),
	              (OS_OPT  )OS_OPT_POST_1,
	              (OS_ERR *) &err);
	}	

	if (SET == USART_GetITStatus(uartPort, USART_IT_TXE)) {                  /* ������һ���ֽ�                           */     
		if(0 != p_uart_dev_status->BufLen) {                                 /* �����ͻ��������ݻ�δ���ͽ���               */                           
			USART_SendData(uartPort, *(p_uart_dev_status->BufPtr));
			p_uart_dev_status->BufPtr++;
			p_uart_dev_status->BufLen--;   
		} else {                                                             /* ��ʾ���ͽ���                              */
			USART_ITConfig(uartPort, USART_IT_TXE, DISABLE);                 /* �ر��ж�                                  */
			OSSemPost((OS_SEM *)&(p_uart_dev_status->SemTxWait),
	              (OS_OPT  )OS_OPT_POST_1,
	              (OS_ERR *) &err);
		}       
	}
	
}


/*
*********************************************************************************************************
*                                        BSP_UART1_Handler()
*
* Description : UART1�жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : ϵͳ
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART1_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_1);
}


/*
*********************************************************************************************************
*                                        BSP_UART2_Handler()
*
* Description : UART2�жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : ϵͳ
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART2_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_2);
}


/*
*********************************************************************************************************
*                                        BSP_UART3_Handler()
*
* Description : UART3�жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : ϵͳ
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART3_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_3);
}


/*
*********************************************************************************************************
*                                        BSP_UART4_Handler()
*
* Description : UART4�жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : ϵͳ
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART4_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_4);
}


/*
*********************************************************************************************************
*                                        BSP_UART5_Handler()
*
* Description : UART5�жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : ϵͳ
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART5_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_5);
}


/*
*********************************************************************************************************
*                                        BSP_UART5_Handler()
*
* Description : UART5�жϴ�����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : ϵͳ
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART6_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_6);
}




