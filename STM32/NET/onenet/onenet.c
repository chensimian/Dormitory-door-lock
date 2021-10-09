#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include <string.h>
#include <stdio.h>

#define PROID "406537"	  //��ƷID
#define AUTH_INFO "1234"  //��Ȩ��Ϣ
#define DEVID "688993048" //�豸ID
extern unsigned char Server_Flag;
extern unsigned char esp8266_buf[256];
extern unsigned int Fingerprint_Num;
extern unsigned char Password[6];
uint8_t status_connected = 0;
//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //Э���

	unsigned char *dataPtr;

	_Bool status = 1;

	//printf("OneNet_DevLink\r\nPROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);

	if (++status_connected >= 1)
	{
		//printf("Error\r\n");
	}

	if (MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); //�ϴ�ƽ̨

		dataPtr = ESP8266_GetIPD(250); //�ȴ�ƽ̨��Ӧ
		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch (MQTT_UnPacketConnectAck(dataPtr))
				{
				case 0:
					//printf("Tips:	���ӳɹ�\r\n");
					status = 0;
					break;

				case 1:
					//printf("WARN:	����ʧ�ܣ�Э�����\r\n");
					break;
				case 2:
					//printf("WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");
					break;
				case 3:
					//printf("WARN:	����ʧ�ܣ�������ʧ��\r\n");
					break;
				case 4:
					//printf("WARN:	����ʧ�ܣ��û������������\r\n");
					break;
				case 5:
					//printf("WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");
					break;

				default:
					//printf("ERR:	����ʧ�ܣ�δ֪����\r\n");
					break;
				}
			}
		}

		MQTT_DeleteBuffer(&mqttPacket); //ɾ��
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
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����
//==========================================================
void OneNet_SendData(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //Э���

	char buf[128];

	short body_len = 0, i = 0;

	//printf("Tips:	OneNet_SendData-MQTT\r\n");

	memset(buf, 0, sizeof(buf));

	body_len = OneNet_FillBuf(buf); //��ȡ��ǰ��Ҫ���͵����������ܳ���

	if (body_len)
	{
		if (MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0) //���
		{
			for (; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];

			ESP8266_SendData(mqttPacket._data, mqttPacket._len); //�ϴ����ݵ�ƽ̨
			//printf("Send %d Bytes\r\n", mqttPacket._len);
			MQTT_DeleteBuffer(&mqttPacket); //ɾ��
		}
		//else
		//printf("WARN:	EDP_NewBuffer Failed\r\n");
	}
}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����
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
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //Э���

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	unsigned short req_len = 0;

	unsigned char type = 0;

	short result = 0;

	type = MQTT_UnPacketRecv(cmd);
	switch (type)
	{
	case MQTT_PKT_CMD: //�����·�

		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len); //���topic����Ϣ��
		if (result == 0)
		{
			//printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0) //����ظ����
			{
				//printf("Tips:	Send CmdResp\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len); //�ظ�����
				MQTT_DeleteBuffer(&mqttPacket);						 //ɾ��
			}
		}

		break;

	case MQTT_PKT_PUBACK: //����Publish��Ϣ��ƽ̨�ظ���Ack

		if (MQTT_UnPacketPublishAck(cmd) == 0)
			//printf("Tips:	MQTT Publish Send OK\r\n");

			break;

	default:
		result = -1;
		break;
	}

	ESP8266_Clear(); //��ջ���

	if (result == -1)
		return;
	if (type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}
//==========================================================
//	�������ƣ�	OneNet_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����
//==========================================================
_Bool OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{

	//unsigned char i = 0;
	//unsigned char *dataPtr = NULL;
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //Э���

	// for(; i < topic_cnt; i++)
	// 	UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);

	//---------------------------------------------����һ�����---------------------------------------------
	if (MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
	{
		//---------------------------------------------���������������-----------------------------------------
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); //��ƽ̨���Ͷ�������
	}
	MQTT_DeleteBuffer(&mqttPacket); //ɾ��
	return 0;
}
_Bool OneNet_Ping(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; //Э���
	if (MQTT_PacketPing(&mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
	}
	MQTT_DeleteBuffer(&mqttPacket);
	return 0;
}
