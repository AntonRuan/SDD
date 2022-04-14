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
void refresh_AnimatedImage();

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

  mqtt_init();

  reflash_time.setInterval(300);
  reflash_time.onRun(reflashTime);
  reflash_Animate.setInterval(100);
  reflash_Animate.onRun(refresh_AnimatedImage);
  controller.run();
}

void loop() {
  if (controller.shouldRun())
    controller.run();

  mqtt_loop();
  httpServer.handleClient();
}

void reflashTime() {
  time_loop();
  wifi_loop();
  mqtt_connect_loop();
  weater_loop();
}

void refresh_AnimatedImage() {
  int speed = anim_loop();
  reflash_Animate.setInterval(speed);
}