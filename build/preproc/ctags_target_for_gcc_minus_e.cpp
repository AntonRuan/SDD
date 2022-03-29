# 1 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino"
# 2 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 3 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 4 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 5 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 6 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 7 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 8 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2

# 10 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 11 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2
# 12 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 2


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

void reflashTime()
{
  time_loop();
}

WiFiClient wificlient1;
PubSubClient mqttClient(wificlient1);

const char* mqttServer = "52.77.221.206"; //常量, mqtt服务器地址
const int port = 1883;//MQTT服务器端口号
String messageString = "ack"; //发布消息的变量,默认是关闭状态
String publishTopicName = "topicsend" ;
String subscribeTopicName = "home/nodes/sensor/rpi-nas/monitor" ;

// 连接MQTT服务器
void connectMQTTServer(){
    // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
    String clientId = "esp8266-" + WiFi.macAddress();

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT Server Connected.");
      Serial.print("Server Address:");
      Serial.println(mqttServer);
      Serial.print("ClientId:");
      Serial.println(clientId);
      mqttClient.setBufferSize(1024);
      subscribeTopic(); // ****订阅指定主题****
    } else {
      Serial.print("MQTT Server Connect Failed. Client State:");
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

    // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
    if(mqttClient.subscribe(subTopic)){
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
      Serial.print(((reinterpret_cast<const __FlashStringHelper *>(
# 121 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "SDD.ino" "." "121" "." "140" "\", \"aSM\", @progbits, 1 #"))) = (
# 121 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino"
                  "deserializeJson() failed: "
# 121 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino" 3
                  ); &__pstr__[0];}))
# 121 "/Users/shuangshuangruan/Documents/Arduino/SDD/SDD.ino"
                  ))));
      Serial.println(error.f_str());
      return;
    }
    JsonObject json = doc.as<JsonObject>();

    if (!json["info"].isNull()){
      JsonObject info = json["info"];
      JsonObject cpu = info["cpu"];
      String load = cpu["load_1min_prcnt"];
      String temp = info["temperature_c"];
      display_cpu(load);
      display_temp(temp);
      Serial.println("变更亮度完成...");
    }

    pubMQTTmsg(); //调用函数发布消息
}

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
  reflash_Animate.setInterval(100); //设置帧率
  reflash_Animate.onRun(refresh_AnimatedImage);
  controller.run();
}

void loop() {
  // put your main code here, to run repeatedly:
  httpServer.handleClient();
  if (controller.shouldRun())
    controller.run();

  //MQTT服务连接检测
  if (mqttClient.connected()) { // 如果开发板成功连接服务器
    mqttClient.loop();
  } else { // 如果开发板未能成功连接服务器
    connectMQTTServer(); // 则尝试连接服务器
  }
}
