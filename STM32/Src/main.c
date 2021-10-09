/**
PA4    ����ESP8266 RST����
ESP8266ʹ��Uart1
WiFi������esp8266.c
��Ȩ������onenet.c

  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "esp8266.h"
#include "onenet.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "LCD12864.h"
#include "string.h"

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

//���涨���ĸ������ߵ͵�ƽ�仯
#define key1_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
#define key1_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)

#define key2_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET)
#define key2_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET)

#define key3_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define key3_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)

#define key4_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define key4_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)

#define key5_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define key5_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

#define key6_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define key6_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)

#define key7_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)
#define key7_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)

#define key8_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)
#define key8_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)

#define key1 HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)
#define key2 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)
#define key3 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)
#define key4 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)
#define key5 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)
#define key6 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)
#define key7 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)
#define key8 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) //������ز�������

unsigned char Key_Data = 0;	 //�������µ�����
unsigned char Key_flag = 0;	 //�������±�־λ�����ְ�������һ�ε�
unsigned char key_back = 16; //������̣�0-15 ���ݣ������ڰ���֮��ḳֵ ����

#define JDQ_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET)
#define JDQ_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET) //�̵���IO�仯

#define beep_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET)
#define beep_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET) //������IO�仯����

unsigned char beep_flag = 0;
const char *sub_topic[2] = {"kai", "/kai"};
//���涨��ָ���봮��
uint8_t aRxBuffer[3]; //����һ���ݴ����飬��������ǽ��մ������ݵ�Ȼ����¸�����
uint8_t RxBuffer[3];
unsigned char Uart_dat[1024];  //�洢���ڹ���������
unsigned char Uart1_dat[1000]; //�洢���ڹ���������
unsigned int uart_count = 0;   //�������ݽ��ռ���
unsigned int uart1_count = 0;  //�������ݽ��ռ���

#define fingerprint HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) //��ָ��ģ������ָ�Ƶ�ʱ�����IO���ɸߵ�ƽ

unsigned char Search_Sign = 0;	//���������������
unsigned char Search_time = 0;	//������ʱ�����
unsigned char Command_flag = 1; //����ָ�����ʱ����

unsigned char Password[6] = {0, 0, 0, 0, 0, 0};
unsigned char Password1[6] = {0, 0, 0, 0, 0, 0};
unsigned char Password_flsh[6] = {0, 0, 0, 0, 0, 0};
unsigned char Password_count = 0; //�����������������

unsigned int Fingerprint_Num = 0;

unsigned char system_time = 0;
unsigned char beep_time = 0;
unsigned char JDQ_time = 0; //�̵����պ�ʱ�䣬��������ʱ�仹�е�����ҳ�棬���صĵ���ʱʱ�����

unsigned long Heartbeat_Timer = 0;
//��������������Σ�����ָ�ƴ������� ����60S
unsigned char mi_ma_Error = 0;
unsigned char zhiwen_Error = 0;

//���涨�嶨ʱ���õ���һЩ����
unsigned char s0 = 0;	   //��˸�õ���
unsigned int Time2_ms = 0; //��ʱ��2����ʱʱ��

unsigned char state = 0; //��ʾ�������
extern unsigned char esp8266_buf[128];
extern uint8_t Onebyte[1];
extern unsigned short esp8266_cnt;
extern unsigned short esp8266_cntPre;
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);

void OneNET_transfer(uint16_t id);
void OneNET_transferkey(uint8_t k1, uint8_t k2, uint8_t k3, uint8_t k4, uint8_t k5, uint8_t k6);
void Single_Deletion(unsigned char Num);
void Delete_all(void);

void Small_Delay(unsigned long z)
{
	while (z--)
		;
}
void Write_flsh_byte(uint32_t EEPROM_ADD, uint32_t EEPROM_Data)
{

	HAL_FLASH_Unlock();		  //1������FLASH
	FLASH_EraseInitTypeDef f; //2������FLASH��ʼ��FLASH_EraseInitTypeDef
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = EEPROM_ADD;
	f.NbPages = 1;
	uint32_t PageError = 0;												//����PageError
	HAL_FLASHEx_Erase(&f, &PageError);									//���ò�������
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, EEPROM_ADD, EEPROM_Data); //3����FLASH��д
	HAL_FLASH_Lock();													//4����סFLASH
}

uint32_t readFlash(uint32_t EEPROM_ADD) //FLASH��ȡ���ݲ���
{
	uint32_t temp = *(__IO uint32_t *)(EEPROM_ADD);
	return temp;
}

void memory(unsigned char dat1, unsigned char dat2, unsigned char dat3, unsigned char dat4, unsigned char dat5, unsigned char dat6)
{

	Write_flsh_byte(0x08004000 + 0x2000, dat1);
	Write_flsh_byte(0x08006100 + 0x2000, dat2);
	Write_flsh_byte(0x08008200 + 0x2000, dat3);
	Write_flsh_byte(0x0800A300 + 0x2000, dat4);
	Write_flsh_byte(0x0800C400 + 0x2000, dat5);
	Write_flsh_byte(0x0800E500 + 0x2000, dat6);
}

void read_memory()
{
	Password1[0] = readFlash(0x08004000 + 0x2000);
	Password1[1] = readFlash(0x08006100 + 0x2000);
	Password1[2] = readFlash(0x08008200 + 0x2000);
	Password1[3] = readFlash(0x0800A300 + 0x2000);
	Password1[4] = readFlash(0x0800C400 + 0x2000);
	Password1[5] = readFlash(0x0800E500 + 0x2000);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) //1ms��ʱ
{
	Time2_ms++;
	Heartbeat_Timer++;
	if (Time2_ms % 50 == 0)
	{
		if (Search_time != 0)
			Search_time--;
		else
			Search_Sign = 0;
		if (JDQ_time != 0) //�̵���ʱ�䲻Ϊ0���ͱպϣ�ģ�⿪��
		{
			if (JDQ_time > 30)
			{
				system_time = 2;
				state = 4;
			}
			JDQ_time--;
			JDQ_1;
		}
		else
			JDQ_0; //�������

		if (beep_time != 0)
		{
			beep_time--;
			GPIOA->ODR ^= GPIO_PIN_0; //ȡ����
		}
		else
		{
			beep_flag = 1;
			beep_0;
		}
	}

	if (Time2_ms % 666 == 0)
	{
		s0 = s0 ^ 0x01; //ȡ��
		if (state == 0)
			Command_flag = 1; //ָ��ģ�鷢�ͱ�־λ���ڳ�ʼ����
	}

	if (Time2_ms % 1000 == 0)
	{
		if (system_time != 0)
			system_time--;
		else
			state = 0;
	}

	if (Time2_ms >= 2000)
	{
		Time2_ms = 0;
		if (state == 7)
			Command_flag = 1; //ָ��ģ�鷢�ͱ�־λ��¼��ָ�ƽ���
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) //���ڵĻص�����
{
	uint8_t ret = HAL_OK;
	if (huart->Instance == USART1) //�жϴ��ں�
	{
		////printf("Rec:%s", esp8266_buf);
		esp8266_buf[esp8266_cnt++] = Onebyte[0];
		if (esp8266_cnt > sizeof(esp8266_buf))
			esp8266_cnt = 0;
	}
	else if (huart == &huart2)
	{
		Uart_dat[uart_count] = aRxBuffer[0];
		uart_count = (uart_count + 1) % 1024; //  EF 01 FF FF FF FF 07 00 07 00 00 05 00 60 00 73
		Uart_dat[uart_count] = 0;			  //  EF 01 FF FF FF FF 07 00 07 09 00 00 00 00 00 17 	 //��������λ��ָ�Ʊ��
		if (state == 0 && uart_count >= 16 && Uart_dat[uart_count - 16] == 0xEF && Uart_dat[uart_count - 15] == 0x01 && Uart_dat[uart_count - 11] == 0xFF && Uart_dat[uart_count - 8] == 0x07 && Uart_dat[uart_count - 10] == 0x07)
		{
			if (Uart_dat[uart_count - 7] == 0x00) //ȷ��������
			{
				Fingerprint_Num = Uart_dat[uart_count - 5];
				JDQ_time = 40;
				Search_Sign = 0; //�������
				beep_time = 1;

				zhiwen_Error = 0;
				mi_ma_Error = 0;
			}
			else //û���������������Ǵ���
			{
				Search_Sign = 0;
				zhiwen_Error++;
				if (zhiwen_Error >= 3)
				{
					system_time = 60;
					beep_time = 6;
					state = 17;
				}
				else
				{
					state = 3;
					system_time = 2;
					beep_time = 6;
				}
			}
			uart_count = 0; //�������
		}
		else if (state == 7 && uart_count >= 14 && Uart_dat[uart_count - 14] == 0xEF && Uart_dat[uart_count - 13] == 0x01 && Uart_dat[uart_count - 9] == 0xFF && Uart_dat[uart_count - 6] == 0x05)
		{
			//  ����ɹ� ���أ� EF 01 FF FF FF FF 07 00 05 00 00 06 00 12		//���е�������λ�Ǳ��
			//  ���ʧ�� ���أ� EF 01 FF FF FF FF 07 00 05 02 48 00 00 56
			if (Uart_dat[uart_count - 5] == 0x00) //ע��ɹ�
			{
				system_time = 2;
				state = 5;
				Fingerprint_Num = Uart_dat[uart_count - 3];
				uart_count = 0; //�������
				beep_time = 6;
			}
			else //ע��ʧ��,һ�㲻��ʧ��
			{
			}

			uart_count = 0; //�������
		}

		else if (state == 0 && uart_count >= 12 && Uart_dat[uart_count - 12] == 0xEF && Uart_dat[uart_count - 11] == 0x01 && Uart_dat[uart_count - 7] == 0xFF && Uart_dat[uart_count - 4] == 0x03)
		{
			//  ����ɹ� ���أ�  EF 01 FF FF FF FF 07 00 03 00 00 0A
			//  ���ʧ�� ���أ�  EF 01 FF FF FF FF 07 00 03 02 00 0C
			if (state == 0)
			{
				if (Uart_dat[uart_count - 3] == 0x00) //�ɹ�
				{
					if (Search_Sign == 0) //�����ǰ�ǻ�ȡָ��ͼ��״̬����⵽�ɹ�������������ָ��ģ��״̬
					{
						Search_time = 60; // ��������ǵ��ɹ���ȡָ��ͼ��֮�󣬿�ʼ��ʱ������󲻹���û�гɹ���������ͷ��ʼִ����������
						Search_Sign = 1;
					}
					else if (Search_Sign == 1) //�����ǰ������ָ��ģ��״̬����⵽�ɹ�������������ָ������״̬
					{
						Search_Sign = 2;
					}
				}
				else
				{
					Search_Sign = 0;
				}
			}
			else
			{
				Search_Sign = 0;
			}

			uart_count = 0;
		}
		//    HAL_UART_Transmit(&huart1,(unsigned char *)&aRxBuffer[0],1,0);
		do
		{
			ret = HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer[0], 1);
		} while (ret != HAL_OK);
	}
}

void Key_assignment(unsigned char z)
{

	Key_Data = 0x00; //����
	if (z == 0)
	{
		key1_0;
		key2_0;
		key3_0;
		key4_0;
		key5_1;
		key6_1;
		key7_1;
		key8_1; //ʹ�õķ����Ƿ�ת��
	}
	else
	{
		key1_1;
		key2_1;
		key3_1;
		key4_1;
		key5_0;
		key6_0;
		key7_0;
		key8_0; //ʹ�õķ����Ƿ�ת��
	}

	Small_Delay(222); //����ʱһ��

	if (key1 == 1)
		Key_Data = Key_Data | 0x80;
	else
		Key_Data = Key_Data | 0x00;

	if (key2 == 1)
		Key_Data = Key_Data | 0x40;
	else
		Key_Data = Key_Data | 0x00;

	if (key3 == 1)
		Key_Data = Key_Data | 0x20;
	else
		Key_Data = Key_Data | 0x00;

	if (key4 == 1)
		Key_Data = Key_Data | 0x10;
	else
		Key_Data = Key_Data | 0x00;

	if (key5 == 1)
		Key_Data = Key_Data | 0x08;
	else
		Key_Data = Key_Data | 0x00;

	if (key6 == 1)
		Key_Data = Key_Data | 0x04;
	else
		Key_Data = Key_Data | 0x00;

	if (key7 == 1)
		Key_Data = Key_Data | 0x02;
	else
		Key_Data = Key_Data | 0x00;

	if (key8 == 1)
		Key_Data = Key_Data | 0x01;
	else
		Key_Data = Key_Data | 0x00; //����˸�IOƴ��һ������Ҳ���ǵ���51�����һ��IO
}

void Key_Dispose() //����������
{
	unsigned char Key_x;
	Key_assignment(0);	  //״̬ 0 ����ʱ����û�а��µĻ��� 0x0f
	Small_Delay(50);	  //����ʱ
	if (Key_Data != 0x0f) //ֻҪ�ǰ������£��ͻ�ʹ�� Key_Data!=0x0f
	{
		if (Key_flag == 1) //����Ǳ�֤��������ִֻ��һ�ε�
		{
			Key_flag = 0;				 //����
			Key_x = Key_Data;			 //����ʱ�ļ�ֵ��ֵ
			Key_assignment(1);			 //�л�״̬ 1
			Small_Delay(10);			 //����ʱһ��
			Key_Data = Key_Data & Key_x; //�������ݰ�λ�룬�ͻ������һ�޶�������
										 //		     HAL_UART_Transmit(&huart1,&Key_Data,1,100);   //�����ǲ鿴����ֵ�ģ����Գ���

			switch (Key_Data)
			{
			case 0x11:
				key_back = 1;
				break;
			case 0x12:
				key_back = 2;
				break;
			case 0x14:
				key_back = 3;
				break;
			case 0x18:
				key_back = 12;
				break;

			case 0x21:
				key_back = 4;
				break;
			case 0x22:
				key_back = 5;
				break;
			case 0x24:
				key_back = 6;
				break;
			case 0x28:
				key_back = 13;
				break;

			case 0x41:
				key_back = 7;
				break;
			case 0x42:
				key_back = 8;
				break;
			case 0x44:
				key_back = 9;
				break;
			case 0x48:
				key_back = 14;
				break;

			case 0x81:
				key_back = 10;
				break;
			case 0x82:
				key_back = 0;
				break;
			case 0x84:
				key_back = 11;
				break;
			case 0x88:
				key_back = 15;
				break;
			}
			system_time = 10;
			beep_time = 1;
		}
	}
	else
		Key_flag = 1;
}
void key_data_dispose()
{
	if (key_back != 16)
	{
		if (state == 0) //���������ʱ��ֻ��  �л�������������水�������������水������
		{
			if (key_back == 12)
			{
				key_back = 16;
				state = 1;
			}
			if (key_back <= 9)
			{
				if (Password_count < 6)
				{
					Password[Password_count] = key_back;
					Password_count++;
				}
				key_back = 16;
				state = 14;
			}
		}
		else if (state == 1)
		{
			if (key_back <= 9) //�������룬0-9
			{
				if (Password_count < 6)
				{
					Password[Password_count] = key_back;
					Password_count++;
				}
				else
				{
					beep_time = 6;
				}
				key_back = 16;
			}
			else if (key_back == 10) //������������
			{
				if (Password_count > 0)
					Password_count--;
				key_back = 16;
			}
			else if (key_back == 11) //ȷ�ϰ���
			{
				if ((Password[0] == 0 && Password[1] == 0 && Password[2] == 0 && Password[3] == 0 && Password[4] == 0 && Password[5] == 0) || (Password[0] == Password1[0] && Password[1] == Password1[1] && Password[2] == Password1[2] && Password[3] == Password1[3] && Password[4] == Password1[4] && Password[5] == Password1[5]))
				{
					state = 2;
					zhiwen_Error = 0;
					mi_ma_Error = 0;
				}
				else
				{

					beep_time = 6;

					mi_ma_Error++;
					if (mi_ma_Error >= 3)
					{
						state = 17;
						system_time = 60;
					}
					else
					{
						state = 6;
						system_time = 2;
					}
				}
				key_back = 16;
			}
			else if (key_back == 15) //������������
			{
				state = 0;
				key_back = 16;
			}
			else if (key_back == 14) //������������
			{
				state = 0;
				key_back = 16;
			}
		}
		else if (state == 2)
		{
			if (key_back == 1) //�������밴������
			{
				key_back = 16;
				state = 7;
			}
			else if (key_back == 2) //�ֶ�����
			{
				key_back = 16;
				JDQ_time = 30;
			}
			else if (key_back == 3) //ɾ��ָ��ָ��
			{
				key_back = 16;
				state = 10;
				Password_count = 0;
				Fingerprint_Num = 0;
			}
			else if (key_back == 4) //��������
			{
				key_back = 16;
				system_time = 10;
				Password_count = 0;
				Fingerprint_Num = 0;
				state = 8;
			}
			else if (key_back == 15) //������������
			{
				state = 0;
				key_back = 16;
			}
			else if (key_back == 14) //������������
			{
				state = 0;
				key_back = 16;
			}
		}
		else if (state == 6 || state == 3 || state == 11 || state == 4 || state == 5)
		{
			if (key_back != 16)
			{
				key_back = 16;
				state = 0;
				system_time = 0;
			}
		}
		else if (state == 7)
		{
			if (key_back == 15) //������������
			{
				state = 0;
				key_back = 16;
			}
			else if (key_back == 14) //������������
			{
				state = 2;
				key_back = 16;
			}
		}
		else if (state == 10)
		{
			if (key_back <= 9) //�������룬0-9
			{
				if (Password_count < 3)
				{
					if (Password_count == 0)
					{
						Fingerprint_Num = Fingerprint_Num + key_back;
					}
					else if (Password_count == 1)
					{
						Fingerprint_Num = Fingerprint_Num * 10 + key_back;
					}
					else if (Password_count == 2)
					{
						Fingerprint_Num = Fingerprint_Num * 10 + key_back;
					}
					Password_count++;
				}
				else
				{
					beep_time = 6;
				}
				key_back = 16;
			}
			else if (key_back == 10) //������������
			{

				if (Password_count > 0)
					Password_count--;
				if (Password_count == 0)
				{
					Fingerprint_Num = 0;
				}
				else if (Password_count == 1)
				{
					Fingerprint_Num = Fingerprint_Num / 10 % 10;
				}
				else if (Password_count == 2)
				{
					Fingerprint_Num = Fingerprint_Num / 100 % 10 * 10 + Fingerprint_Num / 10 % 10;
				}

				key_back = 16;
			}
			else if (key_back == 11) //��һɾ��
			{
				key_back = 16;

				if (Fingerprint_Num == 999)
				{
					Delete_all();
				}
				else
				{
					Single_Deletion(Fingerprint_Num);
				}
				system_time = 2;
				state = 11;
			}
			else if (key_back == 15) //������������
			{
				state = 0;
				key_back = 16;
				system_time = 0;
			}
			else if (key_back == 14) //������������
			{
				state = 2;
				key_back = 16;
				system_time = 0;
			}
		}
		else if (state == 8)
		{
			if (key_back <= 9) //�������룬0-9
			{
				if (Password_count < 12)
				{
					if (Password_count < 6)
					{
						Password[Password_count] = key_back;
					}
					else if (Password_count < 12)
					{
						Password_flsh[Password_count - 6] = key_back;
					}
					Password_count++;
				}
				else
				{
					beep_time = 3;
				}
				key_back = 16;
			}
			else if (key_back == 10) //������������
			{
				if (Password_count > 0)
					Password_count--;
				key_back = 16;
			}
			else if (key_back == 11) //���������ȷ�ϰ�������
			{
				key_back = 16;
				if (Password[0] == Password_flsh[0] && Password[1] == Password_flsh[1] && Password[2] == Password_flsh[2] && Password[3] == Password_flsh[3] && Password[4] == Password_flsh[4] && Password[5] == Password_flsh[5])
				{ //������������һֱ
					system_time = 2;
					state = 12;
					memory(Password[0], Password[1], Password[2], Password[3], Password[4], Password[5]);
					Small_Delay(666);
					read_memory();
				}
				else //�����������벻һ��
				{
					system_time = 2;
					state = 13;
					Password_count = 0;
				}
			}
		}
		else if (state == 13 || state == 12)
		{
			if (key_back != 16)
			{
				state = 2;
			}
		}
		else if (state == 14)
		{
			if (key_back <= 9) //�������룬0-9
			{
				state = 14;
				if (Password_count < 6)
				{
					Password[Password_count] = key_back;
					Password_count++;
				}
				else
				{
					beep_time = 6;
				}
			}
			else if (key_back == 10) //������������
			{
				if (Password_count > 0)
					Password_count--;
				key_back = 16;
			}
			else if (key_back == 11) //ȷ�ϰ���
			{
				if (Password[0] == Password1[0] && Password[1] == Password1[1] && Password[2] == Password1[2] && Password[3] == Password1[3] && Password[4] == Password1[4] && Password[5] == Password1[5])
				{
					system_time = 2;
					JDQ_time = 30;
					state = 16;
					mi_ma_Error = 0;
					zhiwen_Error = 0;
					OneNet_SendData();
				}
				else
				{
					beep_time = 6;
					mi_ma_Error++;
					if (mi_ma_Error >= 3)
					{
						state = 17;
						system_time = 60;
					}
					else
					{
						state = 6;
						system_time = 2;
					}
				}
				key_back = 16;
			}
			else if (key_back == 15) //������������
			{
				state = 0;
				key_back = 16;
			}
			else if (key_back == 14) //������������
			{
				state = 0;
				key_back = 16;
			}
		}
		else if (state == 16)
		{
			if (key_back != 0)
				state = 0;
		}
		key_back = 16;
	}
}
void display()
{
	if (state == 0)
	{
		Password_count = 0;
		Password[0] = 0;
		Password[1] = 0;
		Password[2] = 0;
		Password[3] = 0;
		Password[4] = 0;
		Password[5] = 0;

		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    ��ӭʹ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"  ָ�����������  ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"                ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"    ��������    ");
	}
	else if (state == 1)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    ���ý���    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"    ��������     ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"     ");

		if (Password_count == 0)
		{
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"          ");
		}
		else if (Password_count == 1)
		{
			LCD12864_writebyte((unsigned char *)"*");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"         ");
		}
		else if (Password_count == 2)
		{
			LCD12864_writebyte((unsigned char *)"**");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"        ");
		}
		else if (Password_count == 3)
		{
			LCD12864_writebyte((unsigned char *)"***");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"       ");
		}
		else if (Password_count == 4)
		{
			LCD12864_writebyte((unsigned char *)"****");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"      ");
		}
		else if (Password_count == 5)
		{
			LCD12864_writebyte((unsigned char *)"*****");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"     ");
		}
		else if (Password_count == 6)
		{
			LCD12864_writebyte((unsigned char *)"******");
			LCD12864_writebyte((unsigned char *)"     ");
		}
	}
	else if (state == 2)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"1:����ָ��    ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"2:�ֶ�����      ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"3:ɾ��ָ��      ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"4:��������      ");
	}
	else if (state == 3)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    �׶Բ���    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"    ����ʧ��    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");

		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"  δ������ָ��  ");
		Search_Sign = 0;
	}
	else if (state == 4)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    �׹�ϲ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"    ���ųɹ�    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");

		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"  ָ�Ʊ��:");
		LCD12864_write(1, 0x30 + Fingerprint_Num / 100 % 10);
		LCD12864_write(1, 0x30 + Fingerprint_Num / 10 % 10);
		LCD12864_write(1, 0x30 + Fingerprint_Num % 10);
		LCD12864_writebyte((unsigned char *)"  ");
		Search_Sign = 0;
		OneNet_SendData();
		ESP8266_Clear();
	}
	else if (state == 5)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    �׹�ϲ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"    ��ӳɹ�    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");

		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"  ָ�Ʊ��:");
		LCD12864_write(1, 0x30 + Fingerprint_Num / 100 % 10);
		LCD12864_write(1, 0x30 + Fingerprint_Num / 10 % 10);
		LCD12864_write(1, 0x30 + Fingerprint_Num % 10);
		LCD12864_writebyte((unsigned char *)"  ");
	}
	else if (state == 6)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    �׶Բ���    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"    �������    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"���ⰴ�����¼���");
	}
	else if (state == 7)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    ����ָ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"�뽫��ָ����ģ��");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"�˳��밴���ذ���");
	}
	else if (state == 8)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    ��������  ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"����������λ����");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"��������:");
		if (Password_count == 0)
		{
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"      ");
		}
		else if (Password_count == 1)
		{
			LCD12864_writebyte((unsigned char *)"*");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"     ");
		}
		else if (Password_count == 2)
		{
			LCD12864_writebyte((unsigned char *)"**");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"    ");
		}
		else if (Password_count == 3)
		{
			LCD12864_writebyte((unsigned char *)"***");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"   ");
		}
		else if (Password_count == 4)
		{
			LCD12864_writebyte((unsigned char *)"****");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"  ");
		}
		else if (Password_count == 5)
		{
			LCD12864_writebyte((unsigned char *)"*****");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)" ");
		}
		else if (Password_count == 6)
		{
			LCD12864_writebyte((unsigned char *)"******");
			LCD12864_writebyte((unsigned char *)" ");
		}

		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"�ٴ�����:");
		if (Password_count == 6)
		{
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"      ");
		}
		else if (Password_count == 7)
		{
			LCD12864_writebyte((unsigned char *)"*");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"     ");
		}
		else if (Password_count == 8)
		{
			LCD12864_writebyte((unsigned char *)"**");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"    ");
		}
		else if (Password_count == 9)
		{
			LCD12864_writebyte((unsigned char *)"***");
			if (s0)
				LCD12864_writebyte("_");
			else
				LCD12864_writebyte(" ");
			LCD12864_writebyte("   ");
		}
		else if (Password_count == 10)
		{
			LCD12864_writebyte("****");
			if (s0)
				LCD12864_writebyte("_");
			else
				LCD12864_writebyte(" ");
			LCD12864_writebyte("  ");
		}
		else if (Password_count == 11)
		{
			LCD12864_writebyte("*****");
			if (s0)
				LCD12864_writebyte("_");
			else
				LCD12864_writebyte(" ");
			LCD12864_writebyte(" ");
		}
		else if (Password_count == 12)
		{
			LCD12864_writebyte("******");
			LCD12864_writebyte(" ");
		}
		else
			LCD12864_writebyte("       ");
	}
	else if (state == 9)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("    ����ָ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("    ¼��ʧ��   ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte("���ⰴ�����¼���");
	}
	else if (state == 10)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("    ��ʾ����    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("  ɾ��ָ��ָ��  ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte("����ָ�ƺ�:");
		LCD12864_write(1, 0x30 + Fingerprint_Num / 100 % 10);
		LCD12864_write(1, 0x30 + Fingerprint_Num / 10 % 10);
		LCD12864_write(1, 0x30 + Fingerprint_Num % 10);
	}
	else if (state == 11)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("    �׹�ϲ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("    ɾ���ɹ�    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte("                ");
	}
	else if (state == 12)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("    �׹�ϲ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("    ���ĳɹ�    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte("                ");
	}
	else if (state == 13)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("    �׶Բ���    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("    ����ʧ��    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte("                ");
	}
	else if (state == 14)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte((unsigned char *)"    ��ӭʹ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte((unsigned char *)"    ��������     ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte((unsigned char *)"       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte((unsigned char *)"     ");

		if (Password_count == 0)
		{
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"          ");
		}
		else if (Password_count == 1)
		{
			LCD12864_writebyte((unsigned char *)"*");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"         ");
		}
		else if (Password_count == 2)
		{
			LCD12864_writebyte((unsigned char *)"**");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"        ");
		}
		else if (Password_count == 3)
		{
			LCD12864_writebyte((unsigned char *)"***");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"       ");
		}
		else if (Password_count == 4)
		{
			LCD12864_writebyte((unsigned char *)"****");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"      ");
		}
		else if (Password_count == 5)
		{
			LCD12864_writebyte((unsigned char *)"*****");
			if (s0)
				LCD12864_writebyte((unsigned char *)"_");
			else
				LCD12864_writebyte((unsigned char *)" ");
			LCD12864_writebyte((unsigned char *)"     ");
		}
		else if (Password_count == 6)
		{
			LCD12864_writebyte((unsigned char *)"******");
			LCD12864_writebyte((unsigned char *)"     ");
		}
	}
	else if (state == 16)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("    �׹�ϲ��    ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("    ���ųɹ�    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");

		LCD12864_Pos(3, 0);
		LCD12864_writebyte("                ");
	}
	else if (state == 17)
	{
		LCD12864_Pos(0, 0);
		LCD12864_writebyte("      ����      ");
		LCD12864_Pos(1, 0);
		LCD12864_writebyte("    ϵͳ����    ");
		LCD12864_Pos(2, 0);
		LCD12864_writebyte("       ");
		LCD12864_write(1, 0x30 + system_time / 10 % 10);
		LCD12864_write(1, 0x30 + system_time % 10);
		LCD12864_writebyte("       ");
		LCD12864_Pos(3, 0);
		LCD12864_writebyte("                ");
		zhiwen_Error = 0;
		mi_ma_Error = 0;
	}
}

//�����������Ϊָ�ƴ������ݷ���
void UartData(unsigned char q)
{
	unsigned char Tab[1];
	Tab[0] = q;
	HAL_UART_Transmit_IT(&huart2, Tab, 1);
	HAL_Delay(1);
}

void Register()
{

	if (Command_flag)
	{
		Command_flag = 0;
		Command_flag = 0;
		UartData(0xEF);
		UartData(0x01);
		UartData(0xFF);
		UartData(0xFF);
		UartData(0xFF);
		UartData(0xFF);
		UartData(0x01);
		UartData(0x00);
		UartData(0x03);
		UartData(0x10);
		UartData(0x00);
		UartData(0x14);
	}
}

void Search()
{
	//Search_Sign����0��ȡͼ��1ģ�壬2����
	//��������Ҫһ�ײ���ģ���ȡͼ��Ȼ������ģ�壬Ȼ������
	//  ȡͼ���·�����   EF 01 FF FF FF FF 01 00 03 01 00 05
	//  ����ɹ� ���أ�  EF 01 FF FF FF FF 07 00 03 00 00 0A
	//  ���ʧ�� ���أ�  EF 01 FF FF FF FF 07 00 03 02 00 0C

	//  ����ģ�� ����    EF 01 FF FF FF FF 01 00 04 02 01 00 08
	//  ����ɹ� ���أ�  EF 01 FF FF FF FF 07 00 03 00 00 0A
	//  ���ʧ�� ���أ�  EF 01 FF FF FF FF 07 00 03 02 00 0C

	//  ����ָ�� ����    EF 01 FF FF FF FF 01 00 08 04 01 00 00 03 E8 00 F9
	//  ����ɹ� ���أ�  EF 01 FF FF FF FF 07 00 07 00 00 05 00 60 00 73 	//��������������
	//  ���ʧ�� ���أ�  EF 01 FF FF FF FF 07 00 07 09 00 00 00 00 00 17  	//00 ����������   09��û�У����������ģ����ֲ�

	if (Command_flag)
	{
		Command_flag = 0;
		Command_flag = 0;

		if (Search_Sign == 0)
		{
			UartData(0xEF);
			UartData(0x01);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0x01);
			UartData(0x00);
			UartData(0x03);
			UartData(0x01);
			UartData(0x00);
			UartData(0x05); //��ȡͼ��
		}
		else if (Search_Sign == 1)
		{
			UartData(0xEF);
			UartData(0x01);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0x01);
			UartData(0x00);
			UartData(0x04);
			UartData(0x02);
			UartData(0x01);
			UartData(0x00);
			UartData(0x08); //����ģ��
		}
		else if (Search_Sign == 2)
		{
			UartData(0xEF);
			UartData(0x01);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0xFF);
			UartData(0x01);
			UartData(0x00);
			UartData(0x08);
			UartData(0x04);
			UartData(0x01);
			UartData(0x00);
			UartData(0x00);
			UartData(0x03);
			UartData(0xE8);
			UartData(0x00);
			UartData(0xF9); //����
		}
	}
}

void Single_Deletion(unsigned char Num) //ָ��ɾ��ָ������
{
	unsigned char Check = 0;
	UartData(0xEF);
	UartData(0x01);
	UartData(0xFF);
	UartData(0xFF);
	UartData(0xFF);
	UartData(0xFF);
	UartData(0x01);
	UartData(0x00);
	UartData(0x07);
	UartData(0x0C);
	UartData(0x00);
	UartData(Num);
	UartData(0x00);
	UartData(0x01);
	Check = 0x01 + 0x07 + 0x0C + Num + 0x01;
	UartData(Check / 256);
	UartData(Check % 256);
}

void Delete_all() //ɾ������ָ������
{
	UartData(0xEF);
	UartData(0x01);
	UartData(0xFF);
	UartData(0xFF);
	UartData(0xFF);
	UartData(0xFF);
	UartData(0x01);
	UartData(0x00);
	UartData(0x03);
	UartData(0x0d);
	UartData(0x00);
	UartData(0x11);
}
/**
  * @brief  The application entry point.
  * @retval int
  */
unsigned char Server_Flag = 0;
unsigned char *dataPtr = NULL;
int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_USART2_UART_Init();
	JDQ_0;
	beep_0;
	HAL_Delay(200); //��ʱ����ϵ磬��Ƭ���������ˣ�Һ����û�����ã�����ɲ���ʾ���������ܶ������������ڳ�ʼ��Һ��֮ǰ����ʱһ��
	LCD12864_int(); //Һ����ʼ��
	LCD12864_Pos(0, 0);
	LCD12864_writebyte((unsigned char *)"    ��ӭʹ��    ");
	LCD12864_Pos(1, 0);
	LCD12864_writebyte((unsigned char *)"  ָ�����������  ");
	LCD12864_Pos(2, 0);
	LCD12864_writebyte((unsigned char *)"                ");
	LCD12864_Pos(3, 0);
	LCD12864_writebyte((unsigned char *)"    �ȴ�����     ");

	LCD12864_Pos(0, 0);
	LCD12864_writebyte("  ��������WiFi  ");
	LCD12864_Pos(1, 0);
	LCD12864_writebyte("                ");
	LCD12864_Pos(2, 0);
	LCD12864_writebyte("    ���Ժ�       ");
	LCD12864_Pos(3, 0);
	LCD12864_writebyte("                ");
	ESP8266_Init();
	LCD12864_Pos(0, 0);
	LCD12864_writebyte("��������WiFi onenet");
	LCD12864_Pos(1, 0);
	LCD12864_writebyte("                  ");
	LCD12864_Pos(2, 0);
	LCD12864_writebyte("    ���Ժ�        ");
	LCD12864_Pos(3, 0);
	LCD12864_writebyte("                  ");
	while (OneNet_DevLink())
		HAL_Delay(500);
	//while (OneNet_Subscribe(sub_topic, 1))
	//	HAL_Delay(500);

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_UART_Receive_IT(&huart2, &aRxBuffer[0], 1); //���ô��ڽ��յ�һ���ֽھͲ����ж�
	read_memory();
	Time2_ms = 950;
	ESP8266_Clear();
	while (1)
	{
		display(); //������ʾ����
		if (state != 17)
		{
			Key_Dispose(); //����������
			key_data_dispose();

			if (fingerprint == 1)
			{
				if (state == 0)
				{
					Search(); //ָ����������
				}
				else
					Search_Sign = 0;

				if (state == 7)
				{
					Register();
				}
			}
		}
		if (Server_Flag != 0)
		{
			if (Server_Flag == 1)
			{
				JDQ_time = 30;
				beep_time = 2;
				system_time = 2;
				state = 16;
			}
			else
			{
				JDQ_time = 0;
				beep_time = 0;
				system_time = 0;
				state = 0;
			}
			Server_Flag = 0;
		}
		dataPtr = ESP8266_GetIPD(0);
		if (dataPtr != NULL)
		{
			OneNet_RevPro(dataPtr);
		}
		if (Heartbeat_Timer >= 30000)   //����������
		{
			Heartbeat_Timer = 0;
			OneNet_Ping();
		}
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the CPU, AHB and APB busses clocks 
  */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks 
  */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 63;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1000;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 57600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_MultiProcessor_Init(&huart2, 0, UART_WAKEUPMETHOD_IDLELINE) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin : PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PB0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB1 PB10 PB11 PB12 
                           PB13 PB14 PB15 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PA8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
