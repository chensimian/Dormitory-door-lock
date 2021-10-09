#ifndef __LCD12864_H
#define __LCD12864_H

#include "stm32f1xx_hal.h"


 
/*▁▂▃▄▅▆▇█  ↓ 移植修改区域 ↓ █▇▆▅▄▃▂▁*/ 
 
#define LCD12864_GPIO   GPIOB
#define LCD12864_EN     GPIO_PIN_6  //这里是 LCD12864的 EN
#define LCD12864_RW     GPIO_PIN_7  //这里是 LCD12864的 RW
#define LCD12864_RS     GPIO_PIN_5//这里是 LCD12864的 RS

//如果换库，记得修改结构体名称，函数名称

/*▁▂▃▄▅▆▇█  ↑ 移植修改区域 ↑ █▇▆▅▄▃▂▁*/


// lcd_clk_0   lcd_data_1   lcd_cs_0  
//#define lcd_EN_1  GPIO_SetBits (LCD12864_GPIO, LCD12864_EN ) 		
//#define lcd_EN_0  GPIO_ResetBits(LCD12864_GPIO, LCD12864_EN )  	

//#define lcd_RW_1  GPIO_SetBits(LCD12864_GPIO, LCD12864_RW ) 		
//#define lcd_RW_0  GPIO_ResetBits(LCD12864_GPIO,LCD12864_RW)

//#define lcd_RS_1  GPIO_SetBits  (LCD12864_GPIO,LCD12864_RS) 		  
//#define lcd_RS_0  GPIO_ResetBits  (LCD12864_GPIO,LCD12864_RS)

#define lcd_EN_1  HAL_GPIO_WritePin(LCD12864_GPIO, LCD12864_EN, GPIO_PIN_SET)	
#define lcd_EN_0  HAL_GPIO_WritePin(LCD12864_GPIO, LCD12864_EN, GPIO_PIN_RESET)

#define lcd_RW_1 HAL_GPIO_WritePin(LCD12864_GPIO, LCD12864_RW, GPIO_PIN_SET)			
#define lcd_RW_0 HAL_GPIO_WritePin(LCD12864_GPIO, LCD12864_RW, GPIO_PIN_RESET)

#define lcd_RS_1  HAL_GPIO_WritePin(LCD12864_GPIO, LCD12864_RS, GPIO_PIN_SET)			  
#define lcd_RS_0  HAL_GPIO_WritePin(LCD12864_GPIO, LCD12864_RS, GPIO_PIN_RESET)




void LCD12864_int(void);
void LCD12864_Pos(unsigned char x, unsigned char y);
void show_word(unsigned char ch);
void GPIO_Configuration(void);
void spi_function(unsigned char del);
void send_cmd(unsigned char com);
void delay100us(void);
void LCD12864_write(unsigned char cmd, unsigned char dat);
void LCD12864_writebyte(unsigned char *prointer);			
 

#endif



