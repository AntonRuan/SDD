#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#include "netmanager.h"
#include "tftdriver.h"


struct config_type
{
  char stassid[32]; //定义配网得到的WIFI名长度(最大32字节)
  char stapsw[64];  //定义配网得到的WIFI密码长度(最大64字节)
};

ESP8266WiFiMulti wifiMulti;
WiFiManager wm;
WiFiUDP Udp;

const uint32_t connectTimeoutMs = 5000;
int wifi_addr = 30; //被写入数据的EEPROM地址编号  20wifi-ssid-psw
config_type wificonf = {{"账号"}, {"密码"}};


void Webconfig()
{
  WiFi.mode(WIFI_STA);
  delay(3000);
  wm.resetSettings();

  //wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  //
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi", "restart"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  wm.setMinimumSignalQuality(20);

  bool res = wm.autoConnect("AutoConnectAP");
  while (!res)
  {
    ;
  }
}

//删除原有eeprom中的信息
void deletewificonfig()
{
  config_type deletewifi = {{""}, {""}};
  uint8_t *p = (uint8_t *)(&deletewifi);
  for (unsigned int i = 0; i < sizeof(deletewifi); i++)
  {
    EEPROM.write(i + wifi_addr, *(p + i)); //在闪存内模拟写入
  }
  delay(10);
  EEPROM.commit(); //执行写入ROM
  delay(10);
}

//从eeprom读取WiFi信息ssid，psw
void readwificonfig()
{
  uint8_t *p = (uint8_t *)(&wificonf);
  for (unsigned int i = 0; i < sizeof(wificonf); i++)
  {
    *(p + i) = EEPROM.read(i + wifi_addr);
  }

  Serial.printf("Read WiFi Config.....\r\n");
  Serial.printf("SSID:%s\r\n", wificonf.stassid);
  Serial.printf("PSW:%s\r\n", wificonf.stapsw);
}

// wifi ssid，psw保存到eeprom
void savewificonfig()
{
  //开始写入
  uint8_t *p = (uint8_t *)(&wificonf);
  for (unsigned int i = 0; i < sizeof(wificonf); i++)
  {
    EEPROM.write(i + wifi_addr, *(p + i)); //在闪存内模拟写入
  }
  delay(10);
  EEPROM.commit(); //执行写入ROM
  delay(10);

  Serial.printf("Save WiFi Config.....\r\n");
  Serial.printf("SSID:%s\r\n", wificonf.stassid);
  Serial.printf("PSW:%s\r\n", wificonf.stapsw);
}

int wifi_connect() {
  WiFi.persistent(false);
  Serial.println("\r\n正在连接WIFI: ");

  readwificonfig();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(wificonf.stassid, wificonf.stapsw);

  wifiMulti.addAP("OpenWrt", "12345678");

  while (wifiMulti.run() != WL_CONNECTED)
  {
    int progress = loading();
    if (progress >= 194)
    {
      Serial.println("Webconfig");
      Web_win();
      Webconfig();
      break;
    }
    delay(500);
  }

  loaded(WiFi.localIP().toString());
  Serial.print("WiFi connected: ");
  Serial.print(WiFi.SSID());
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  Udp.begin(8000);
  Serial.println("等待同步...");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  if (WiFi.status() == WL_CONNECTED)
  {
    strcpy(wificonf.stassid, WiFi.SSID().c_str()); //名称复制
    strcpy(wificonf.stapsw, WiFi.psk().c_str());   //密码复制
    savewificonfig();
  }

  return 0;
}


/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
//NTP服务器
static const char ntpServerName[] = "ntp6.aliyun.com";
const int timeZone = 8;     //东八区

// 向NTP服务器发送请求
void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision

  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  IPAddress ntpServerIP;

  while (Udp.parsePacket() > 0) ;

  WiFi.hostByName(ntpServerName, ntpServerIP);

  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;

      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 无法获取时间时返回0
}

