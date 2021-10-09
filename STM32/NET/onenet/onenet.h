#ifndef _ONENET_H_
#define _ONENET_H_





_Bool OneNet_DevLink(void);

void OneNet_SendData(void);

void OneNet_RevPro(unsigned char *cmd);

_Bool OneNet_Subscribe(const char *topics[], unsigned char topic_cnt);

_Bool OneNet_Ping(void);

#endif
