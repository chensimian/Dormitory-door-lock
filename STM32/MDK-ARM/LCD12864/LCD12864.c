#include "LCD12864.h"


void delay20ms()
{
	unsigned int i = 50;
	while(i)
	{
		i = i - 1;
	}
}

void LCD12864_int(void)
{
	  GPIO_Configuration();
		// the initial delay time i make it all the same
		//初始化命令之间的延时所有定义为最长延时，20ms 手册上说是10 ms
		send_cmd(0x34)	;
		delay20ms();
		send_cmd(0x30)	;
		delay20ms();
		send_cmd(0x0c)	;
		delay20ms();
		send_cmd(0x01)	;
		delay20ms();
		send_cmd(0x06)	;
		delay20ms();
}
//指定在液晶的哪一行显示  
void LCD12864_Pos(unsigned char x, unsigned char y)
{
	unsigned char pos = 0;
	send_cmd(0x30);						//这里是最重要的   不同的功能切换的时候事先一定要声明是在哪个模式下的命令
	if(x == 0)
		x = 0x80;
	else if(x == 1)
		x = 0x90;
	else if(x == 2)
		x = 0x88;
	else if(x == 3)
		x = 0x98;
	
	pos = x + y;
	send_cmd(pos);
}

//显示一个数据函数
void show_word(unsigned char ch)
{
	unsigned char data;		//发送数据，也是要分三个字节来发送，注意这次是fa开头，是发送，且发送的是数据的意思       
	lcd_RS_1;
	data = ch;
	spi_function(0x0fa);
	spi_function(data & 0xf0);
	spi_function( (data << 4) & 0xf0);
	lcd_RS_0;	
}

void GPIO_Configuration()
{
	
//	
//		GPIO_InitTypeDef GPIO_InitStructure;
//		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
//  	GPIO_InitStructure.GPIO_Pin = LCD12864_EN | LCD12864_RW	| LCD12864_RS	;
//  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
//  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  	GPIO_Init(LCD12864_GPIO, &GPIO_InitStructure);
//		GPIO_SetBits(LCD12864_GPIO, LCD12864_EN | LCD12864_RW	| LCD12864_RS);	 // turn off all led

  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_WritePin(GPIOB, LCD12864_EN | LCD12864_RW	| LCD12864_RS, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = LCD12864_EN | LCD12864_RW	| LCD12864_RS;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD12864_GPIO, &GPIO_InitStruct);
	
	
}


void delay100us(void)
{
	unsigned int i = 50;
	while(i)
	{
		i = i - 1;
	}
}
void spi_function(unsigned char del)
{
	unsigned char i = 8 , data;
	data = del;
	lcd_EN_1;								//每次发送的数据或者命令的话，都先拉高时钟线和数据线
	lcd_RW_1;
	delay100us();
	while(i)
	{
		if(data & 0x80)
			lcd_RW_1;
		else
			lcd_RW_0;
		delay100us();
		lcd_EN_0;							//时钟线拉高，延时一段时间再拉低，拉低意味着数据线上的数据要被12864锁存了
		delay100us();
		delay100us();
		lcd_EN_1;
		delay100us();	
		data = data << 1;
		i = i -1;	
	}
}

void send_cmd(unsigned char com)				//发送命令，分三个字节发送，注意第一个字节哦，f8，是指发送动作，且是发送命令
{
	unsigned char data;
	lcd_RS_1;
	data = com;
	spi_function(0x0f8);
	spi_function(data & 0xf0);
	spi_function( (data << 4) & 0xf0);
	lcd_RS_0;
}



void LCD12864_write(unsigned char cmd,unsigned char dat)
{
	unsigned char data;										//发送数据，也是要分三个字节来发送，注意这次是fa开头，是发送，且发送的是数据的意思
	lcd_RS_1;
	data = dat;
	if(cmd!=0) spi_function(0x0fa);
	else 	     spi_function(0x0f8);
	spi_function(data & 0xf0);
	spi_function( (data << 4) & 0xf0);
	lcd_RS_0;	
}
void LCD12864_writebyte(unsigned char *prointer)				
{
    while(*prointer!='\0')
    {
        LCD12864_write(1,*prointer);
        prointer++;
    }
}








