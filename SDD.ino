#include <EEPROM.h>
#include <Thread.h>                 //协程
#include <StaticThreadController.h> //协程控制
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>

#include "src/netmanager.h"
#include "src/tftdriver.h"
#include "src/weather.h"
#include "src/mqtt.h"


ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//创建时间更新函数线程
Thread reflash_time = Thread();
//创建副标题切换线程
Thread reflash_Banner = Thread();
//创建恢复WIFI链接
Thread reflash_openWifi = Thread();
//创建动画绘制线程
Thread reflash_Animate = Thread();

//创建协程池
StaticThreadController<4> controller(&reflash_time, &reflash_Banner, &reflash_openWifi, &reflash_Animate);

void reflashTime();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(1024);
  tft_init();
  wifi_connect();
  jpg_init();

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  getCityCode();
  //getCityWeater();

  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, port);
  // 收到信息后的回调函数
  mqttClient.setCallback(receiveCallback);
  // 连接MQTT服务器
  connectMQTTServer();

  reflash_time.setInterval(300);
  reflash_time.onRun(reflashTime);
  reflash_Animate.setInterval(10); //设置帧率
  reflash_Animate.onRun(refresh_AnimatedImage);
  controller.run();
}

void loop() {
  // put your main code here, to run repeatedly:
  httpServer.handleClient();
  if (controller.shouldRun())
    controller.run();

  if (mqttClient.connected())
    mqttClient.loop();
}

void reflashTime() {
  time_loop();
  wifi_loop();
  if (!mqttClient.connected()) {
    connectMQTTServer();
  }
}