#define ESP8266_WIFI_INFO "AT+CWJAP=\"CMCC-GHPN\",\"y7n2qrya\"\r\n"
#define ESP8266_ONENET_INFO "AT+CIPSTART=\"TCP\",\"183.230.40.39\",6002\r\n"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "esp8266.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;

//#define CWSTARTSMART		//�����궨��
uint8_t Onebyte[1] = {0};
unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if (esp8266_cnt == 0) //������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;

	if (esp8266_cnt == esp8266_cntPre && esp8266_cnt >= 3) //�����һ�ε�ֵ�������ͬ����˵���������
	{
		return REV_OK; //���ؽ�����ɱ�־
	}

	esp8266_cntPre = esp8266_cnt; //��Ϊ��ͬ

	return REV_WAIT; //���ؽ���δ��ɱ�־
}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res, uint16_t time)
{
	Usart1_SendString(cmd, strlen(cmd));

	while (time--)
	{
		if (ESP8266_WaitRecive() == REV_OK) //����յ�����
		{
			if (strstr((const char *)esp8266_buf, res) != NULL) //����������ؼ���
			{
				ESP8266_Clear(); //��ջ���

				return 0;
			}
		}

		HAL_Delay(10);
	}

	return 1;
}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];

	ESP8266_Clear();						   //��ս��ջ���
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len); //��������
	if (!ESP8266_SendCmd(cmdBuf, ">", 200))	   //�յ���>��ʱ���Է�������
	{
		Usart1_SendString((char *)data, len); //�����豸������������
	}
}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;

	do
	{
		if (ESP8266_WaitRecive() == REV_OK) //����������
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,"); //������IPD��ͷ
			if (ptrIPD == NULL)							  //���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{
				////printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':'); //�ҵ�':'
				if (ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}

		HAL_Delay(5); //��ʱ�ȴ�
	} while (timeOut--);

	return NULL; //��ʱ��δ�ҵ������ؿ�ָ��
}
//WIFIд���ڳ�����
//ONENET�ٷ������õķ�ʽ
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
//  * ���� :1���Զ�����WIFI  2��΢��AIrKiss����
//  * ʹ��˵���� �������������AirKiss����������ֱ���ƶ�����
// */
// void WIFI_CWSTARTSMART(void)
// {
// 	if (KEY == 0)
// 	{
// 		HAL_Delay(10);
// 		if (KEY == 0)
// 		{
// 			Gui_DrawFont_GBK16(4, 90, BLUE, BLACK, (u8 *)"AirKiss");
// 			//����SmartConfigģʽ
// 			while (ESP8266_SendCmd("AT+CWSTARTSMART\r\n", "WIFI GOT IP", 400))
// 			{
// 			}
// 			//�ر�SmartConfig
// 			while (ESP8266_SendCmd("AT+CWSTOPSMART\r\n", "OK", 200))
// 			{
// 			}
// 		}
// 	}

// 	//����ESP8266
// 	while (ESP8266_SendCmd("AT+RST\r\n", "WIFI GOT IP", 400))
// 	{
// 	}
// }

//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_Init(void)
{
	//ESP8266��λ���� PA4��λ
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
*	�������ƣ�	Usart1_Init
*
*	�������ܣ�	����2��ʼ��
*
*	��ڲ�����	baud���趨�Ĳ�����
*
*	���ز�����	��
*
*	˵����		TX-PA2		RX-PA3
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
//	�������ƣ�	USART1_IRQHandler
//
//	�������ܣ�	����2�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
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

// 	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //�����ж�
// 	{
// 		if (esp8266_cnt >= sizeof(esp8266_buf))
// 			esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
// 		esp8266_buf[esp8266_cnt++] = USART2->DR;

// 		USART_ClearFlag(USART2, USART_FLAG_RXNE);
// 	}
// }

/*
************************************************************
*	�������ƣ�	Usart_SendString
*
*	�������ܣ�	�������ݷ���
*
*	��ڲ�����	USARTx��������
*				str��Ҫ���͵�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void Usart1_SendString(char *str, unsigned short len)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)str, len, 0xFFFF);
}
