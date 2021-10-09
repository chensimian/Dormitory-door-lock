const char WiFi_SSID[] = "CMCC-GHPN";                  //WiFi参数
const char WiFi_Password[] = "y7n2qrya";               //WiFi密码
const char Server_IP[] = "183.230.40.39";              //服务器IP
const char Device_ID[] = "688993048";                  //设备ID
const char Product_ID[] = "406537";                    //产品ID
const char API_Key[] = "KQxwjjogXznBs2rG7VYAVr23lTc="; //APIKEY
const uint16_t Server_Port = 6002;                     //服务器端口
#include <WiFi.h>
#include "esp_camera.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_XGA,
    .jpeg_quality = 12,
    .fb_count = 1,
};
bool Photo_Flag = false;
uint8_t RX_Buffer_Count = 3;
const uint16_t RX_Buffer_Length = 50;
char RX_Buffer[RX_Buffer_Length] = {0};
unsigned long HeartBeat = 0;
const uint8_t Img_Head[25] = {0x00, 0x03, 0x24, 0x64, 0x70, 0x02, 0x00, 0x11, 0x7B, 0x22, 0x64, 0x73, 0x5F, 0x69, 0x64, 0x22, 0x3A, 0x22, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x22, 0x7D};
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, Server_IP, Server_Port, Device_ID, Product_ID, API_Key);
Adafruit_MQTT_Publish Dp = Adafruit_MQTT_Publish(&mqtt, "$dp");
Adafruit_MQTT_Subscribe Door = Adafruit_MQTT_Subscribe(&mqtt, "kai");
Adafruit_MQTT_Subscribe Photo = Adafruit_MQTT_Subscribe(&mqtt, "paizhao");
void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 14, 15);
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);
  if (Camera_Init() != ESP_OK)
  {
    digitalWrite(33, LOW);
    for (;;)
      delay(1);
  }
  WiFi_Init();
}

void loop()
{
  MQTT_Connect();
  if (Photo_Flag)
  {
    digitalWrite(33, LOW);
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
      Serial.println(F("Camera capture failed."));
    else
    {
      Photo_Flag = false;
      Serial.print(F("Size:"));
      Serial.print(fb->width);
      Serial.print("x");
      Serial.print(fb->height);
      Serial.print("\t");
      Serial.println(fb->len);
      //包头
      client.write(0x30);
      //长度
      uint8_t Msg_buffer[4] = {0};
      unsigned long Msg_Len = fb->len + 29;
      Msg_buffer[0] = uint8_t(Msg_Len % 128);
      Msg_buffer[1] = uint8_t(Msg_Len / 128);
      if (Msg_buffer[1] != 0)
        Msg_buffer[0] |= 0x80;
      Msg_buffer[2] = uint8_t(Msg_Len / 16384);
      if (Msg_buffer[2] != 0)
        Msg_buffer[1] |= 0x80;
      Msg_buffer[3] = uint8_t(Msg_Len / 2097152);
      if (Msg_buffer[3] != 0)
        Msg_buffer[2] |= 0x80;
      for (int i = 0; i < 4; i++)
      {
        if (Msg_buffer[i] != 0)
          client.write(Msg_buffer[i]);
      }
      //协议头
      client.write(Img_Head, 25);
      //BIN数据
      client.write(uint8_t(fb->len >> 24));
      client.write(uint8_t(fb->len >> 16));
      client.write(uint8_t(fb->len >> 8));
      client.write(uint8_t(fb->len));
      client.write(fb->buf, fb->len);
      Serial.println(F("Succes to send image for MQTT."));
    }
    esp_camera_fb_return(fb);
    digitalWrite(33, HIGH);
  }
  mqtt.processPackets(3000);
  STM32_Run();
  if (millis() - HeartBeat >= 60000)
  {
    HeartBeat = millis();
    Serial.println(F("MQTT Ping"));
    if (!mqtt.ping())
      mqtt.disconnect();
  }
}
void WiFi_Init(void)
{
  Serial.print(F("\r\n\r\nConnecting to "));
  Serial.print(WiFi_SSID);
  WiFi.begin(WiFi_SSID, WiFi_Password);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(33, !digitalRead(33));
    delay(500);
    Serial.print(".");
  }
  digitalWrite(33, HIGH);
  Serial.print(F("\r\nWiFi Connected\r\nIP address: "));
  Serial.println(WiFi.localIP());
  Door.setCallback(Door_Callback);
  Photo.setCallback(Photo_Callback);
  mqtt.subscribe(&Door);
  mqtt.subscribe(&Photo);
}
esp_err_t Camera_Init(void)
{
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    Serial.print(F("Camera Init Failed"));
    return err;
  }
  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV2640_PID)
    ;
  Serial.print(F("Camera Init OK"));
  return ESP_OK;
}
void STM32_Run(void)
{
  while (Serial1.available())
  {
    RX_Buffer[RX_Buffer_Count] = Serial1.read();
    if (RX_Buffer[RX_Buffer_Count] == '}')
    {
      Serial.println(F("Receive complete JSON data."));
      for (int i = 3; i <= RX_Buffer_Count; i++)
        Serial.print(RX_Buffer[i]);
      Serial.println();
      RX_Buffer[0] = 0x03;
      RX_Buffer[1] = uint8_t((RX_Buffer_Count - 2) >> 8);
      RX_Buffer[2] = uint8_t(RX_Buffer_Count - 2);
      //此处发送MQTT消息
      Dp.publish((uint8_t *)RX_Buffer, RX_Buffer_Count + 1);
      RX_Buffer_Count = 3;
      memset(RX_Buffer, 0, RX_Buffer_Length);
    }
    if (RX_Buffer[3] == '{')
      RX_Buffer_Count++;
  }
}
void Photo_Callback(char *data, uint16_t len)
{
  Serial.print(F("Photo callback: "));
  Serial.println(data);
  if (strstr(data, "1") != NULL)
  {
    Serial.println(F("Report photo."));
    Photo_Flag = true;
  }
}
void Door_Callback(char *data, uint16_t len)
{
  Serial.print(F("Door callback: "));
  Serial.println(data);
  if (strstr(data, "1") != NULL)
  {
    Serial.println(F("->STM32 A."));
    Serial1.print('A');
  }
  else if (strstr(data, "0") != NULL)
  {
    Serial.println(F("->STM32 B."));
    Serial1.print('B');
  }
}
void MQTT_Connect()
{
  int8_t ret;
  if (mqtt.connected())
  {
    return;
  }
  Serial.println(F("Connecting to MQTT... "));
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println(F("Retrying MQTT connection in 3 seconds..."));
    mqtt.disconnect();
    delay(3000);
    retries--;
    if (retries == 0)
    {
      while (1)
        ;
    }
  }
  Serial.println(F("MQTT Connected!"));
}
