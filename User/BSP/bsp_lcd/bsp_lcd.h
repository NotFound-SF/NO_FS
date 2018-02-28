
#ifndef     __BSP_LCD_H
#define     __BSP_LCD_H

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f4xx.h"


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define    BSP_LCD_Bank                              FSMC_Bank1_NORSRAM4

// ʹ�õ���A12��Ϊ��������

#define    BSP_LCD_DAT_BASE                          (uint32_t)0x6C002000
#define    BSP_LCD_CMD_BASE                          (uint32_t)0x6C000000

#define    BSP_LCD_CMD                               (*(__IO uint16_t *)BSP_LCD_CMD_BASE)  
#define    BSP_LCD_DAT                               (*(__IO uint16_t *)BSP_LCD_DAT_BASE)

/*
*********************************************************************************************************
*                                               ��ɫ����
*********************************************************************************************************
*/

#define      WHITE		 		           0xFFFF	   //��ɫ
#define      BLACK                         0x0000	   //��ɫ 
#define      GREY                          0xF7DE	   //��ɫ 
#define      BLUE                          0x001F	   //��ɫ 
#define      BLUE2                         0x051F	   //ǳ��ɫ 
#define      RED                           0xF800	   //��ɫ 
#define      MAGENTA                       0xF81F	   //����ɫ�����ɫ 
#define      GREEN                         0x07E0	   //��ɫ 
#define      CYAN                          0x7FFF	   //����ɫ����ɫ 
#define      YELLOW                        0xFFE0	   //��ɫ 
#define      BRED                          0xF81F
#define      GRED                          0xFFE0
#define      GBLUE                         0x07FF


/*
*********************************************************************************************************
*                                               �����
*********************************************************************************************************
*/
#define      CMD_SetCoordinateX		 		         0x2A	     //����X����
#define      CMD_SetCoordinateY		 		         0x2B	     //����Y����
#define      CMD_SetPixel		 		             0x2C	     //�������
#define      CMD_GetPixel                            0x2E        //��ȡGRAM


/***************************** ILI934 ��ʾ�������ʼ������������� ***************************/
#define      ILI9341_DispWindow_X_Star		         0           //��ʼ���X����
#define      ILI9341_DispWindow_Y_Star		         0           //��ʼ���Y����

#define 	 ILI9341_LESS_PIXEL	  		        	 240	     //Һ�����϶̷�������ؿ��
#define 	 ILI9341_MORE_PIXEL	 		        	 320	     //Һ�����ϳ���������ؿ��
#define      ILI9341_ALL_PIXEL                       76800       //���ص���� > 65535



/*
*********************************************************************************************************
*                                               LCD�������Ŷ���
*********************************************************************************************************
*/

/*-----------------------------  ��ֲʱ��Ҫ�޸ĸ����Ŷ���  ------------------------*/
#define    LCD_GPIO_PORT_NCE                     GPIOG
#define    LCD_GPIO_PORT_RCC_NCE                 RCC_AHB1Periph_GPIOG
#define    LCD_PIN_NCE                           GPIO_Pin_12  
#define    LCD_PIN_SOURCE_NCE                    GPIO_PinSource12

#define    LCD_GPIO_PORT_BL                      GPIOF
#define    LCD_GPIO_PORT_RCC_BL                  RCC_AHB1Periph_GPIOF
#define    LCD_PIN_BL                            GPIO_Pin_10  
#define    LCD_PIN_SOURCE_BL                     GPIO_PinSource10

// LCD�������

#define    BSP_LCD_BL_ON()                       (LCD_GPIO_PORT_BL->BSRRL = LCD_PIN_BL)
#define    BSP_LCD_BL_OFF()                      (LCD_GPIO_PORT_BL->BSRRH = LCD_PIN_BL)


// DMA���

#define         LCD_DMA_EN                       0          // ����0��ʾʹ��DMA

#if  LCD_DMA_EN > 0

#define         LCD_DMA_ID                       DMA2
#define         LCD_DMA_CTC_MASK                 DEF_BIT_11
#define         LCD_DMA_FTC_MASK                 DEF_BIT_11
#define         LCD_IFCR                         LIFCR 
#define         LCD_ISR                          LISR 
#define         LCD_DMA_CLK                      RCC_AHB1Periph_DMA2
#define         LCD_DMA_CHANNEL                  DMA_Channel_0
#define         LCD_DMA_STREAM                   DMA2_Stream1

#endif /* LCD_DMA_EN */





/*
*********************************************************************************************************
*                                               FUNCTION
*********************************************************************************************************
*/

void       BSP_LCD_Init       (void);
uint16_t   LCD_GetHeight      (void);
uint16_t   LCD_GetWidth       (void);

void       BSP_LCD_ClrScr(uint16_t usColor); 
uint16_t   BSP_LCD_GetPointPixel (uint16_t usX, uint16_t usY);
void       BSP_LCD_SetPointPixel (uint16_t usX, uint16_t usY, uint16_t usColor);
void       BSP_LCD_DrawHLine(uint16_t usX1 , uint16_t usY1 , uint16_t usX2 , uint16_t usColor);
void       BSP_LCD_DrawVLine(uint16_t usX1 , uint16_t usY1 , uint16_t usY2 , uint16_t usColor);
void       BSP_LCD_FillRect(uint16_t usX, uint16_t usY, uint16_t usHeight, uint16_t usWidth, uint16_t usColor);
void       BSP_LCD_DrawHColorLine(uint16_t usX1 , uint16_t usY1, uint16_t usWidth, const uint16_t *pColor);


#endif      /* __BSP_LCD_H */











