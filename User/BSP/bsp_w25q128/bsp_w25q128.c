
#include "bsp_spi.h"
#include "bsp_w25q128.h"



/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


#ifdef    __USE_FS

static        volatile                  DSTATUS TM_FATFS_FLASH_SPI_Stat = STA_NOINIT;	/* Physical drive status */

#endif    /* __USE_FS */

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void  BSP_FLASH_WriteEnable(void);
static void  BSP_FLASH_WaitForWriteEnd(void);
static void  BSP_FLASH_PageWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_FLASH_Init()
*
* Description : ��ʼ���ص�FLASHоƬ�ĵײ�SPI�ӿ�
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_FLASH_Init(void) 
{	
	// ��ʼ��SPI�ײ�ӿ�
	
	BSP_SPIx_Init();
	
	BSP_Flash_WAKEUP();                                       // ����FLASH
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_ReadID()
*
* Description : ��ȡ����FLASH��IDֵ
*
* Argument(s) : none
*
* Return(s)   : ����FLASH��IDֵ
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/


uint32_t BSP_FLASH_ReadID(void)
{
	uint8_t  Temp[3];
	
	SPI_FLASH_CS_LOW();                                        // ����Ƭѡ��
	
	SPI_FLASH_SendByte(W25X_JedecDeviceID);                    // ����ָ��
    SPI_FLASH_ReadBuff(Temp, 3);                               // �������������Ч��

	SPI_FLASH_CS_HIGH();                                       // ����Ƭѡ
			  
    return ((uint32_t)Temp[0] << 16| (uint32_t)Temp[1] << 8| (uint32_t)Temp[2]);
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_ReadDeviceID()
*
* Description : ��ȡ�豸��ַ
*
* Argument(s) : none
*
* Return(s)   : 32λ���豸ID
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/


uint32_t BSP_FLASH_ReadDeviceID(void)
{
	uint32_t  Temp = 0;
	
    SPI_FLASH_CS_LOW();

    // ���Ͷ�ȡ�豸IDָ����Ҷ�ȡID
    SPI_FLASH_SendByte(W25X_DeviceID);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
	Temp = BSP_SPI_ReadByte();

    SPI_FLASH_CS_HIGH();

	return Temp;
}



/*
*********************************************************************************************************
*                                             BSP_FLASH_SectorErase()
*
* Description : ������������
*
* Argument(s) : SectorAddr : ������ַ
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/

void BSP_FLASH_SectorErase(uint32_t SectorAddr)
{
	BSP_FLASH_WriteEnable();
	BSP_FLASH_WaitForWriteEnd();
	
	SPI_FLASH_CS_LOW();
  
	// ������������ָ��
	
	SPI_FLASH_SendByte(W25X_SectorErase);
	
	// ����Ҫ������������ַ
	
	SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(SectorAddr & 0xFF);
  
	SPI_FLASH_CS_HIGH();
 
	// �ȴ���������
	
	BSP_FLASH_WaitForWriteEnd();
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_BulkErase()
*
* Description : ��������FLASH
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/


void BSP_FLASH_BulkErase(void)
{
	OS_ERR   err;
	
	BSP_FLASH_WriteEnable();
	BSP_FLASH_WaitForWriteEnd();
	
	SPI_FLASH_CS_LOW();
	
	SPI_FLASH_SendByte(W25X_ChipErase);                        // ����ָ��
	
	SPI_FLASH_CS_HIGH();
	
	// ����FLASH����ʱ����Ϊ25s
	
	OSTimeDlyHMSM( 0, 0, 25, 0,
		           OS_OPT_TIME_HMSM_STRICT,
                   &err );
	
	// ȷ����������
	
	BSP_FLASH_WaitForWriteEnd();
}


/*
*********************************************************************************************************
*                                             BSP_Flash_WAKEUP()
*
* Description : �ӵ���ģʽ����Flash
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/

void BSP_Flash_WAKEUP(void)   
{
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_ReleasePowerDown);

	SPI_FLASH_CS_HIGH();                                       
}   


/*
*********************************************************************************************************
*                                             BSP_Flash_PowerDown()
*
* Description : ʹFlash�������ģʽ
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/

void BSP_Flash_PowerDown(void)   
{ 
	
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_PowerDown);

	SPI_FLASH_CS_HIGH();
}   




/*
*********************************************************************************************************
*                                             BSP_FLASH_BufferWrite()
*
* Description : FLASH ����д��
*
* Argument(s) : pBuffer         :Ҫд��Ļ�����
*               WriteAddr       :Ҫд��ĵ�ַ
*               NumByteToWrite  :Ҫд����ֽ���
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/


void BSP_FLASH_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t  NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = WriteAddr % SPI_FLASH_PageSize;
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    if (Addr == 0) {                                                              // ������ַҳ�����
		if (NumOfPage == 0) {
			BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
		} else {                                                                  // ��ʾ������ҳ
		
			// д������ҳ
			
			while (NumOfPage--) {                                                 // д����ҳ
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
		}
	} else {                                                                      // ������ַδҳ�����
		if (NumOfPage == 0) {                                                     // û������ҳ
			if (NumOfSingle > count) {
				temp = NumOfSingle - count;
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, count);
				WriteAddr +=  count;
				pBuffer += count;
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, temp);
			} else {
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
			}
		} else {                                                                  // ������ҳ
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			BSP_FLASH_PageWrite(pBuffer, WriteAddr, count);
			WriteAddr +=  count;
			pBuffer += count;

			// д������ҳ
			
			while (NumOfPage--) {                      
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			if (NumOfSingle != 0) {
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_BufferRead()
*
* Description : FLASH ��������
*
* Argument(s) : pBuffer         :Ҫд��Ļ�����
*               WriteAddr       :Ҫ���ĵ�ַ
*               NumByteToWrite  :Ҫ�����ֽ���
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/


void BSP_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    SPI_FLASH_CS_LOW();

	// ���Ͷ�ָ����ҷ��Ͷ���ַ
	
    SPI_FLASH_SendByte(W25X_ReadData);
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

	// ��ȡ������
	
	SPI_FLASH_ReadBuff(pBuffer, NumByteToRead);

	SPI_FLASH_CS_HIGH();
}



/*
*********************************************************************************************************
*********************************************************************************************************
**                                        �ļ�ϵͳ�ӿں���
*********************************************************************************************************
*********************************************************************************************************
*/

#ifdef    __USE_FS

/*
*********************************************************************************************************
*                                             TM_FATFS_FLASH_SPI_disk_status()
*
* Description : ���ļ�ϵͳ��ȡ�������豸����״̬
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : TM_FATFS_FLASH_SPI_disk_initialize();
*
* Note(s)     : 
*********************************************************************************************************
*/


DSTATUS TM_FATFS_FLASH_SPI_disk_status(void)
{
	if(sFLASH_ID == BSP_FLASH_ReadID())			        
	{
		return TM_FATFS_FLASH_SPI_Stat &= ~STA_NOINIT;	        // ���  STA_NOINIT flag 
	}else
	{
		return TM_FATFS_FLASH_SPI_Stat |= STA_NOINIT;
	}
}


/*
*********************************************************************************************************
*                                             TM_FATFS_FLASH_SPI_disk_initialize()
*
* Description : ��ʼ��FLASH�豸
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 
*
* Note(s)     : 
*********************************************************************************************************
*/

DSTATUS TM_FATFS_FLASH_SPI_disk_initialize(void) 
{
	// ��ʼ���ײ��豸
	
	BSP_FLASH_Init();
	
	// �����豸״̬
	
	return TM_FATFS_FLASH_SPI_disk_status(); 
}



/*
*********************************************************************************************************
*                                             TM_FATFS_FLASH_SPI_disk_ioctl()
*
* Description : ��ȡ������Ϣ
*
* Argument(s) : 
*
* Return(s)   : 
*
* Caller(s)   : 
*
* Note(s)     : 
*********************************************************************************************************
*/

DRESULT TM_FATFS_FLASH_SPI_disk_ioctl(BYTE cmd, char *buff)
{
	switch (cmd) {
		case GET_SECTOR_COUNT:
			*(DWORD * )buff = 2560;		//sector����   
			break;
		
		case GET_SECTOR_SIZE :          // Get R/W sector size (WORD)
			*(WORD * )buff = 4096;		
			break;
		
		case GET_BLOCK_SIZE :           // Get erase block size in unit of sector (DWORD)
			*(DWORD * )buff = 1;		//flash��1��sectorΪ��С������λ
			break;
		
		case CTRL_ERASE_SECTOR:
			break;
		
		case CTRL_SYNC :
			break;
	}
	
	return RES_OK;
}


/*
*********************************************************************************************************
*                                             TM_FATFS_FLASH_SPI_disk_read()
*
* Description : ���̶�����
*
* Argument(s) : buff     : ������ָ��
*               sector   : ������
*               count    : ��������
*
* Return(s)   : RES_OK
*
* Caller(s)   : FatFs
*
* Note(s)     : 
*********************************************************************************************************
*/



DRESULT TM_FATFS_FLASH_SPI_disk_read(BYTE *buff, DWORD sector, UINT count)
{
	if ((TM_FATFS_FLASH_SPI_Stat & STA_NOINIT)) 
	{
		return RES_NOTRDY;
	}
//	sector+=1536;//����ƫ�ƣ��ⲿFlash�ļ�ϵͳ�ռ�����ⲿFlash����6M�ռ�
	
	BSP_FLASH_BufferRead(buff, sector <<12, count<<12);
	
	return RES_OK;
}



/*
*********************************************************************************************************
*                                             TM_FATFS_FLASH_SPI_disk_write()
*
* Description : ����д����
*
* Argument(s) : buff     : ������ָ��
*               sector   : ������
*               count    : д�ĳ���
*
* Return(s)   : RES_OK
*
* Caller(s)   : FatFs
*
* Note(s)     : 
*********************************************************************************************************
*/

DRESULT TM_FATFS_FLASH_SPI_disk_write(BYTE *buff, DWORD sector, UINT count)
{
	uint32_t write_addr;  

//	sector+=1536;//����ƫ�ƣ��ⲿFlash�ļ�ϵͳ�ռ�����ⲿFlash����4M�ռ�
	
	write_addr = sector<<12;    
	BSP_FLASH_SectorErase(write_addr);
	BSP_FLASH_BufferWrite(buff,write_addr,4096);
	
	return RES_OK;
}

#endif   /* __USE_FS */




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_FLASH_WriteEnable()
*
* Description : Flashдʹ��
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_PageWrite(), BSP_FLASH_SectorErase()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_FLASH_WriteEnable(void)
{
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_WriteEnable);

	SPI_FLASH_CS_HIGH();
}



 

/*
*********************************************************************************************************
*                                             BSP_FLASH_WaitForWriteEnd()
*
* Description : оƬæ�ȴ���������д�붼�Ǻ�ʱ�ģ��ú����ǵȴ�FlashоƬ����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_SectorErase(), BSP_FLASH_PageWrite()
*
* Note(s)     : �ú����������ʱ����
*********************************************************************************************************
*/


static void BSP_FLASH_WaitForWriteEnd(void)
{
	OS_ERR   err;
	uint8_t  FLASH_Status = 0;
	
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_ReadStatusReg);                      // ��ȡ״̬������ָ��
	
	do {
		// ��ʱ1ms,��ΪFLASHҳ���ʱ�����ֵ��0.7ms
		
		OSTimeDly((OS_TICK )  1, 
				  (OS_OPT  )  OS_OPT_TIME_DLY, 
				  (OS_ERR *)& err);
		
		FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);           //��ȡ״̬�Ĵ���
		
	} while (SET == (FLASH_Status & WIP_Flag));
	
	SPI_FLASH_CS_HIGH();
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_PageWrite()
*
* Description : FLASH ҳд�뺯��
*
* Argument(s) : pBuffer         :Ҫд��Ļ�����
*               WriteAddr       :Ҫд��ĵ�ַ
*               NumByteToWrite  :Ҫд����ֽ���
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_BufferWrite()
*
* Note(s)     : none
*********************************************************************************************************
*/

static void BSP_FLASH_PageWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	BSP_FLASH_WriteEnable();

	SPI_FLASH_CS_LOW();
	
	// ����ҳ���ָ��
	
	SPI_FLASH_SendByte(W25X_PageProgram);
	  
	// ����ҳ��ַ
	
	SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(WriteAddr & 0xFF);

	// �ж�д��ʱ�򳬹�һҳ
	
	if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
	}
	
	// ������������д��
	
	SPI_FLASH_WriteBuff(pBuffer, NumByteToWrite);              
	
	SPI_FLASH_CS_HIGH();

	// �ȴ�Ҳд�����
	
	BSP_FLASH_WaitForWriteEnd();
}


