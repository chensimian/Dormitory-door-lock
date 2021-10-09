#define ESP8266_WIFI_INFO "AT+CWJAP=\"CMCC-GHPN\",\"y7n2qrya\"\r\n"
#define ESP8266_ONENET_INFO "AT+CIPSTART=\"TCP\",\"183.230.40.39\",6002\r\n"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "esp8266.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;

//#define CWSTARTSMART		//配网宏定义
uint8_t Onebyte[1] = {0};
unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if (esp8266_cnt == 0) //如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;

	if (esp8266_cnt == esp8266_cntPre && esp8266_cnt >= 3) //如果上一次的值和这次相同，则说明接收完毕
	{
		return REV_OK; //返回接收完成标志
	}

	esp8266_cntPre = esp8266_cnt; //置为相同

	return REV_WAIT; //返回接收未完成标志
}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res, uint16_t time)
{
	Usart1_SendString(cmd, strlen(cmd));

	while (time--)
	{
		if (ESP8266_WaitRecive() == REV_OK) //如果收到数据
		{
			if (strstr((const char *)esp8266_buf, res) != NULL) //如果检索到关键词
			{
				ESP8266_Clear(); //清空缓存

				return 0;
			}
		}

		HAL_Delay(10);
	}

	return 1;
}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];

	ESP8266_Clear();						   //清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len); //发送命令
	if (!ESP8266_SendCmd(cmdBuf, ">", 200))	   //收到‘>’时可以发送数据
	{
		Usart1_SendString((char *)data, len); //发送设备连接请求数据
	}
}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;

	do
	{
		if (ESP8266_WaitRecive() == REV_OK) //如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,"); //搜索“IPD”头
			if (ptrIPD == NULL)							  //如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				////printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':'); //找到':'
				if (ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}

		HAL_Delay(5); //延时等待
	} while (timeOut--);

	return NULL; //超时还未找到，返回空指针
}
//WIFI写死在程序中
//ONENET官方例程用的方式
void WIFI_CWJAP(void)
{
	//printf("AT\r\n");
	while (ESP8266_SendCmd("AT\r\n", "OK", 200))
		HAL_Delay(500);
	//printf("CWMODE\r\n");
	while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 200))
		HAL_Delay(500);
	//printf("AT+CWDHCP\r\n");
	while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200))
		HAL_Delay(500);
	//printf("CWJAP\r\n");
	while (ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP", 200))
		HAL_Delay(500);
}
// /*
//  * 功能 :1、自动连接WIFI  2、微信AIrKiss配网
//  * 使用说明： 按键按下则进行AirKiss配网，否则直接制动连接
// */
// void WIFI_CWSTARTSMART(void)
// {
// 	if (KEY == 0)
// 	{
// 		HAL_Delay(10);
// 		if (KEY == 0)
// 		{
// 			Gui_DrawFont_GBK16(4, 90, BLUE, BLACK, (u8 *)"AirKiss");
// 			//进入SmartConfig模式
// 			while (ESP8266_SendCmd("AT+CWSTARTSMART\r\n", "WIFI GOT IP", 400))
// 			{
// 			}
// 			//关闭SmartConfig
// 			while (ESP8266_SendCmd("AT+CWSTOPSMART\r\n", "OK", 200))
// 			{
// 			}
// 		}
// 	}

// 	//重启ESP8266
// 	while (ESP8266_SendCmd("AT+RST\r\n", "WIFI GOT IP", 400))
// 	{
// 	}
// }

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Init(void)
{
	//ESP8266复位引脚 PA4复位
	Usart1_Init();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_Delay(250);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_Delay(500);
	ESP8266_Clear();
	WIFI_CWJAP();
	//printf("CIPSTART\r\n");
	while (ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT", 200))
		HAL_Delay(500);
	//printf("ESP8266 Init OK\r\n");
}

/*
************************************************************
*	函数名称：	Usart1_Init
*
*	函数功能：	串口2初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PA2		RX-PA3
************************************************************
*/
void Usart1_Init(void)
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);
	HAL_UART_Receive_IT(&huart1, Onebyte, 1);
}

//==========================================================
//	函数名称：	USART1_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {

// }
void USART1_IRQHandler(void)
{
	uint32_t timeout = 0;
	HAL_UART_IRQHandler(&huart1);
	timeout = 0;
	while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY)
	{
		timeout++;
		if (timeout > HAL_MAX_DELAY)
			break;
	}
	timeout = 0;
	while (HAL_UART_Receive_IT(&huart1, (uint8_t *)Onebyte, 1) != HAL_OK)
	{
		timeout++;
		if (timeout > HAL_MAX_DELAY)
			break;
	}
}
// void USART1_IRQHandler(void)
// {

// 	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
// 	{
// 		if (esp8266_cnt >= sizeof(esp8266_buf))
// 			esp8266_cnt = 0; //防止串口被刷爆
// 		esp8266_buf[esp8266_cnt++] = USART2->DR;

// 		USART_ClearFlag(USART2, USART_FLAG_RXNE);
// 	}
// }

/*
************************************************************
*	函数名称：	Usart_SendString
*
*	函数功能：	串口数据发送
*
*	入口参数：	USARTx：串口组
*				str：要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Usart1_SendString(char *str, unsigned short len)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)str, len, 0xFFFF);
}
