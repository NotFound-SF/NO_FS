

/*
*********************************************************************************************************
*        ����������ʹ�����ź��������Բ�����ISR���ø��ļ�����������SemLockĬ�ϳ�ʼ��Ϊ1���Զ�д����
*        ͬ��������������ʹ�õ���ģʽ����ʹ�ӻ�������ʼ�ź��������豸��ַƥ�䣬��������жϣ�����Ĭ
*        ��State����ʼ��ΪBSP_I2C_STATE_IDLE���������ж��в���������һ����Ӧ
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_iic.h"

/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct {
	OS_SEM                 SemLock;                                  //��IIC��ռ�ź���                                
    OS_SEM                 SemWait;                                  //IIC�ȴ��ź���                                  
	CPU_INT16U             BufLen;                                   //������д�����ݳ���                              
	CPU_INT08U            *BufPtr;                                   //��д���ݵĻ�����ָ��                            
	CPU_INT08U             Addr;                                     //�ӻ���ַ   
	I2C_State_TypeDef      State;                                    //��ǰI2C״̬  
	I2C_AccessType_TypeDef AccessType;                               //��������	                                   
}BSP_I2C_PERIPH_STATUS; 


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_I2C_PERIPH_STATUS  BSP_I2C_PeriphTbl[BSP_I2C_NBR_MAX]; 


/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/

#define        ADDR_NONE                        0x00

#define        BSP_I2C_REG_SR1_ERR_MASK        (BSP_I2C_REG_SR1_BERR    | \
                                                BSP_I2C_REG_SR1_ARLO    | \
                                                BSP_I2C_REG_SR1_AF      | \
                                                BSP_I2C_REG_SR1_OVR     | \
                                                BSP_I2C_REG_SR1_PECERR  | \
												BSP_I2C_REG_SR1_TIMEOUT | \
                                                BSP_I2C_REG_SR1_SMBALERT)
                                                

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void         BSP_I2Cx_EV_Handler  (CPU_INT08U i2c_id);
static  void         BSP_I2Cx_ER_Handler  (CPU_INT08U i2c_id);

static  void         NVIC_Configuration   (CPU_INT08U i2c_id);
static  void         BSP_I2C_GPIO_Init    (CPU_INT08U i2c_id);
static  CPU_BOOLEAN  BSP_I2C_SEM_Creat    (CPU_INT08U i2c_id);

static  CPU_BOOLEAN  BSP_I2C_StartXfer ( CPU_INT08U              i2c_id,
                                         CPU_INT08U              i2c_addr,
                                         I2C_AccessType_TypeDef  i2c_access_type,
                                         CPU_INT08U             *p_buf,                    
                                         CPU_INT16U              nbr_bytes);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_I2C_Init()
*
* Description : Initialize the I2C.
*
* Argument(s) : i2c_id    BSP_I2C_ID_x                   x = [1,3]
*               i2c_mode  BSP_I2C_MODE_x                 x = STANDARD,FAST_2_1,FAST_16_9
*            	clk_freq  <= BSP_I2C_MODE_x_MAX_FREQ_HZ  x = STANDARD,FAST
*
* Return(s)   : DEF_OK    ������ʼ��
*               DEF_FAIL  ��ʼ��ʧ�ܣ��������ź���
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Init (CPU_INT08U       i2c_id,
                           I2C_Mode_TypeDef i2c_mode,
						   CPU_INT32U       clk_freq)
						   
{
	CPU_BOOLEAN     status;
	I2C_TypeDef    *i2cPort;
	I2C_InitTypeDef i2cInit;
	
	BSP_I2C_GPIO_Init(i2c_id);                                           //��ʼ��GPIO
	NVIC_Configuration(i2c_id);                                          //���ӵ��ж�
	status = BSP_I2C_SEM_Creat(i2c_id);                                  //������Ӧ���ź���
	
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return DEF_FAIL;
	
	if(clk_freq > BSP_I2C_MODE_FAST_MAX_FREQ_HZ) {                       //��������ģʽ���Ƶ��
		return DEF_FAIL;
	}
		
	if(BSP_I2C_MODE_STANDARD == i2c_mode) {
		if(clk_freq > BSP_I2C_MODE_STANDARD_MAX_FREQ_HZ)                 //������׼ģʽģʽ���Ƶ��
			return DEF_FAIL;
	}
	
	switch (i2c_id) {
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			I2C1_PERIPH_RCC_CMD(I2C1_PERIPH_RCC, ENABLE);                //��������ʱ��
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			I2C2_PERIPH_RCC_CMD(I2C2_PERIPH_RCC, ENABLE);                //��������ʱ��
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			I2C3_PERIPH_RCC_CMD(I2C3_PERIPH_RCC, ENABLE);                //��������ʱ��
			break;
		
		default:
			break;
	}
	      
	
	switch (i2c_mode) {                                                  //����ģʽ��ʱ��ռ�ձ�
		case BSP_I2C_MODE_STANDARD:
		case BSP_I2C_MODE_FAST_2:
			i2cInit.I2C_DutyCycle   = I2C_DutyCycle_2; 
			break;
		
		case BSP_I2C_MODE_FAST_16_9:
			i2cInit.I2C_DutyCycle   = I2C_DutyCycle_16_9;
			break;
		
		default:
			break;
	}
	
	
	i2cInit.I2C_Mode        = I2C_Mode_I2C;                              //����ΪI2Cģʽ
	i2cInit.I2C_ClockSpeed  = clk_freq;                                  //ʱ��Ƶ��
	i2cInit.I2C_Ack         = I2C_Ack_Disable;                           //ʧ��Ӧ��
	i2cInit.I2C_OwnAddress1 = I2C_OWN_ADDRESS7;                          //�����ַ	                      
	i2cInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;      //7λ��ַģʽ   	

											                             //���˿�����ΪĬ��״̬
	(BSP_I2C_PeriphTbl+(i2c_id)) -> BufLen = 0;                             
	(BSP_I2C_PeriphTbl+(i2c_id)) -> Addr   = ADDR_NONE;                    
	(BSP_I2C_PeriphTbl+(i2c_id)) -> BufPtr = (CPU_INT08U*)0;         
	(BSP_I2C_PeriphTbl+(i2c_id)) -> State  = BSP_I2C_STATE_IDLE;            
	(BSP_I2C_PeriphTbl+(i2c_id)) -> AccessType = BSP_I2C_ACCESS_TYPE_NONE; 
	
	I2C_Init(i2cPort, &i2cInit);
	I2C_Cmd(i2cPort, ENABLE);
	
	return status;
}



/*
*********************************************************************************************************
*                                        BSP_I2C_Rd()
*
* Description : Read 'n' bytes from the I2C bus.
*
* Argument(s) : i2c_id       BSP_I2C_ID_x              x = [1,3]
*
*               i2c_addr     The I2C device address
*
*               p_buf        Pointer to the buffer into which the bytes will be stored.
*
*               nbr_bytes    Number of bytes to be read.
*
* Return(s)   : DEF_OK       If all bytes were read.
*               DEF_FAIL     If all bytes could not be read.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Rd (CPU_INT08U      i2c_id,
                            CPU_INT08U   i2c_addr,
                            CPU_INT08U  *p_buf,
                            CPU_INT16U   nbr_bytes)
{
	CPU_BOOLEAN  err;
	
	err = BSP_I2C_StartXfer(i2c_id,                                       //�ú������в������
							i2c_addr,
							BSP_I2C_ACCESS_TYPE_RD, 
							p_buf, nbr_bytes);    

	return err;
}


/*
*********************************************************************************************************
*										  BSP_I2C_Wr()
*
* Description :  Write 'n' bytes from the I2C bus.
*
* Argument(s) :  i2c_id 	      BSP_I2C_ID_x				x = [1,3]
*
*				 i2c_addr	  The I2C device address
*
*				 p_buf		  Pointer to the buffer into which the bytes will be stored.
*
*				 nbr_bytes	  Number of bytes to be write.
*
* Return(s)   :  DEF_OK 	  If all bytes were write.
*				 DEF_FAIL	  If all bytes could not be write.
*
* Caller(s)   :  Application
*
* Note(s)	  :  none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Wr (CPU_INT08U      i2c_id,
                            CPU_INT08U   i2c_addr,
                            CPU_INT08U  *p_buf,
                            CPU_INT16U   nbr_bytes)
{
 	CPU_BOOLEAN  err;
 
 	err = BSP_I2C_StartXfer(i2c_id,									   //�ú������в������
						    i2c_addr,
						    BSP_I2C_ACCESS_TYPE_WR, 
						    p_buf, nbr_bytes);    

    return err;
}




/*
*********************************************************************************************************
*                                         BSP_I2C_WrRd()
*
* Description :  ��������дһ���ֽڻ��������ֽڣ�Ȼ�����·�����ʼ�źŴ������϶�ȡn���ֽڣ����һ��������ֽڱ��������ݸ���.
*
* Argument(s) :  i2c_id       BSP_I2C_ID_x              x = [1,3]
*
*                i2c_addr     The I2C device address
*
*                p_buf        Pointer to the buffer into which the bytes will be stored.
*
*                nbr_bytes    Number of bytes to be read.
*
* Return(s)   :  DEF_OK       If all bytes were read.
*                DEF_FAIL     If all bytes could not be read.
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_WrRd (CPU_INT08U       i2c_id,
                               CPU_INT08U   i2c_addr,
                               CPU_INT08U  *p_buf,
                               CPU_INT16U   nbr_bytes)
{
    CPU_BOOLEAN  err;
                             
    err = BSP_I2C_StartXfer(i2c_id,                                    //�ú������в������
                            i2c_addr,
                            BSP_I2C_ACCESS_TYPE_WR_RD, 
                            p_buf, nbr_bytes); 

    return err;
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
*                                        BSP_I2C_GPIO_Init()
*
* Description : ��ʼ�� GPIOӳ�䵽I2C����
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2C_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_I2C_GPIO_Init (CPU_INT08U i2c_id)
{
	GPIO_InitTypeDef gpioInit_SCL, gpioInit_SDA;
	GPIO_TypeDef *gpioPortSCL, *gpioPortSDA;
	
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return;
	
	switch (i2c_id) {
		case BSP_I2C_ID_1:
			gpioPortSDA = I2C1_GPIO_PORT_SDA;
		    gpioPortSCL = I2C1_GPIO_PORT_SCL;
			gpioInit_SDA.GPIO_Pin = I2C1_SDA_PIN;
			gpioInit_SCL.GPIO_Pin = I2C1_SCL_PIN;
			RCC_AHB1PeriphClockCmd(I2C1_GPIO_SDA_RCC, ENABLE);
			RCC_AHB1PeriphClockCmd(I2C1_GPIO_SCL_RCC, ENABLE);
			GPIO_PinAFConfig(gpioPortSDA, I2C1_SDA_PIN_SOURCE, GPIO_AF_I2C1);
			GPIO_PinAFConfig(gpioPortSCL, I2C1_SCL_PIN_SOURCE, GPIO_AF_I2C1);
			break;
		
		case BSP_I2C_ID_2:
			gpioPortSDA = I2C2_GPIO_PORT_SDA;
		    gpioPortSCL = I2C2_GPIO_PORT_SCL;
			gpioInit_SDA.GPIO_Pin = I2C2_SDA_PIN;
			gpioInit_SCL.GPIO_Pin = I2C2_SCL_PIN;
			RCC_AHB1PeriphClockCmd(I2C2_GPIO_SDA_RCC, ENABLE);
			RCC_AHB1PeriphClockCmd(I2C2_GPIO_SCL_RCC, ENABLE);
			GPIO_PinAFConfig(gpioPortSDA, I2C2_SDA_PIN_SOURCE, GPIO_AF_I2C2);
			GPIO_PinAFConfig(gpioPortSCL, I2C2_SCL_PIN_SOURCE, GPIO_AF_I2C2);
			break;
		
		case BSP_I2C_ID_3:
			gpioPortSDA = I2C3_GPIO_PORT_SDA;
		    gpioPortSCL = I2C3_GPIO_PORT_SCL;
			gpioInit_SDA.GPIO_Pin = I2C3_SDA_PIN;
			gpioInit_SCL.GPIO_Pin = I2C3_SCL_PIN;
			RCC_AHB1PeriphClockCmd(I2C3_GPIO_SDA_RCC, ENABLE);
			RCC_AHB1PeriphClockCmd(I2C3_GPIO_SCL_RCC, ENABLE);
			GPIO_PinAFConfig(gpioPortSDA, I2C3_SDA_PIN_SOURCE, GPIO_AF_I2C3);
			GPIO_PinAFConfig(gpioPortSCL, I2C3_SCL_PIN_SOURCE, GPIO_AF_I2C3);
			break;
		
		default:
			break;
	}
	
	gpioInit_SDA.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_SDA.GPIO_OType = GPIO_OType_OD;
	gpioInit_SDA.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit_SDA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortSDA, &gpioInit_SDA);
	
	gpioInit_SCL.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_SCL.GPIO_OType = GPIO_OType_OD;
	gpioInit_SCL.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit_SCL.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortSCL, &gpioInit_SCL);
}


/*
*********************************************************************************************************
*                                        NVIC_Configuration()
*
* Description : ��ʼ�� I2C ���ӵ�NVIC
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2C_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  NVIC_Configuration (CPU_INT08U i2c_id)
{
	NVIC_InitTypeDef nvicInitEV, nvicInitER;
  
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return;
	
	switch (i2c_id) {                                                           /* �����ж�Դ                      */
		case BSP_I2C_ID_1:
			nvicInitEV.NVIC_IRQChannel = BSP_I2C1_EV_IRQ;
			nvicInitER.NVIC_IRQChannel = BSP_I2C1_ER_IRQ;
			break;
		
		case BSP_I2C_ID_2:
			nvicInitEV.NVIC_IRQChannel = BSP_I2C2_EV_IRQ;
			nvicInitER.NVIC_IRQChannel = BSP_I2C2_ER_IRQ;
			break;
		
		case BSP_I2C_ID_3:
			nvicInitEV.NVIC_IRQChannel = BSP_I2C3_EV_IRQ;
			nvicInitER.NVIC_IRQChannel = BSP_I2C3_ER_IRQ;
			break;
		
		default:
			break;
	}         

	nvicInitEV.NVIC_IRQChannelPreemptionPriority = BSP_I2C_NVIC_PRE_PRIO;         /* �������ȼ�                      */
	nvicInitEV.NVIC_IRQChannelSubPriority        = BSP_I2C_NVIC_SUB_PRIO;         /* �����ȼ�                        */
	nvicInitEV.NVIC_IRQChannelCmd                = ENABLE;                        /* ʹ���ж�                        */
	NVIC_Init(&nvicInitEV);
	
	nvicInitER.NVIC_IRQChannelPreemptionPriority = BSP_I2C_NVIC_PRE_PRIO;         /* �������ȼ�                      */
	nvicInitER.NVIC_IRQChannelSubPriority        = BSP_I2C_NVIC_SUB_PRIO;         /* �����ȼ�                        */
	nvicInitER.NVIC_IRQChannelCmd                = ENABLE;                        /* ʹ���ж�                        */
	NVIC_Init(&nvicInitER);
}



/*
*********************************************************************************************************
*                                        BSP_I2C_SEM_Creat()
*
* Description : ����id������Ӧ���ź���
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : DEF_OK     �����ɹ�
*               DEF_FAIL   ����ʧ��
*
* Caller(s)   : BSP_I2C_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_I2C_SEM_Creat (CPU_INT08U i2c_id)
{
	CPU_ERR  err;
	BSP_I2C_PERIPH_STATUS *currentStatusPtr;
	
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return DEF_FAIL;
	
	currentStatusPtr = i2c_id+BSP_I2C_PeriphTbl;
	
	OSSemCreate((OS_SEM    *)&(currentStatusPtr->SemLock),
                (CPU_CHAR  *)"I2C SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	OSSemCreate((OS_SEM    *)&(currentStatusPtr->SemWait),
                (CPU_CHAR  *)"I2C SemWait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
	
		if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	return DEF_OK;
}


/*
*********************************************************************************************************
*                                       BSP_I2C_StartXfer()
*
* Description : Initialize and Start a new transfer in the I2C bus.
*
* Argument(s) : i2c_id             BSP_I2C_ID_x   x=[1,3]
*
*               i2c_addr           The I2C device address
*
*               i2c_acess_type     I2C Access Type
*                                      BSP_I2C_ACCESS_TYPE_RD
*                                      BSP_I2C_ACCESS_TYPE_WR
*                                      BSP_I2C_ACCESS_TYPE_WR_RD
*                               
*               p_buf              Pointer to the buffer into which the bytes will be stored.
*
*               nbr_bytes          Number of bytes to read.
*
* Return(s)   : DEF_OK            If the transfer could be initialized and started 
*               DEF_FAIL          If the transfer could no bet initialized and started
*
* Caller(s)   : BSP_I2C_Rd()
*               BSP_I2C_Wr()
*               BSP_I2C_WrRd()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  CPU_BOOLEAN  BSP_I2C_StartXfer (CPU_INT08U              i2c_id,
                                        CPU_INT08U              i2c_addr,
                                        I2C_AccessType_TypeDef  i2c_access_type,
                                        CPU_INT08U             *p_buf,                    
                                        CPU_INT16U              nbr_bytes)
{
	CPU_BOOLEAN           state = DEF_FAIL;
	OS_ERR                err;
	I2C_TypeDef           *i2cPort;
	BSP_I2C_PERIPH_STATUS *i2cPeriphStatusPtr;                                          //��ǰ�˿�״ָ̬��

	if(0 == nbr_bytes)                                                                  //�������
		return state;

    if(0 == p_buf)
        return state;
    
	switch (i2c_id) {                                                                   //ѡ���˿ں�
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			break;
		
		default:
			return DEF_FAIL;                                                            //�����ж�
	}
	
	i2cPeriphStatusPtr = BSP_I2C_PeriphTbl+i2c_id;                                      //��Ӧ����ǰ�˿�ʹ�õ�״̬�ṹ
	
	OSSemPend((OS_SEM *)&(i2cPeriphStatusPtr->SemLock),                                 //�����ö˿�
	          (OS_TICK ) BSP_I2C_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
	i2cPeriphStatusPtr->AccessType = i2c_access_type;
	i2cPeriphStatusPtr->Addr       = i2c_addr;
	i2cPeriphStatusPtr->BufLen     = nbr_bytes;
	i2cPeriphStatusPtr->BufPtr     = p_buf;
	i2cPeriphStatusPtr->State      = BSP_I2C_STATE_START; 
	
	I2C_ITConfig(i2cPort, I2C_IT_EVT, ENABLE);                                          //ʹ���ж�
	I2C_ITConfig(i2cPort, I2C_IT_ERR, ENABLE);
	I2C_GenerateSTART(i2cPort,ENABLE);                                                  //������ʼ�ź�
	
	OSSemPend((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                                 //�����ö˿�
	          (OS_TICK ) BSP_I2C_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(0 == i2cPeriphStatusPtr->BufLen)                                                //��ʾ���쳣
		state = DEF_OK;

	OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemLock),
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
	return state;
}




/*
*********************************************************************************************************
*                                        BSP_I2Cx_EV_Handler()
*
* Description : ����id������Ӧ���¼�
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2Cx_EV_Handler()     x=[1,3]
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_I2Cx_EV_Handler (CPU_INT08U i2c_id)
{
	OS_ERR                err;
	I2C_TypeDef           *i2cPort;
	BSP_I2C_PERIPH_STATUS *i2cPeriphStatusPtr;                                          //��ǰ�˿�״ָ̬��
	
	switch (i2c_id) {                                                                   //ѡ���˿ں�
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			break;
		
		default:
			return;                                                                     //�����ж�
	}
	
	i2cPeriphStatusPtr = BSP_I2C_PeriphTbl+i2c_id;                                      //��Ӧ����ǰ�˿�ʹ�õ�״̬�ṹ
	
	switch(i2cPeriphStatusPtr->State) {
		case BSP_I2C_STATE_START:   
			if(SET == I2C_GetITStatus(i2cPort, I2C_IT_SB)) {                            //��ʼλ���ͺ��¼�
				if(BSP_I2C_ACCESS_TYPE_RD == i2cPeriphStatusPtr->AccessType) {          //�˿ڶ�����
					I2C_Send7bitAddress(i2cPort, 
					                    i2cPeriphStatusPtr->Addr, 
					                    I2C_Direction_Receiver);                        //���Ͷ���ַ
				} else {                                                                                            
					I2C_Send7bitAddress(i2cPort, 
					                    i2cPeriphStatusPtr->Addr, 
					                    I2C_Direction_Transmitter);                     //����д��ַ
				}
				i2cPeriphStatusPtr->State = BSP_I2C_STATE_ADDR;                         //�����ַ�ѷ���״̬
			}
			break;
		
		case BSP_I2C_STATE_ADDR:
			if(SET == I2C_GetITStatus(i2cPort, I2C_IT_ADDR)) {                          //��ַ���ͺ�ӻ���ӦACK�¼�
				switch (i2cPeriphStatusPtr->AccessType) {
					case BSP_I2C_ACCESS_TYPE_RD:
						if(i2cPeriphStatusPtr->BufLen > 2) {                            //���մ��������ֽ�Ҫ����ʱ��ʹ��ACK
                            I2C_AcknowledgeConfig(i2cPort, ENABLE);                     
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_DATA;             //״̬�л�׼����������
						} else { 
                            if(2 == i2cPeriphStatusPtr->BufLen) {                           //��ʾֻ������������
							    I2C_AcknowledgeConfig(i2cPort, DISABLE);                    //ʧ��Ӧ��
							    I2C_NACKPositionConfig(i2cPort,I2C_NACKPosition_Next);      //ֻ���������ֽ�ʱ������λPOS
							    i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;             //�л��������׶βμ�STM32F4�ο��ֲ�
                             } else {                                                       //ֻ��ȡһ���ֽ�,ע�ⲻ����������λSTOP,�ֲ�˴��е�ģ��
                                I2C_ITConfig(i2cPort,I2C_IT_BUF, ENABLE);                   //����ʹ��TXE�жϣ���Ϊ���ֽڽ��ղ�����λBTF
                                I2C_AcknowledgeConfig(i2cPort, DISABLE);                    //ʧ��Ӧ��
                                i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;
                             }
						} 
						I2C_ReadRegister(i2cPort, I2C_Register_SR2);                        //����ADDR��־λ,�˴����ղο��ֲ�˳��	
						break;
 
					case BSP_I2C_ACCESS_TYPE_WR_RD:
						I2C_ReadRegister(i2cPort, I2C_Register_SR2);                        //�ڷ�������ǰ����������ADDR��־λ�������ܽ����ж�	
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
							I2C_SendData(i2cPort, *(i2cPeriphStatusPtr->BufPtr));           //���͵�һ������
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_DATA;
						}
						break;
					
					case BSP_I2C_ACCESS_TYPE_WR:
						I2C_ReadRegister(i2cPort, I2C_Register_SR2);                        //�ڷ�������ǰ����������ADDR��־λ�������ܽ����ж�	
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
							I2C_SendData(i2cPort, *(i2cPeriphStatusPtr->BufPtr));           //���͵�һ������
							i2cPeriphStatusPtr->BufLen--;
							i2cPeriphStatusPtr->BufPtr++;
							if(0 != i2cPeriphStatusPtr->BufLen) {                           //�ж��Ƿ�ֻ����һ���ֽ�
								i2cPeriphStatusPtr->State = BSP_I2C_STATE_DATA;
							} else {                                                        
								i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;   
							}
						}
						break;
						
					default:
						break;
				}	
			} else {                                                                   //��Ԥ���������
				i2cPeriphStatusPtr->State      = BSP_I2C_STATE_IDLE;
				i2cPeriphStatusPtr->AccessType = BSP_I2C_ACCESS_TYPE_NONE;	
                I2C_ITConfig(i2cPort,I2C_IT_EVT, DISABLE);
			    I2C_ITConfig(i2cPort,I2C_IT_ERR, DISABLE);
                I2C_ITConfig(i2cPort,I2C_IT_BUF, DISABLE);
                I2C_GenerateSTOP(i2cPort, ENABLE);
                OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                    //�ͷ��ź���
	                      (OS_OPT  )OS_OPT_POST_1,
	                      (OS_ERR *) &err);
			}
			break;
		
		case BSP_I2C_STATE_DATA:
			switch (i2cPeriphStatusPtr->AccessType) {
				case BSP_I2C_ACCESS_TYPE_RD:	
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_BTF)) {
						*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);      //��ȡ����
						i2cPeriphStatusPtr->BufLen--;
						i2cPeriphStatusPtr->BufPtr++; 
						if(2 == i2cPeriphStatusPtr->BufLen) {                          //��ʾ���ܽ�����������ֽ�
							I2C_AcknowledgeConfig(i2cPort, DISABLE);
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;
						} 
					}
					break;
				
				case BSP_I2C_ACCESS_TYPE_WR:
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
						I2C_SendData(i2cPort, *(i2cPeriphStatusPtr->BufPtr));          //д����
						i2cPeriphStatusPtr->BufLen--;
						i2cPeriphStatusPtr->BufPtr++;
						if(0 == i2cPeriphStatusPtr->BufLen) {
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;            //��ʾ����
						}
					}
					break;

                case BSP_I2C_ACCESS_TYPE_WR_RD:
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
                        i2cPeriphStatusPtr->AccessType = BSP_I2C_ACCESS_TYPE_RD;       //���ɵ���ģʽ
                        i2cPeriphStatusPtr->State      = BSP_I2C_STATE_START;          //׼�����뿪ʼ״̬
						I2C_GenerateSTART(i2cPort, ENABLE);
					}
					break;
					
				default:
					break;
			}
			break;
         		
   		case BSP_I2C_STATE_STOP:
   			switch (i2cPeriphStatusPtr->AccessType) {
   				case BSP_I2C_ACCESS_TYPE_RD:
					if(2 == i2cPeriphStatusPtr->BufLen) {                                 //���������ʽһ�ζ�ȡ�������ݲμ��ֲ�
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_BTF)) {
							I2C_GenerateSTOP(i2cPort, ENABLE);                            //�����ֲ�˳����λSTOP
							*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);     //��ȡ�����ڶ����ֽ�
							i2cPeriphStatusPtr->BufPtr++;      
							*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);     //��ȡ���һ���ֽ�
							i2cPeriphStatusPtr->BufLen -= 2;                              //�ݼ�����������
						}
					} else {                                                              //ֻ�ж�ȡ���ֽ�ʱ�Żᷢ��
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_RXNE)) {                //���ֽ�ģʽ������λBTF
							I2C_GenerateSTOP(i2cPort, ENABLE);                            
							*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);      
							i2cPeriphStatusPtr->BufLen--;       
							I2C_ITConfig(i2cPort,I2C_IT_BUF, DISABLE);                    //ʧ��TXE��RXNE�ж�
						}
					}
					break;
		
			    case BSP_I2C_ACCESS_TYPE_WR:
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
						I2C_GenerateSTOP(i2cPort, ENABLE);
					}
					break;
			
				default:
					break;	                 
			}

			OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                           //֪ͨ��д������ȡ������
	                  (OS_OPT  )OS_OPT_POST_1,
	                  (OS_ERR *) &err);
			
			/*------------------------  �ر��ж�  ----------------------------*/
			I2C_ITConfig(i2cPort,I2C_IT_EVT, DISABLE);
			I2C_ITConfig(i2cPort,I2C_IT_ERR, DISABLE);
			break;
			
	default:
		break;
	}
}




/*
*********************************************************************************************************
*                                        BSP_I2Cx_ER_Handler()
*
* Description : ����id������Ӧ�Ĵ��󣬼����˿ڵ�״̬�ָ������ǲ�����BufLen
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2Cx_ER_Handler()     x=[1,3]
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_I2Cx_ER_Handler (CPU_INT08U i2c_id)
{
	OS_ERR                err;
	CPU_INT32U            status;
	I2C_TypeDef           *i2cPort;
	BSP_I2C_PERIPH_STATUS *i2cPeriphStatusPtr;                                        //��ǰ�˿�״ָ̬��
	
	switch (i2c_id) {                                                                 //ѡ���˿ں�
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			break;
		
		default:
			return;                                                                   //�����ж�
	}
	
	i2cPeriphStatusPtr = BSP_I2C_PeriphTbl+i2c_id; 
	
	status  = I2C_ReadRegister(i2cPort, I2C_Register_SR1);                            //��ȡSR1
	status &= BSP_I2C_REG_SR1_ERR_MASK;                                               //ֻ��������λ   
	i2cPort->SR1 &= ~status;                                                          //��������־λ

	 if (i2cPeriphStatusPtr->State != BSP_I2C_STATE_IDLE) {
        i2cPeriphStatusPtr->State      = BSP_I2C_STATE_IDLE;
        i2cPeriphStatusPtr->AccessType = BSP_I2C_ACCESS_TYPE_NONE;
		 
		I2C_GenerateSTOP(i2cPort, ENABLE);
                                       
		OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                           //�ͷ��ź���                    
	              (OS_OPT  )OS_OPT_POST_1,
	              (OS_ERR *) &err);
		 
		/*------------------------  �ر��ж�  ----------------------------*/
		I2C_ITConfig(i2cPort,I2C_IT_EVT, DISABLE);
        I2C_ITConfig(i2cPort,I2C_IT_BUF, DISABLE);
		I2C_ITConfig(i2cPort,I2C_IT_ERR, DISABLE);        	
    }
}



/*
*********************************************************************************************************
*                                              I2C1ϵͳ�¼��жϺ���������жϺ���
*********************************************************************************************************
*/

void BSP_I2C1_EV_Handler(void)
{
	BSP_I2Cx_EV_Handler(BSP_I2C_ID_1);
}

void BSP_I2C1_ER_Handler(void)
{
	BSP_I2Cx_ER_Handler(BSP_I2C_ID_1);
}


/*
*********************************************************************************************************
*                                              I2C2ϵͳ�¼��жϺ���������жϺ���
*********************************************************************************************************
*/

void BSP_I2C2_EV_Handler(void)
{
	BSP_I2Cx_EV_Handler(BSP_I2C_ID_2);
}

void BSP_I2C2_ER_Handler(void)
{
	BSP_I2Cx_ER_Handler(BSP_I2C_ID_2);
}



/*
*********************************************************************************************************
*                                              I2C3ϵͳ�¼��жϺ���������жϺ���
*********************************************************************************************************
*/

void BSP_I2C3_EV_Handler(void)
{
	BSP_I2Cx_EV_Handler(BSP_I2C_ID_3);
}

void BSP_I2C3_ER_Handler(void)
{
	BSP_I2Cx_ER_Handler(BSP_I2C_ID_3);
}


