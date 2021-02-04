#include <ESP8266WiFi.h>                   //ESP8266WiFi库
#include <ESP8266WiFiMulti.h>              //ESP8266WiFiMulti库 
#include <ESP8266WebServer.h>              //ESP8266WebServer 库
#include <FS.h>                            //FS库
#include <Servo.h>                         //Servo库

Servo myservo;                             //舵机实例对象
ESP8266WebServer esp8266_server(80);       //建立esp8266网站服务器对象，端口号80
File fsUploadFile;                         //建立文件对象用于闪存文件上传


void setup() {
  //设置串口波特率
  Serial.begin(9600);
  //连接WiFi
  connectWifi();
  //启动闪存文件系统
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Started");
  } else {
    Serial.println("SPIFFS Failed to Start");
  }

  //设置开发板内置LED引脚为输出模式
  pinMode(LED_BUILTIN, OUTPUT);
  //舵机信号线连接在GPIO2
  myservo.attach(2);

  //初始化网络服务器
  handleRequest();
  //启动网站服务
  esp8266_server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //处理客户端请求
  esp8266_server.handleClient();
}

/******************************************************
   函 数 名: connectWifi
   描   述: 用于连接网络
   参   数: 无
   返 回 值: 无
 ******************************************************/
void connectWifi() {
  //ESP8266WiFiMulti对象
  ESP8266WiFiMulti wifiMulti;
  //通过addAp函数存储  WiFi名称  WiFi密码
  wifiMulti.addAP("********", "********");
  wifiMulti.addAP("********", "********");
  wifiMulti.addAP("********", "********");
  //正在连接
  Serial.println("Connecting to WiFi");
  //连接在当前环境中addAP函数所存储的中连接信号最强的那一个WiFi
  //连接成功后，wifiMulti.run()将会返回“WL_CONNECTED”
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Successfully!");
  //显示连接成功的WiFi信息
  //WiFi名称
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  //IP地址
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

/******************************************************
   函 数 名: handleRequest
   描   述: 用于处理浏览器的请求
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleRequest() {
  //处理浏览器对灯开关控制的请求
  esp8266_server.on("/changeLedState", handleChangeLedState);
  //处理浏览器对灯亮度调节的请求
  esp8266_server.on("/changeLedBrightness", handleChangeLedBrightness);
  //处理浏览器对灯状态选择的请求
  esp8266_server.on("/selectLedMode", handleSelectLedMode);
  //处理浏览器信息交互的请求
  esp8266_server.on("/interactInfo", handleInteractInfo);
  //处理浏览器获取模拟引脚读数的请求
  esp8266_server.on("/getAnalogPinValue", handleGetAnalogPinValue);
  //处理浏览器上传文件的请求
  esp8266_server.on("/uploadFile",      // 如果客户端通过upload页面
                    HTTP_POST,          // 向服务器发送文件(请求方法POST)
                    respondOK,          // 则回复状态码 200 给客户端
                    handleFileUpload);  // 并且运行处理文件上传函数
  //处理浏览器获取可供下载文件信息的请求
  esp8266_server.on("/loadFiles", handleLoadFiles);
  //处理浏览器下载文件的请求
  esp8266_server.on("/downloadFile", handleDownloadFile);
  //处理浏览器控制舵机的请求
  esp8266_server.on("/changeServoAngle", handleChangeServoAngle);
  //处理其他网络请求
  esp8266_server.onNotFound(handleUserRequest);
}

/******************************************************
   函 数 名: handleChangeLedState
   描   述: 用于向客户端发送200响应信息(成功)
   参   数: 无
   返 回 值: 无
 ******************************************************/
void respondOK() {
  esp8266_server.send(200, "text/plain", "Success");
}

/******************************************************
   函 数 名: handleChangeLedState
   描   述: 用于控制LED灯的亮灭
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleChangeLedState() {
  //取出请求URL的LedState字段
  String LedstateStr = esp8266_server.arg("LedState");
  Serial.print("LedState = ");
  Serial.println(LedstateStr);
  //如果是true，则开灯
  if (LedstateStr == "true") {
    analogWrite(LED_BUILTIN, 0);
  }
  //如果是false，则关灯
  if (LedstateStr == "false") {
    analogWrite(LED_BUILTIN, 1023);
  }
  //向客户端发送200响应信息(成功)
  respondOK();
}

/******************************************************
   函 数 名: handleChangeLedBrightness
   描   述: 用于控制LED灯的亮度
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleChangeLedBrightness() {
  //取出请求URL的LedBrightness字段
  String LedBrightnessStr = esp8266_server.arg("LedBrightness");
  //映射前(也即原始的数值)
  Serial.print("LedBrightnessStr = ");
  Serial.println(LedBrightnessStr);
  //将其转换成整形
  int LedBrightness = LedBrightnessStr.toInt();
  //数值范围映射(求数值为0-100，转为0-1023)
  LedBrightness = 1023 - map(LedBrightness, 0, 100, 0, 1023);
  //映射后(也即写入管脚的电平数值(对应不同pwm占空比))
  Serial.print("LedBrightness = ");
  Serial.println(LedBrightness);
  //写入电平
  analogWrite(LED_BUILTIN, LedBrightness);
  //向客户端发送200响应信息(成功)
  respondOK();
}

/******************************************************
   函 数 名: handleSelectLedMode
   描   述: 用于控制LED灯的状态
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleSelectLedMode() {
  //取出请求URL的LedMode字段
  String LedModeStr = esp8266_server.arg("LedMode");
  //LED灯的模式
  Serial.print("LedMode = ");
  Serial.println(LedModeStr);
  //呼吸灯模式
  if (LedModeStr == "1") {
    //循环5次
    for (int i = 0; i < 5; i++) {
      for (int value = 0 ; value <= 1023; value += 5) {
        analogWrite(LED_BUILTIN, value);
        delay(5);
      }
      for (int value = 1023; value >= 0; value -= 5) {
        analogWrite(LED_BUILTIN, value);
        delay(5);
      }
    }
  }
  //向客户端发送200响应信息(成功)
  respondOK();
}

/******************************************************
   函 数 名: handleInteractInfo
   描   述: 用于与客户端交互(一般来说，服务器是用来响应客户端请求的
   ，服务器不主动和客户端交流)
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleInteractInfo() {
  //取出请求URL的InteractInfo字段
  String InteractInfoStr = esp8266_server.arg("InteractInfo");
  //浏览器发送的信息
  Serial.print("InteractInfo = ");
  Serial.println(InteractInfoStr);

  //回复给浏览器的信息
  String resInfo = "Hello from server!";
  if (InteractInfoStr == "闪存信息") {
    //闪存信息对象
    FSInfo fs_info;
    //获取闪存文件信息                         
    SPIFFS.info(fs_info);
    resInfo = "全部空间: " + String(fs_info.totalBytes) + " Bytes\n已用空间: " + String(fs_info.usedBytes) + " Bytes";
  }
  if (InteractInfoStr == "服务器IP") {
    IPAddress ipAddress = WiFi.localIP();
    resInfo = String(ipAddress[0]) + String(".") + \
              String(ipAddress[1]) + String(".") + \
              String(ipAddress[2]) + String(".") + \
              String(ipAddress[3]);
  }
  if (InteractInfoStr == "项目地址") {
    resInfo = "https://github.com/SingleMoonlight/NodeMCU-projects";
  }
  //向客户端发送200响应信息(成功)，附带回复信息
  esp8266_server.send(200, "text/plain", resInfo);
}

/******************************************************
   函 数 名: handleGetAnalogPinValue
   描   述: 用于向客户端发送A0引脚的读数
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleGetAnalogPinValue() {
  //读取A0引脚读数
  int a = analogRead(A0);
  //转换为字符串型
  String adcValue = String(a);
  //向客户端发送200响应信息(成功)，附带读数
  esp8266_server.send(200, "text/plain", adcValue);
}

/******************************************************
   函 数 名: handleFileUpload
   描   述: 处理上传文件函数
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleFileUpload() {
  //处理文件上传
  HTTPUpload& upload = esp8266_server.upload();
  //如果上传状态为UPLOAD_FILE_START
  if (upload.status == UPLOAD_FILE_START) {
    //建立字符串变量用于存放上传的文件名
    String filename = upload.filename;
    //为上传文件名前加上"/updownload/"，即将文件放在updownload目录下
    if (!filename.startsWith("/")) {
      filename = "/updownload/" + filename;
    }
    //打印上传文件的名称
    Serial.println("File Name: " + filename);
    //在SPIFFS中建立文件用于写入用户上传的文件数据
    fsUploadFile = SPIFFS.open(filename, "w");
  }
  //如果上传状态为UPLOAD_FILE_WRITE
  else if (upload.status == UPLOAD_FILE_WRITE) {
    //文件对象建立成功
    if (fsUploadFile) {
      //向SPIFFS文件写入浏览器发来的文件数据
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  //如果上传状态为UPLOAD_FILE_END
  else if (upload.status == UPLOAD_FILE_END) {
    //如果文件成功建立
    if (fsUploadFile) {
      //将文件关闭
      fsUploadFile.close();
      Serial.println("Upload successfully!");
    }
    //如果文件未能成功建立
    else {
      //向浏览器发送相应代码500(服务器错误)
      esp8266_server.send(500, "text/plain");
    }
  }
}

/******************************************************
   函 数 名: handleLoadFiles
   描   述: 用于获取可供下载文件信息
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleLoadFiles() {
  //建立/updownload目录对象
  Dir dir = SPIFFS.openDir("/updownload");
  //定义一个Json数组，存储目录文件信息
  String fileJson = "[";
  //将文件信息添加到Json数组中
  while (dir.next()) {
    fileJson += "{\"filename\":";
    fileJson += "\"" + dir.fileName() + "\"";
    fileJson += "},";
  }
  fileJson = fileJson.substring(0, fileJson.length() - 1);
  fileJson += "]";
  Serial.print("FileInfo = ");
  Serial.println(fileJson);
  //向客户端发送200响应信息(成功)，附带Json文件信息
  esp8266_server.send(200, "text/plain", fileJson);
}

/******************************************************
   函 数 名: handleDownloadFile
   描   述: 用于获取可供下载文件信息
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleDownloadFile() {
  //取出请求URL的SelectFile字段
  String SelectFileStr = esp8266_server.arg("SelectFile");
  //下载的文件名
  Serial.print("SelectFile = ");
  Serial.println(SelectFileStr);

  //获取文件类型
  String contentType = getContentType(SelectFileStr);

  //找到文件
  if (SPIFFS.exists(SelectFileStr)) {
    //只读打开
    File file = SPIFFS.open(SelectFileStr, "r");
    //以流的形式传回浏览器
    esp8266_server.streamFile(file, contentType);
    //关闭文件
    file.close();
    //向客户端发送200响应信息(成功)
    respondOK();
  }
  //没有找到文件
  else{
    esp8266_server.send(404, "text/plain", "404 Not Found");  
  }
}

/******************************************************
   函 数 名: handleChangeServoAngle
   描   述: 用于控制舵机
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleChangeServoAngle() {
  //取出请求URL的ServoAngle字段
  String ServoAngleStr = esp8266_server.arg("ServoAngle");
  //舵机的角度
  Serial.print("ServoAngle = ");
  Serial.println(ServoAngleStr);
  //将其转换成整形
  int ServoAngle = ServoAngleStr.toInt();
  //转到指定角度
  myservo.write(ServoAngle);
  //向客户端发送200响应信息(成功)
  respondOK();
}

/******************************************************
   函 数 名: handleUserRequest
   描   述: 用于处理浏览器的其他请求
   参   数: 无
   返 回 值: 无
 ******************************************************/
void handleUserRequest() {
  //用户请求资源
  String reqResource = esp8266_server.uri();
  Serial.print("reqResource: ");
  Serial.println(reqResource);

  //处理用户请求资源
  bool fileReadOK = handleFileRead(reqResource);

  //如果在SPIFFS无法找到用户访问的资源，则回复404(错误)
  if (!fileReadOK) {
    esp8266_server.send(404, "text/plain", "404 Not Found");
  }
}

/******************************************************
   函 数 名: handleFileRead
   描   述: 用于处理浏览器HTTP访问
   参   数: 请求资源uri
   返 回 值: true 资源存在 false 资源不存在
 ******************************************************/
bool handleFileRead(String resource) {
  //如果访问地址以"/"为结尾
  if (resource.endsWith("/")) {
    //认为访问首页
    resource = "/index.html";
  }
  //获取文件类型
  String contentType = getContentType(resource);

  //找到文件
  if (SPIFFS.exists(resource)) {
    //只读打开
    File file = SPIFFS.open(resource, "r");
    //以流的形式传回浏览器
    esp8266_server.streamFile(file, contentType);
    //关闭文件
    file.close();
    return true;
  }
  return false;
}

/******************************************************
   函 数 名: getContentType
   描   述: 获取文件类型(MIME)
   参   数: 文件名
   返 回 值: 文件类型
 ******************************************************/
String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
