#include "mqtt.h"
#include "tftdriver.h"


#include <WiFiManager.h>
#include <PubSubClient.h>



WiFiClient wificlient1;
PubSubClient mqttClient(wificlient1);

const char* mqttServer = "xx";  //常量, mqtt服务器地址
const int port = 1883;//MQTT服务器端口号
String messageString = "ack";  //发布消息的变量,默认是关闭状态
String publishTopicName = "topicsend" ;
String subscribeTopicName = "home/nodes/sensor/rpi-nas/monitor" ;
String subscribeTopicName1 = "esp8266/control" ;

// 连接MQTT服务器
void connectMQTTServer(){
    // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
    String clientId = "esp8266-" + WiFi.macAddress();

    if (mqttClient.connect(clientId.c_str())) {
      display_debug("MQ Connected.");
      Serial.println("MQTT Server Connected.");
      Serial.print("Server Address:");
      Serial.println(mqttServer);
      Serial.print("ClientId:");
      Serial.println(clientId);
      mqttClient.setBufferSize(1024);
      subscribeTopic(); // ****订阅指定主题****
    } else {
      Serial.print("MQTT Server Connect Failed. Client State:");
      display_debug("MQ Failed.");
      Serial.println(mqttClient.state());
      delay(3000);
    }
}

// 发布信息
void pubMQTTmsg(){
    // 建立发布主题
    char publishTopic[publishTopicName.length() + 1];
    strcpy(publishTopic, publishTopicName.c_str());

    // 建立发布信息:设备的状态
    char publishMsg[messageString.length() + 1];
    strcpy(publishMsg, messageString.c_str());

    // 实现ESP8266向主题发布信息,并在串口监视器显示出来
    if(mqttClient.publish(publishTopic, publishMsg)){
      Serial.print("发布主题:");
      Serial.println(publishTopic);
      Serial.print("发布消息:");
      Serial.println(publishMsg);
    } else {
      Serial.println("消息发布失败。");
    }
}


// 订阅指定主题
void subscribeTopic(){
    char subTopic[subscribeTopicName.length() + 1];
    strcpy(subTopic, subscribeTopicName.c_str());
    char subTopic1[subscribeTopicName1.length() + 1];
    strcpy(subTopic1, subscribeTopicName1.c_str());

    // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
    if(mqttClient.subscribe(subTopic) && mqttClient.subscribe(subTopic1)){
      Serial.print("订阅:");
      Serial.println(subTopic);
    } else {
      Serial.print("Subscribe Fail...");
    }
}

// 收到信息后的回调函数
void receiveCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("接收到订阅消息：[");
    Serial.print(topic);
    Serial.print("] ");
    String msg = "";
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      msg+=(char)payload[i];
    }


    Serial.println("");
    Serial.print("消息长度(Bytes) ");
    Serial.println(length);
    //接收到消息内容
    Serial.print("接收到消息内容：");
    Serial.println(msg);
    //TODO获取到msg内容

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, msg);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    JsonObject json = doc.as<JsonObject>();

    if (subscribeTopicName1 == topic)
    {
      if (!json["info"].isNull()){
        display_debug(json["info"]);
        set_tft_bright(atoi((json["info"].as<String>().c_str())));
      }

      if (!json["switch"].isNull()){
        display_debug(json["switch"]);
        switch_screen(atoi((json["switch"].as<String>().c_str())));
      }
      Serial.println("变更亮度完成...");
      return;
    }

    if (!json["info"].isNull()){
      JsonObject info = json["info"];
      JsonObject cpu = info["cpu"];
      String load = cpu["load_5min_prcnt"];
      String temp = info["temperature_c"];
      display_cpu(load);
      display_temp(temp);
    }

    pubMQTTmsg(); //调用函数发布消息
}
