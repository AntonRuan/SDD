#include <TimeLib.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <ArduinoJson.h>

#include "tftdriver.h"
#include "weathernum.h"
#include "number.h"
#include "Animate.h"

#include "../img/temperature.h"
#include "../img/humidity.h"
#include "../font/ZdyLwFont_20.h"


// LCD屏幕相关设置
TFT_eSPI tft = TFT_eSPI(); // 引脚请自行配置tft_espi库中的 User_Setup.h文件
TFT_eSprite clk = TFT_eSprite(&tft);
#define LCD_BL_PIN 5 // LCD背光引脚
uint16_t bgColor = 0x0000;
//其余状态标志位
int LCD_Rotation = 0;        // LCD屏幕方向
int LCD_BL_PWM = 50;         //屏幕亮度0-100，默认50
byte loadNum = 6;

Number      dig;
WeatherNum wrat;

void Web_win()
{
  clk.setColorDepth(8);

  clk.createSprite(200, 60); //创建窗口
  clk.fillSprite(0x0000);    //填充率

  clk.setTextDatum(CC_DATUM); //设置文本数据
  clk.setTextColor(TFT_GREEN, 0x0000);
  clk.drawString("WiFi Connect Fail!", 100, 10, 2);
  clk.drawString("SSID:", 45, 40, 2);
  clk.setTextColor(TFT_WHITE, 0x0000);
  clk.drawString("AutoConnectAP", 125, 40, 2);
  clk.pushSprite(20, 50); //窗口位置

  clk.deleteSprite();
}

int loading()//绘制进度条
{
  clk.setColorDepth(8);

  clk.createSprite(200, 100);//创建窗口
  clk.fillSprite(0x0000);   //填充率

  clk.drawRoundRect(0,0,200,16,8,0xFFFF);       //空心圆角矩形
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);   //实心圆角矩形
  clk.setTextDatum(CC_DATUM);   //设置文本数据
  clk.setTextColor(TFT_GREEN, 0x0000);
  clk.drawString("Connecting to WiFi......",100,40,2);
  clk.setTextColor(TFT_WHITE, 0x0000);
  clk.drawRightString("SSR V1.0",180,60,2);
  clk.pushSprite(20,110);  //窗口位置

  clk.deleteSprite();
  loadNum += 50;
  return loadNum;
}

void loaded(String ip)
{
  while (loadNum < 194) //让动画走完
  {
    clk.setColorDepth(8);

    clk.createSprite(200, 100);//创建窗口
    clk.fillSprite(0x0000);   //填充率

    clk.drawRoundRect(0,0,200,16,8,0xFFFF);       //空心圆角矩形
    clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);   //实心圆角矩形
    clk.setTextDatum(CC_DATUM);   //设置文本数据
    clk.setTextColor(TFT_GREEN, 0x0000);
    clk.drawString("Connected to " + ip,100,40,2);
    clk.setTextColor(TFT_WHITE, 0x0000);
    clk.drawRightString("SSR V1.0",180,60,2);
    clk.pushSprite(20,110);  //窗口位置

    clk.deleteSprite();
    loadNum += 1;
    delay(1);
  }
}

void display_temp(String str)
{
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
  clk.createSprite(88, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString("Temp: " + str + "℃", 28, 13);
  clk.pushSprite(15, 190);
  clk.deleteSprite();
  clk.unloadFont();
}

void display_cpu(String str)
{
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
  clk.createSprite(88, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString("CPU: " + str, 28, 13);
  clk.pushSprite(15, 220);
  clk.deleteSprite();
  clk.unloadFont();
}

//星期
String week()
{
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

//月日
String monthDay()
{
  String s = String(month()) + "月" + String(day()) + "日";
  return s;
}

void digitalClockDisplay()
{
  static unsigned char Hour_sign   = 60;
  static unsigned char Minute_sign = 60;
  static unsigned char Second_sign = 60;
  int timey = 82;

  if (hour() != Hour_sign)//时钟刷新
  {
    dig.printfW3660(20,timey,hour()/10);
    dig.printfW3660(60,timey,hour()%10);
    Hour_sign = hour();
  }
  if (minute() != Minute_sign)//分钟刷新
  {
    dig.printfO3660(101,timey,minute()/10);
    dig.printfO3660(141,timey,minute()%10);
    Minute_sign = minute();
  }
  if (second() != Second_sign)//分钟刷新
  {
    dig.printfW1830(182,timey+30,second()/10);
    dig.printfW1830(202,timey+30,second()%10);
    Second_sign = second();
  }

  /***日期****/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);

  //星期
  clk.createSprite(58, 30);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(week(),29,16);
  clk.pushSprite(102,150);
  clk.deleteSprite();

  //月日
  clk.createSprite(95, 30);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(monthDay(),49,16);
  clk.pushSprite(5,150);
  clk.deleteSprite();

  clk.unloadFont();
  /***日期****/
}

// 天气信息写到屏幕上
void weaterData(String *cityDZ, String *dataSK, String *dataFC)
{
  // 解析第一段JSON
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();

  TJpgDec.drawJpg(10,45,temperature, sizeof(temperature));  //温度图标
  TJpgDec.drawJpg(90,45,humidity, sizeof(humidity));  //湿度图标

  /***绘制相关文字***/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);

  // 温度
  clk.createSprite(58, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(sk["temp"].as<String>() + "℃", 28, 13);
  clk.pushSprite(30, 45);
  clk.deleteSprite();

  // 湿度
  clk.createSprite(58, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(sk["SD"].as<String>(), 28, 13);
  clk.pushSprite(110, 45);
  clk.deleteSprite();

  // 城市名称
  clk.createSprite(94, 30);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(sk["cityname"].as<String>(), 44, 16);
  clk.pushSprite(15, 15);
  clk.deleteSprite();

  // PM2.5空气指数
  uint16_t pm25BgColor = tft.color565(156, 202, 127); //优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
  if (pm25V > 200)
  {
    pm25BgColor = tft.color565(136, 11, 32); //重度
    aqiTxt = "重度";
  }
  else if (pm25V > 150)
  {
    pm25BgColor = tft.color565(186, 55, 121); //中度
    aqiTxt = "中度";
  }
  else if (pm25V > 100)
  {
    pm25BgColor = tft.color565(242, 159, 57); //轻
    aqiTxt = "轻度";
  }
  else if (pm25V > 50)
  {
    pm25BgColor = tft.color565(247, 219, 100); //良
    aqiTxt = "良";
  }
  clk.createSprite(56, 24);
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0, 0, 50, 24, 4, pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0x0000);
  clk.drawString(aqiTxt, 25, 13);
  clk.pushSprite(104, 18);
  clk.deleteSprite();

  //天气图标
  wrat.printfweather(170, 15, atoi((sk["weathercode"].as<String>()).substring(1, 3).c_str()));

  clk.unloadFont();
}

int Amimate_reflash_Time = 0; //更新时间记录
const uint8_t *Animate_value; //指向关键帧的指针
uint32_t Animate_size;        //指向关键帧大小的指针
void refresh_AnimatedImage()
{
  if (millis() - Amimate_reflash_Time > 100) // x ms切换一次
  {
    Amimate_reflash_Time = millis();
    imgAnim(&Animate_value, &Animate_size);
    TJpgDec.drawJpg(160, 160, Animate_value, Animate_size);
  }
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void tft_init()
{
  pinMode(LCD_BL_PIN, OUTPUT);
  analogWrite(LCD_BL_PIN, 1023 - (LCD_BL_PWM * 10));

  tft.begin();          /* TFT init */
  tft.invertDisplay(1); //反转所有显示颜色：1反转，0正常
  tft.setRotation(LCD_Rotation);
  tft.fillScreen(0x0000);
  tft.setTextColor(TFT_BLACK, bgColor);

  loading();
}

void jpg_init()
{
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  tft.fillScreen(TFT_BLACK);//清屏
}

void time_loop()
{
  static time_t prevDisplay = 0;
  if (now() != prevDisplay)
  {
    prevDisplay = now();
    digitalClockDisplay();
  }
}