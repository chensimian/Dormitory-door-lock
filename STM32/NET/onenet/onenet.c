#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include <string.h>
#include <stdio.h>

#define PROID "406537"	  //产品ID
#define AUTH_INFO "1234"  //鉴权信息
#define DEVID "688993048" //设备ID
extern unsigned char Server_Flag;
extern unsigned char esp8266_buf[256];
extern unsigned int Fingerprint_Num;
extern unsigned char Password[6];
uint8_t status_connected = 0;
//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //协议包

	unsigned char *dataPtr;

	_Bool status = 1;

	//printf("OneNet_DevLink\r\nPROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);

	if (++status_connected >= 1)
	{
		//printf("Error\r\n");
	}

	if (MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); //上传平台

		dataPtr = ESP8266_GetIPD(250); //等待平台响应
		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch (MQTT_UnPacketConnectAck(dataPtr))
				{
				case 0:
					//printf("Tips:	连接成功\r\n");
					status = 0;
					break;

				case 1:
					//printf("WARN:	连接失败：协议错误\r\n");
					break;
				case 2:
					//printf("WARN:	连接失败：非法的clientid\r\n");
					break;
				case 3:
					//printf("WARN:	连接失败：服务器失败\r\n");
					break;
				case 4:
					//printf("WARN:	连接失败：用户名或密码错误\r\n");
					break;
				case 5:
					//printf("WARN:	连接失败：非法链接(比如token非法)\r\n");
					break;

				default:
					//printf("ERR:	连接失败：未知错误\r\n");
					break;
				}
			}
		}

		MQTT_DeleteBuffer(&mqttPacket); //删包
	}
	//else
	//printf("WARN:	MQTT_PacketConnect Failed\r\n");

	return status;
}

unsigned char OneNet_FillBuf(char *buf)
{
	char text[32];
	strcpy(buf, ",;");
	memset(text, 0, sizeof(text));
	sprintf(text, "fingerprintID,%d;", Fingerprint_Num);
	strcat(buf, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "Password,%d%d%d%d%d%d;", Password[0], Password[1], Password[2], Password[3], Password[4], Password[5]);
	strcat(buf, text);
	return strlen(buf);
}

//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNet_SendData(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //协议包

	char buf[128];

	short body_len = 0, i = 0;

	//printf("Tips:	OneNet_SendData-MQTT\r\n");

	memset(buf, 0, sizeof(buf));

	body_len = OneNet_FillBuf(buf); //获取当前需要发送的数据流的总长度

	if (body_len)
	{
		if (MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0) //封包
		{
			for (; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];

			ESP8266_SendData(mqttPacket._data, mqttPacket._len); //上传数据到平台
			//printf("Send %d Bytes\r\n", mqttPacket._len);
			MQTT_DeleteBuffer(&mqttPacket); //删包
		}
		//else
		//printf("WARN:	EDP_NewBuffer Failed\r\n");
	}
}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	// Usart1_SendString(cmd, strlen(cmd));
	// Usart1_SendString("\r\n", strlen("\r\n"));
	if (strstr((char *)cmd + 3, "kai1"))
	{
		Server_Flag = 1;
		//Usart1_SendString("Open\r\n", strlen("Open\r\n"));
	}
	if (strstr((char *)cmd + 3, "kai0"))
	{
		Server_Flag = 2;
		//Usart1_SendString("Close\r\n", strlen("Close\r\n"));
	}
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //协议包

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	unsigned short req_len = 0;

	unsigned char type = 0;

	short result = 0;

	type = MQTT_UnPacketRecv(cmd);
	switch (type)
	{
	case MQTT_PKT_CMD: //命令下发

		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len); //解出topic和消息体
		if (result == 0)
		{
			//printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0) //命令回复组包
			{
				//printf("Tips:	Send CmdResp\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len); //回复命令
				MQTT_DeleteBuffer(&mqttPacket);						 //删包
			}
		}

		break;

	case MQTT_PKT_PUBACK: //发送Publish消息，平台回复的Ack

		if (MQTT_UnPacketPublishAck(cmd) == 0)
			//printf("Tips:	MQTT Publish Send OK\r\n");

			break;

	default:
		result = -1;
		break;
	}

	ESP8266_Clear(); //清空缓存

	if (result == -1)
		return;
	if (type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}
//==========================================================
//	函数名称：	OneNet_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	0-成功	1-失败
//
//	说明：
//==========================================================
_Bool OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{

	//unsigned char i = 0;
	//unsigned char *dataPtr = NULL;
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //协议包

	// for(; i < topic_cnt; i++)
	// 	UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);

	//---------------------------------------------步骤一：组包---------------------------------------------
	if (MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
	{
		//---------------------------------------------步骤二：发送数据-----------------------------------------
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); //向平台发送订阅请求
	}
	MQTT_DeleteBuffer(&mqttPacket); //删包
	return 0;
}
_Bool OneNet_Ping(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //协议包
	if (MQTT_PacketPing(&mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
	}
	MQTT_DeleteBuffer(&mqttPacket);
	return 0;
}
