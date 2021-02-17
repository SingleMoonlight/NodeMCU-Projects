#include <ESP8266WiFi.h>                         //ESP8266WiFi库
#include <PubSubClient.h>                        //PubSubClient
#include <dht11.h>                               //dht11库
#include <Ticker.h>                              //Ticker库

const char* ssid = "********";                   //WiFi名称
const char* password = "********";               //WiFi密码
const char* mqttServer = "test.mosquitto.org";   //MQTT服务器地址

WiFiClient wifiClient;                           //建立WiFiClient对象
PubSubClient mqttClient(wifiClient);             //建立mqttClient对象
dht11 DHT11;                                     //建立dht11传感器对象

Ticker sentInfoTicker;                           //向服务器发送温湿度信息定时器
int sentInfoInterval = 0;                        //发送温湿度信息时间间隔


void setup() {
  //设置串口波特率
  Serial.begin(9600);
  //设置LED引脚为输出模式
  pinMode(LED_BUILTIN, OUTPUT);
  //设置es8266工作在station模式(无线终端模式)
  WiFi.mode(WIFI_STA);
  //连接WiFi
  connectWifi();

  //设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 1883);
  //设置收到信息后的回调函数
  mqttClient.setCallback(receiveCallback);
  //连接MQTT服务器
  connectMQTTserver();

  //开启发送温湿度信息定时器
  sentInfoTicker.attach(1, tickerSentInfoInterval);
}

void loop() {
  //如果开发板成功连接服务器
  if (mqttClient.connected()) { 
    //每隔10s
    if(sentInfoInterval >= 10) {
      //向服务器发送温湿度信息
      publishTempAndHum();
      //重新计时
      sentInfoInterval = 0;
    }
    //维持心跳
    mqttClient.loop();          
  } 
  //如果开发板未能成功连接服务器，则尝试重新连接服务器
  else {                    
    connectMQTTserver();        
  }
}

/******************************************************
   函 数 名: tickerSentInfoInterval
   描   述: 用于向服务器发送温湿度信息定时(每隔1s执行一次)
   参   数: 无
   返 回 值: 无
 ******************************************************/
void tickerSentInfoInterval() {
  sentInfoInterval++;
}

/******************************************************
   函 数 名: connectWifi
   描   述: 用于连接WiFi
   参   数: 无
   返 回 值: 无
 ******************************************************/
void connectWifi() {
  //连接WiFi
  WiFi.begin(ssid, password);
  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  //显示连接成功的WiFi信息
  //WiFi名称
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  //IP地址
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/******************************************************
   函 数 名: connectMQTTserver
   描   述: 连接MQTT服务器并订阅信息
   参   数: 无
   返 回 值: 无
 ******************************************************/
void connectMQTTserver(){
  //设置客户端ID(尽量独一无二，避免重复)
  String clientId = "NodeMCU-" + WiFi.macAddress();
 
  //成功连接MQTT服务器
  if (mqttClient.connect(clientId.c_str())) { 
    Serial.println("MQTT Server Connected");
    Serial.println("Server Address: " + String(mqttServer));
    Serial.println("ClientId: " + clientId);
    //订阅主题
    subscribeTopic(); 
  } 
  //连接失败
  else {
    //输出状态码
    Serial.print("MQTT server connect failed! Client state: ");
    Serial.println(mqttClient.state());
    //延迟5s再尝试重连
    delay(5000);
  }   
}

/******************************************************
   函 数 名: subscribeTopic
   描   述: 订阅指定主题
   参   数: 无
   返 回 值: 无
 ******************************************************/
void subscribeTopic(){
  //订阅LED状态控制主题(尽量独一无二，以免收到使用同一服务器客户端的无用信息)
  String ledStateTopicString = WiFi.macAddress() + "/control/ledState";
  char ledStateTopic[ledStateTopicString.length() + 1];  
  strcpy(ledStateTopic, ledStateTopicString.c_str());

  //订阅LED亮度调节主题(尽量独一无二，以免收到使用同一服务器客户端的无用信息)
  String ledBrightnessTopicString = WiFi.macAddress() + "/control/ledBrightness";
  char ledBrightnessTopic[ledBrightnessTopicString.length() + 1];  
  strcpy(ledBrightnessTopic, ledBrightnessTopicString.c_str());
  
  //如果订阅成功，输出订阅信息
  if(mqttClient.subscribe(ledStateTopic)){
    Serial.println("Subscrib Topic: " + ledStateTopicString);
  } else {
    Serial.print("Subscribe fail!");
  }  

  //如果订阅成功，输出订阅信息
  if(mqttClient.subscribe(ledBrightnessTopic)){
    Serial.println("Subscrib Topic: " + ledBrightnessTopicString);
  } else {
    Serial.print("Subscribe fail!");
  } 
}

/******************************************************
   函 数 名: receiveCallback
   描   述: 收到信息后的回调函数
   参   数: topic 主题，payload 消息(有效负载)，length 消息(有效负载)长度
   返 回 值: 无
 ******************************************************/
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  //订阅的主题
  Serial.println("Receive message from topic: " + String(topic));
  //该主题的消息
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Message: " + message);
  //消息的长度
  Serial.println("Message Length: " + String(length) + " Bytes");

  //根据主题以及消息内容执行响应的动作
  //LED状态控制主题
  if(String(topic) == "48:3F:DA:9D:09:BA/control/ledState"){
    controlLedState(message);
  }
  //LED亮度调节主题
  if(String(topic) == "48:3F:DA:9D:09:BA/control/ledBrightness"){
    controlLedBrightness(message);
  }
}

/******************************************************
   函 数 名: publishTempAndHum
   描   述: 读取并发送dht11的温湿度信息
   参   数: 无
   返 回 值: 无
 ******************************************************/
void publishTempAndHum() {
  //读取传感器信息，传感器连接引脚(GPIO14 D5)
  DHT11.read(14); 

  //建立温度发布主题
  String tempTopicString = WiFi.macAddress() + "/monitor/temperature";
  char tempTopic[tempTopicString.length() + 1];  
  strcpy(tempTopic, tempTopicString.c_str());
 
  //建立温度发布信息
  String tempMessageString = String(DHT11.temperature); 
  char tempMsg[tempMessageString.length() + 1];   
  strcpy(tempMsg, tempMessageString.c_str());
  
  //向温度主题发布信息
  if(mqttClient.publish(tempTopic, tempMsg)){
    Serial.println("Publish Topic: " + tempTopicString);
    Serial.println("Temperature message: " + tempMessageString);  
  } else {
    Serial.println("Temperature message publish failed!"); 
  }

  //建立湿度发布主题
  String humTopicString = WiFi.macAddress() + "/monitor/humidity";
  char humTopic[humTopicString.length() + 1];  
  strcpy(humTopic, humTopicString.c_str());
 
  //建立湿度发布信息
  String humMessageString = String(DHT11.humidity); 
  char humMsg[humMessageString.length() + 1];   
  strcpy(humMsg, humMessageString.c_str());
  
  //向湿度主题发布信息
  if(mqttClient.publish(humTopic, humMsg)){
    Serial.println("Publish Topic: " + humTopicString);
    Serial.println("Humidity message: " + humMessageString);
  } else {
    Serial.println("Humidity message publish failed!"); 
  }
}

/******************************************************
   函 数 名: controlLedState
   描   述: 处理控制LED状态的命令
   参   数: msg 订阅的控制LED状态主题的消息
   返 回 值: 无
 ******************************************************/
void controlLedState(String msg) {
  //如果消息为on，则打开led
  if (msg == "on") {
    digitalWrite(LED_BUILTIN, LOW);
  }
  //如果消息为off，则关闭led
  if (msg == "off") {                           
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

/******************************************************
   函 数 名: controlLedBrightness
   描   述: 处理调节LED亮度的命令
   参   数: msg 订阅的调节LED亮度主题的消息
   返 回 值: 无
 ******************************************************/
void controlLedBrightness(String msg) {
  //将其转换成整形
  int LedBrightness = msg.toInt();
  //数值范围映射(求数值为0-100，转为0-1023)
  LedBrightness = 1023 - map(LedBrightness, 0, 100, 0, 1023);
  //映射后(写入管脚的电平数值(对应不同pwm占空比))
  analogWrite(LED_BUILTIN, LedBrightness);
}
