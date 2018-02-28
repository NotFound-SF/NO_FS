


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_touch.h"


/*
*********************************************************************************************************
*                                             �˲���صĺ�
*********************************************************************************************************
*/
#define   SAMP_CNT             4
#define   SAMP_CNT_DIV2        2


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void                   XPT2046_GPIO_SPI_Config               ( void );

static void                   XPT2046_DelayUS                       ( __IO uint32_t ulCount );
static void                   XPT2046_WriteCMD                      ( uint8_t ucCmd );
static uint16_t               XPT2046_ReadCMD                       ( void );




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        XPT2046_Init()
*
* Description : ��ʼ��XPT2046��������оƬ�Ķ�Ӧ����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

void XPT2046_Init ( void )
{
	XPT2046_GPIO_SPI_Config ();		
}



/*
*********************************************************************************************************
*                                        XPT2046_ReadAdc_Fliter()
*
* Description : ѡ��һ��ģ��ͨ��������ADC��������ADC�������
*
* Argument(s) : ucCh = 0x90 ��ʾYͨ���� 0xd0 ��ʾXͨ��
*
* Return(s)   : �˲����12λADCֵ
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint16_t XPT2046_ReadAdc_Fliter(uint8_t channel)
{
	uint8_t   i,j,min;
    uint16_t  temp;
    uint16_t  tempXY[SAMP_CNT];
    static uint16_t adc_x = 0,adc_y = 0;

    if(TOUCH_PEN_RD()== macXPT2046_INT_ActiveLevel) {
		
		for(i = 0;i < SAMP_CNT;i++) {
			XPT2046_WriteCMD(channel);
			tempXY[i] = XPT2046_ReadCMD();     
        }
		
        // ��������
        for(i=0; i<SAMP_CNT-1; i++) {
			min=i;
           for (j=i+1; j<SAMP_CNT; j++) {
		       if (tempXY[min] > tempXY[j]) min=j;
		   }
		   
		   temp        = tempXY[i];
           tempXY[i]   = tempXY[min];
           tempXY[min] = temp;
		}
   
		// �趨��ֵ
	
		if((tempXY[SAMP_CNT_DIV2]-tempXY[SAMP_CNT_DIV2-1]) > 5) {
		
			// �������м�ֵ���̫Զ����������������ݣ�������һ�εĴ�������
		
			if(channel == macXPT2046_CHANNEL_Y )
				return adc_x;                                                //xͨ��
			else
				return adc_y;                                                //yͨ�� 
		}
    
		// ���м�ֵ�ľ�ֵ  
	
		if(channel == macXPT2046_CHANNEL_Y) {
			adc_x = (tempXY[SAMP_CNT_DIV2]+tempXY[SAMP_CNT_DIV2-1]) / 2;
			return adc_x;
		} else {
			adc_y = (tempXY[SAMP_CNT_DIV2]+tempXY[SAMP_CNT_DIV2-1]) / 2;
			return adc_y;
		}     
  }else {
    return 0;                                                                 //û�д���������0
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
*                                        XPT2046_GPIO_SPI_Config()
*
* Description : ��ʼ��XPT2046��������оƬ�Ķ�Ӧ����,���ó�ģ��SPI��Ӧ��ģʽ
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : XPT2046_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void XPT2046_GPIO_SPI_Config ( void ) 
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;
	

    // ����GPIOʱ�� 
	
	RCC_AHB1PeriphClockCmd(TOUCH_PEN_GPIO_CLK, ENABLE);             // �����ж�����ʱ��
	RCC_AHB1PeriphClockCmd(TOUCH_CS_GPIO_CLK, ENABLE);              // ����Ƭѡ����ʱ��
	RCC_AHB1PeriphClockCmd(TOUCH_CLK_GPIO_CLK, ENABLE);             // ����ģ��ʱ������ʱ��
	RCC_AHB1PeriphClockCmd(TOUCH_MOSI_GPIO_CLK, ENABLE);            // ����MOSI����ʱ��
	RCC_AHB1PeriphClockCmd(TOUCH_MISO_GPIO_CLK, ENABLE);            // ����MISO����ʱ��
	
 
	// ��ʼ�� CS, MOSI, CLK
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_CS_PIN;                    // CS����
	GPIO_Init(TOUCH_CS_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_MOSI_PIN;                  // MOSI����
	GPIO_Init(TOUCH_MOSI_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_CLK_PIN;                   // CLK����
	GPIO_Init(TOUCH_CLK_PORT, &GPIO_InitStructure);
	
	
	// ��ʼ�� MISO, PEN
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_MISO_PIN;                   // MISO����
	GPIO_Init(TOUCH_MISO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = TOUCH_PEN_PIN;                    // PEN����
	GPIO_Init(TOUCH_PEN_PORT, &GPIO_InitStructure);
	
  
    // ����Ƭѡ��ѡ��XPT2046 
	TOUCH_CS_ENABLE();
}



/*
*********************************************************************************************************
*                                        XPT2046_DelayUS()
*
* Description : ģ��SPI����Ҫ����ʱ����Ȼ�Ƕ�����ϵͳ������Ϊ����ʱ���źţ����Բ��±���Ľ��̴��
*
* Argument(s) : ulCount Ҫ��ʱ��΢����
*
* Return(s)   : none
*
* Caller(s)   : XPT2046_WriteCMD()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void XPT2046_DelayUS ( uint32_t ulCount )
{	uint8_t noUse;
	
	while(ulCount--) {
		noUse = 0x08;
		while(noUse--);
	}
}



/*
*********************************************************************************************************
*                                        XPT2046_WriteCMD()
*
* Description : SPIд����
*
* Argument(s) : ucCmd Ҫд���ָ��
*
* Return(s)   : none
*
* Caller(s)   : XPT2046_ReadAdc_Fliter()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void XPT2046_WriteCMD ( uint8_t ucCmd ) 
{
	uint8_t i;

	TOUCH_MOSI_0();
	TOUCH_CLK_LOW();

	for ( i = 0; i < 8; i ++ ) {
		if ((ucCmd >> (7 - i)) & 0x01) {
			TOUCH_MOSI_1();
		} else {
			TOUCH_MOSI_0();
		}
		
		XPT2046_DelayUS(5);		
	    TOUCH_CLK_HIGH();
	    XPT2046_DelayUS(5);
	    TOUCH_CLK_LOW();
	}
}


/*
*********************************************************************************************************
*                                        XPT2046_ReadCMD()
*
* Description : SPI������
*
* Argument(s) : none
*
* Return(s)   : ��ȡ��������
*
* Caller(s)   : XPT2046_ReadAdc_Fliter()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static uint16_t XPT2046_ReadCMD ( void ) 
{
	uint8_t i;
	uint16_t usBuf=0, usTemp;
	
	TOUCH_MOSI_0();
	TOUCH_CLK_HIGH();

	for ( i=0;i<12;i++ ) {
		TOUCH_CLK_LOW();    
		usTemp = TOUCH_MISO_RD();
		usBuf |= usTemp << ( 11 - i );
		TOUCH_CLK_HIGH();
	}
	
	return usBuf;
}



