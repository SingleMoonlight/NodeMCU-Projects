#  NodeMCU-projects

NodeMCU是一个开源的物联网平台，拥有开源，交互式，可编程，低成本，简单，智能，WI-FI硬件等特点，可以在Arduino IDE中进行编程开发，对新手十分友好。这是本人使用NodeMCU开发的一些小项目，**仅为自己一边学习一边总结记录**。

+ [weather-clock](#weather-clock)
+ [LAN_microserver](#LAN_microserver)
+ [WAN_microcontroller](#WAN_microcontroller)
+ [loading](#loading)
+ [参考资料](#参考资料)


##  weather-clock

###  概述

首先使NodeMCU模块连接WiFi，接入互联网；然后连接阿里的NTP服务器，获取网络授时，向心知天气服务器发送HTTP请求，对返回的天气信息进行解析，获得今天、明天、后天的天气信息；最后通过OLED屏幕显示获得的信息。

###  硬件

+ NodeMCU(CH340)
+ 0.96英寸I^2^C接口OLED显示屏(SSD1306)

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/hardware.png" width = "387" height = "258" alt = "hardware" /></div>

###  软件

+ Arduino IDE
+ PCtoLCD2002 取模软件

###  设计

####  连接WiFi

NodeMCU连接WiFi方式有很多种：

1. 使用ESP8266WiFi库，在程序中写入一个WiFi信息，并连接到该WiFi。

2. 使用ESP8266WiFiMulti库，在程序中写入多个WiFi信息，并自动连接到信号最强的WiFi(本项目使用该方式)。

3. 使用WiFiManager库，不需要在程序中写入WiFi信息。使用WiFiManager库，当NodeMCU上电时，会在闪存中读取曾经连接并被保存的WiFi信息，然后连接到WiFi。如果连接失败，NodeMCU会创建一个热点，用手机或电脑连接该热点，然后扫描附近的WiFi，选择WiFi后输入密码进行连接(类似于路由器的配置)。

####  实时时间获取

#####  NTP服务

NTP(Network Time Protocol)，网络时间协议，是用来使计算机时间同步化的一种协议。

#####  NTPClient库

> **An NTPClient to connect to a time sever** Get time from a NTP sever and keep it in sync.

使用NTPClient库连接NTP服务器获取网络时间并保持时间同步。

实例化NTPClient对象，选择服务器(阿里)、偏移量(时区)及更新间隔(单位为毫秒)

```c++
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
```

获取时间

```c++
timeClient.update();
nowTime = timeClient.getFormattedTime();
```

####  天气信息获取

#####  心知天气API

心知天气通过标准的RestfulAPI接口，提供标准化的数据访问。拥有许多丰富的功能，本项目仅使用了天气类中逐日天气预报和昨日天气这一个子类，由于是普通用户，该接口只能只返回 3 天天气预报。

请求地址示例

> 北京今天和未来 4 天的预报 https://api.seniverse.com/v3/weather/daily.json?key=your_api_key&location=beijing&language=zh-Hans&unit=c&start=0&days=5
>
> 北京的昨日天气、今天和未来 3 天的预报（昨日天气独立于逐日天气预报，需要单独购买） https://api.seniverse.com/v3/weather/daily.json?key=your_api_key&location=beijing&language=zh-Hans&unit=c&start=-1&days=5

#####  请求天气数据

按照心知天气官网介绍的API使用方式，建立HTTP请求信息。

```c++
//WiFiClient对象
WiFiClient client;
//建立心知天气API天气请求资源地址
String reqRes = "/v3/weather/daily.json?key=" + privateKey +
                + "&location=" + requireCity + "&language=en&unit=" +
                temperatureUnit + "&start=0&days=3";
//建立http请求信息
String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n";
```

连接服务器，发送HTTP请求。

```c++
if (client.connect(host, 80)) {
    client.print(httpRequest);
    ......
  }
```

####  Json数据解析

#####  ArduinoJson库

> A simple and efficient JSON library for embedded C++. ArduinoJson supports  seriaization, deserialization, MessagePack, fixed allcation, zero-copy, streams, fitering, and more.

ArduinoJson库是解析JSON格式信息的第三方库，可以很简单地解析json数据(反序列化)和构造序列化json(序列化)。在ArduinoJson官网里还提供了在线生成程序的工具[ArduinoJson Assistant](https://arduinojson.org/v6/assistant/)，十分便捷。

#####  天气信息解析

利用ArduinoJson官网的在线工具[ArduinoJson Assistant](https://arduinojson.org/v6/assistant/)，首先选择处理器、模式和输入类型，接着输入心知天气服务器返回的json数据，然后选择DynamicJsonDocument对象所占的空间(按照推荐即可)，最后点击生成程序。

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/arduinojson.png" width = "703" height = "286" alt = "arduinojson" /></div>

生成的程序修改参数后选择需要的信息，可以直接使用(不同的ArduinoJson版本生成的程序会有些许不同)。

####  多任务实现

##### Ticker库

> The **Arduino Ticker Library** allows you to create easily Ticker callbacks, which can call a function in a predetermined interval. You can change the number of repeats of the callbacks, if repeats is 0 the ticker runs in endless mode. Works like a "thread", where a secondary function will run when necessary. The library use no interupts of the hardware timers and works with the **micros() / millis()** function. You are not (really) limited in the number of Tickers.

正常情况esp8266执行任务时是自上而下的，最终在 **loop()** 函数中无限循环。为了实现向服务器发送请求、显示时间、显示天气等任务并行进行，可以借助Ticker库，让esp8266间隔固定时间调用任务函数，这里的定时调用不同于delay延时，它不受程序运行的的影响也不对程序的运行造成影响，达到了类似多线程(thread)的效果。

#####  attach函数

attach函数有两个参数，第一个参数是调用函数的时间间隔，单位是秒，第二个参数是被调用的函数，需要注意的是被调用的函数必须执行地很快。

```c++
//向服务器请求定时器(实例化Ticker对象)
Ticker requestTicker;
//向服务器请求时间间隔
int requestInterval = 0;
```

```c++
//开启服务器请求定时
requestTicker.attach(1, tickerRequestInterval);
```

这里对变量 **requestInterval** 进行加1操作，在 **loop()** 函数里对变量 **requestInterval** 进行判断，当其达到预定值后就去执行任务函数，这里是执行 **getWeather()** 函数。

```c++
void tickerRequestInterval() {
  requestInterval++;
}
```

最后将变量 **requestInterval** 重新置为0，程序继续循环。

```c++
//每隔60s向服务器发起一次请求
if (requestInterval >= 60) {
  //获取天气数据
  getWeather();
  //重新计时
  requestInterval = 0;
}
```

####  显示

#####  U8g2库

> U8g2 is a monochrome graphics library for embedded devices. U8g2 supports monochrome OLEDs and LCDs, which include the following controllers: SSD1305, SSD1306, SSD1309, SSD1322, SSD1325, SSD1327, SSD1329, SSD1606, SSD1607, SH1106, SH1107, SH1108, SH1122, T6963, RA8835, LC7981, PCD8544, PCF8812, HX1230, UC1601, UC1604, UC1608, UC1610, UC1611, UC1701, ST7565, ST7567, ST7588, ST75256, NT7534, IST3020, ST7920, LD7032, KS0108, SED1520, SBN1661, IL3820, MAX7219 (see [here](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp) for a full list).

U8g2是一个用于嵌入式设备的单色图形库，支持许多单色OLED和LCD显示控制器，如SSD1306。U8g2库集成了大多数显示屏的底层驱动，拥有许多字库，而且支持中文，API众多，功能十分强大。

#####  函数的使用

关于U8g2库相关函数的使用，网上有很多详细的资料，U8g2库也提供了丰富的示例程序(运行时需要取消构造函数的注释)，可供研究使用。本项目里绘制所使用的比较重要的函数有：

1. u8g2.drawStr()

绘制字符串。该函数有三个参数，前两个参数是待绘制的字符串首字符的坐标，第三个参数是待绘制的字符串。

```c++
u8g2.drawStr(0, 0, "Connecting to WiFi");
```

2. u8g2.print()

在入指定光标位置打印内容，支持变量。该函数只有一个参数，就是需要打印的内容。

```c++
//设置光标位置  
u8g2.setCursor(65, 32);
//打印最低温度到最高温度
u8g2.print("T: " + dayWeather.low + "~" + dayWeather.high + "°C");
```

3. u8g2.drawXBMP()

绘制FLASH中位图图像。该函数有五个变量，前两个参数是待绘制的位图图像的坐标，第三、四个参数是位图的宽高，最后一个参数是位图资源。

```c++
//晴天图标
u8g2.drawXBMP(0, 0, 64, 64, Sunny);
```

#####  图片取模

借助PS和画图等软件，制作好图标的单色位图(bmp格式)图片，将图片导入到PCtoLCD2002中，在字模选项中调整设置，然后生成字模，生成的字模可以直接复制到程序中使用。

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/pctolcd2002.png" width = "359" height = "215" alt = "pctolcd2002" /></div>

###  展示

####  启动界面

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/start.png" width = "230" height = "147" alt = "start" /></div>

启动界面的汉字是图片取模制作成的，不是直接使用函数打印的。

####  连接WiFi界面

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/connectwifi.png" width = "432" height = "181" alt = "connectwifi" /></div>

WiFi连接过程中 **“.”** 会逐个增加，以提示连接的过程，连接成功后会打印 **“Successfully!”** 提示连接成功。然后到下一个界面显示连接到的WiFi名称以及分配的IP地址。

####  时间日期界面

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/date.png" width = "243" height = "162" alt = "date" /></div>

这里周几的计算用到了[蔡勒公式](https://zh.wikipedia.org/wiki/%E8%94%A1%E5%8B%92%E5%85%AC%E5%BC%8F)。从心知天气解析的Json数据里可以得到当天的日期，将这个日期的年、月、日取出，应用蔡勒公式即可解出当天为周几。时间日期界面显示10s后变为天气界面。

####  天气界面

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/weather_clock/illus/weather.png" width = "525" height = "184" alt = "weather" /></div>

今天、明天、后天的天气信息轮流显示3s，显示信息包括天气图标，日期，天气文字、温度和湿度。

## LAN_microserver

### 概述

这是一个运行在NodeMCU上的微服务器，它工作在局域网(LAN)中。NodeMCU模块连接WiFi后，启动网站服务，响应局域网内其他客户端(浏览器)的请求。

不同于传统意义上专于提供计算和应用服务、执行大量任务的服务器，它的算力十分有限，不过它仍然可以执行一些简单的任务。比如展示丰富多彩的Web网页，进行文件的上传和下载管理。除此之外，还可以利用局域网进行TCP通信，在网页上向NodeMCU发送控制指令(HTTP请求)，让NodeMCU 执行相应的任务，实现远程控制。

### 硬件

+ NodeMCU(CH340)
+ SG90舵机

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/LAN_microserver/illus/hardware.png" width = "523" height = "271" alt = "hardware" /></div>

### 软件

+ Arduino IDE
+ WebStorm 2019.3.3
+ ESP8266FS闪存文件上传插件

### 设计

####  前端

鉴于项目是以NodeMCU为主体的服务端开发，主要目的是学习使用NodeMCU，且前端的知识体系很庞杂，开发使用的框架与方法不尽相同，因此在这里这些内容不做过多的展开。

#####  框架

本项目的前端部分是基于Vue构建的，使用了Element的组件库。

1. Vue

> Vue.js（读音 /vjuː/, 类似于 view） 是一套构建用户界面的渐进式框架。
> Vue 只关注视图层， 采用自底向上增量开发的设计。
> Vue 的目标是通过尽可能简单的 API 实现响应的数据绑定和组合的视图组件。

2. Element

> Element，一套为开发者、设计师和产品经理准备的基于 Vue 2.0 的桌面端组件库。

##### Web界面

1. 首页

程序成功运行起来后，在浏览器地址栏输入NodeMCU的IP地址即可进入首页。首页简要介绍了项目的大致情况，点击按钮可进入控制面板页面。

2. 控制面板

控制面板包含了项目的各个功能模块：板载LED控制、信息交互、文件管理、舵机控制、模拟引脚读数。

#####  请求发送

请求的发送是本项目**前端部分的核心**，前端请求的发送使用了Axios，相当于是Vue中的ajxs，提供给用户异步获取调用接口的组件库。

> Axios，基于 Promise 的 HTTP 客户端，可以工作于浏览器中，也可以在 node.js 中使用。

项目使用get请求改变LED状态的实例。

```javascript
//发送请求
this.$axios.get('/changeLedState',
	{
	    params: {
		//参数
                LedState: this.LedState
	}
    })
    .then(response => {
    	//处理请求成功
    	//如果响应数据为Success
	if (response.data === "Success") {
		this.$message({
			showClose: false,
                	message: '操作成功！',
                	type: 'success',
                	center: true
           	});
        } else {
		this.$message({
			showClose: false,
			message: '操作失败！',
			type: 'error',
			center: true
		});
	}
    })
    .catch(err => {
    	//处理请求失败
        console.log(err);
    });
```

项目使用post请求上传文件的实例。

```javascript
//文件对象
let fileObj = param.file;
//FormData对象
let fd = new FormData();
//添加数据(名称/值对) 
fd.append('UploadFile', fileObj);
//URL
let url = '/uploadFile';
let config = {
		headers: {
            //表单数据类型
    		'Content-Type': 'multipart/form-data'
        	}
        };
//发送请求
this.$axios.post(url, fd, config)
    .then(response=>{
	//处理请求成功
    	//如果响应数据为Success
    	if(response.data === "Success"){
        	//提示上传成功
		this.$message({
			showClose: false,
			message: '上传成功！',
			type: 'success',
    			center: true
		});
	}
    	//否则提示上传失败
    	else {
		this.$message({
			showClose: false,
			message: '上传失败！',
			type: 'error',
			center: true
                });
        }
    })
    .catch(err=>{
    	//处理请求失败
	console.log(err);
	this.$message({
		showClose: false,
		message: '网络错误！',
		type: 'error',
		center: true
	});
    });
```

####  前后端数据交互

前后端数据交互事实上是一个很复杂的问题，牵扯到很多计算机网络方面的知识，不过在开发的过程中底层的内容大多都没有深究，这里也只做一些简单的说明。

#####  数据交互原理

无论前端基于什么框架，做Web开发，前后端的数据交互都是很重要的一个环节，这个过程是通过HTTP请求的方式实现的。HTTP消息有两种类型： 请求(requests)——由客户端发送用来触发一个服务器上的动作；响应(responses)——来自服务器的应答。前端发送请求url给服务器，服务器响应前端的请求，请求和响应的过程都可以携带信息，以此来实现前后端的数据交互。

#####  HTTP请求

> 当浏览器向Web服务器发出请求时，它向服务器传递了一个数据块，也就是请求信息，它是由客户端发出的消息，用来使服务器执行动作，它包括起始行、请求头以及请求体。

起始行：起始行包含请求方法、请求地址URL和HTTP协议版本三个部分。

Headers：请求头为请求报文添加了一些附加信息。

Body：请求正文中可以包含客户端提交的信息。

#####  HTTP响应

> HTTP响应代表服务器向客户端回送的数据，它包括一个状态行、响应头以及响应体。 

状态行：HTTP 响应的起始行被称作状态行，包含协议版本、状态码和状态文本三个部分。

Headers：响应头用于描述服务器以及响应数据的基本信息。

Body：响应的数据，用于存放需要返回给客户端的数据信息。

#####  请求方法

| 请求方法 | 含义                                                         |
| -------- | ------------------------------------------------------------ |
| GET      | 请求指定的页面信息，并返回实体主体。                         |
| HEAD     | 类似于 GET 请求，只不过返回的响应中没有具体的内容，用于获取报头 |
| POST     | 向指定资源提交数据进行处理请求（例如提交表单或者上传文件）。数据被包含在请求体中。POST 请求可能会导致新的资源的建立和/或已有资源的修改。 |
| PUT      | 从客户端向服务器传送的数据取代指定的文档的内容。             |
| DELETE   | 请求服务器删除指定的页面。                                   |
| CONNECT  | HTTP/1.1 协议中预留给能够将连接改为管道方式的代理服务器。    |
| OPTIONS  | 允许客户端查看服务器的性能。                                 |
| TRACE    | 回显服务器收到的请求，主要用于测试或诊断。                   |
| PATCH    | 是对 PUT 方法的补充，用来对已知资源进行局部更新 。           |

HTTP请求方法有很多种，这里只介绍一下项目中使用到的也是最常用的两种请求方法：

1. GET方法

GET方法是默认的HTTP请求方法，GET方法要求服务器将URL定位的资源放在响应报文的数据部分，发送给客户端。使用GET方法时，请求参数和对应的值附加在URL后面向Web服务器发送，但传递参数长度受限制，而且使用GET方法来提交表单数据存在着安全隐患。

2. POST方法

POST方法主要是向Web服务器提交表单数据，尤其信息比较多的时候。POST方法将请求参数封装在HTTP请求数据中，对传输的数据大小没有限制。通过POST方法提交表单数据时，数据不是作为URL请求的一部分而是作为标准数据传送给Web服务器，这就克服了GET方法中的信息无法保密和数据量太小的缺点。

#####  状态码

| 分类 | 描述                                           |
| ---- | ---------------------------------------------- |
| 1**  | 信息，服务器收到请求，需要请求者继续执行操作   |
| 2**  | 成功，操作被成功接收并处理                     |
| 3**  | 重定向，需要进一步的操作以完成请求             |
| 4**  | 客户端错误，请求包含语法错误或无法完成请求     |
| 5**  | 服务器错误，服务器在处理请求的过程中发生了错误 |

HTTP状态码由三个十进制数字组成，第一个十进制数字定义了状态码的类型，后两个数字没有分类的作用。HTTP状态码共分为5种类型，本项目中使用到的也是常用的HTTP状态码：

1. 200——请求成功

2. 404——请求的资源(网页等)不存在
3. 500——服务器错误

#####  URL

> 在WWW上，每一信息资源都有统一的且在网上唯一的地址，该地址就叫*URL*（Uniform Resource Locator，统一资源定位器），它是WWW的统一资源定位标志，就是指网络地址。

组成：**<协议>://<主机>:<端口>/<路径>**，端口和路径有时可以省略，HTTP默认端口号为80。

参数：以 **"?"** 开始，格式为 **"name=value"** ，如果存在多个参数，则参数之间用 **"&"** 隔开，后端在收到前端的请求时，可以解析出URL参数。

项目中控制板载LED亮的请求URL，其中LedState=true就是参数。

```c++
http://192.168.0.110/changeLedState?LedState=true
```

####  后端

##### 网络服务器

收到前端的请求URL，后端服务器需要做出正确的响应。以NodeMCU作为服务器，这里用到一个很重要的库—— **ESP8266WebServer库** 。

> ESP8266WebServer库用于HTTP协议通讯。通过ESP8266WebServer库，可以使用ESP8266开发板建立网络服务器，从而允许其它网络设备通过HTTP协议访问并实现信息交流。

ESP8266WebServer库有很多函数，下面是项目中使用到的部分库函数：

1. ESP8266WebServer

建立ESP8266WebServer对象，在程序中利用此对象来调用其他函数，实现网络服务器的功能。

```c++
//建立esp8266网站服务器对象，端口号80
ESP8266WebServer esp8266_server(80);
```

2. begin

启动NodeMCU开发板所建立的网络服务器。

```c++
//启动网站服务
esp8266_server.begin();
```

3. handleClient

检查有没有客户端设备通过网络向服务器发送HTTP请求，起到监听的作用，建议将该函数放在loop函数中。

```c++
//处理客户端请求
esp8266_server.handleClient();
```

4. on

客户端向服务器发送HTTP请求时，利用on函数来设置HTTP请求的回调函数。

```c++
//处理浏览器对灯开关控制的请求
esp8266_server.on("/changeLedState", handleChangeLedState);
```

5. onNotFound

客户端向服务器发送HTTP请求时，利用onNotFound函数来设置HTTP请求无效地址的回调函数。

```c++
//处理其他网络请求
esp8266_server.onNotFound(handleUserRequest);
```

6. send

户端向服务器发送HTTP请求时，服务器可使用该函数向客户端发送响应信息，包括响应状态码、相应内容类型(可选参数)、响应内容(可选参数)。

```c++
esp8266_server.send(200, "text/plain", "Success");
```

7. arg

获取客户端向服务器发送请求URL中的指定参数的数值。

```c++
//取出请求URL的LedState字段
String LedstateStr = esp8266_server.arg("LedState");
```

8. upload

用于NodeMCU开发板所建立的服务器处理客户端的文件上传请求。

```c++
//处理文件上传
HTTPUpload& upload = esp8266_server.upload();
```

9. uri

获取客户端向服务器请求的资源路径。

```c++
//用户请求资源
String reqResource = esp8266_server.uri();
```


#####  请求处理

实现不同的功能，客户端会发送不同的HTTP请求，这些请求的URL不同。利用on函数，针对不同的URL编写每个功能的回调函数，这是本项目**后端部分的核心**。为了使代码整洁且便于管理，将所有请求放在handleRequest函数里，然后在setup函数里调用handleRequest函数。

```c++
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
```

例如当客户端发出对LED灯开关控制的请求(/changeLedState)时，程序会执行handleChangeLedState函数，只需要在这个函数里编写对灯开关的控制逻辑代码即可实现远程控制。

```c++
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
```

#####  文件管理

NodeMCU有一个4M的闪存(SPIFFS)，用于存放系统文件、程序代码和一些其他的数据，将打包好的前端网页文件上传至闪存里，开启网络服务器后通过网络请求的方式读取网页文件，就形成了真正意义上的WebServer。

既然是要做一个服务器，那么文件管理的功能就必不可少了，这里包括了文件的上传和下载。NodeMCU的闪存空间很小，为了便于管理，将文件上传和下载使用的文件集中放置在/updownload目录下。

文件上传有下面三个步骤：

1. 在SPIFFS中建立文件

```c++
fsUploadFile = SPIFFS.open(filename, "w");
```

2. 向SPIFFS文件写入浏览器发来的文件数据

```c++
fsUploadFile.write(upload.buf, upload.currentSize);
```

3. 将文件关闭

```c++
fsUploadFile.close();
```

文件的下载有以下两个过程：

1. 客户端要选择可供下载的文件。在这一过程中，服务器将/updownload目录下所有文件的文件名以Json格式发送给客户端，客户端解析后变成一个个可供选择的条目。

2. 服务器将客户端选择的文件数据发送给客户端。在这一过程中，服务器获取客户端选择的文件名。然后在闪存中查找该文件，将该文件以流的形式传回浏览器。

```c++
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
```

### 展示

#### 网站首页

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/LAN_microserver/illus/index.png" alt = "index" /></div>

#### 控制面板

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/LAN_microserver/illus/control.png" alt = "control" /></div>

#### 后台信息

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/LAN_microserver/illus/backend_info.png" alt = "backend_info" /></div>

#### 视频展示

[视频](https://github.com/SingleMoonlight/NodeMCU-projects/tree/main/LAN_microserver/Video)

## WAN_microcontroller

### 概述

NodeMCU模块连接WiFi后，作为一个客户端连接MQTT服务器；手机通过微信小程序连接到相同的服务器上，作为另一个客户端。手机和NodeMCU不需要处于同一局域网下，两者之间的通信不受时间和空间的限制。NodeMCU读取连接的温湿度传感器的信息，手机客户端订阅该信息，可以实现远程监控；NodeMCU订阅手机发布的命令主题，可以实现远程控制。

### 硬件

+ NodeMCU(CH340)
+ DHT11

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/hardware.png" width = "480" height = "320" alt = "hardware" /></div>

### 软件

+ Arduino IDE
+ 微信开发者工具
+ MQTT.fx

### 设计

####  MQTT服务器

#####  MQTT概念

> 消息队列遥测传输（Message Queuing Telemetry Transport）是ISO 标准（ISO/IEC PRF 20922）下基于发布 (Publish)/订阅 (Subscribe)范式的消息协议，可视为“资料传递的桥梁”。它工作在 TCP/IP协议族上，是为硬件性能低下的远程设备以及网络状况糟糕的情况下而设计的发布/订阅型消息协议。

#####  MQTT基本原理

![MQTT: publish / subscribe architecture](https://mqtt.org/assets/img/mqtt-publish-subscribe.png)

在MQTT协议的通信过程中有两个角色，分别是服务器(Broker)和客户端(Client)。服务器的任务是将某一主题(Topic)的信息推送给所有订阅(Subscribe)了该主题的客户端；客户端既可以向服务器发布(Publish)信息，也可以从服务器获取信息。

用一个通俗的例子来讲，把B站看成MQTT服务器，up主和观众就是客户端。我们关注一个up主就是订阅一个主题，up更新一个视频就是发布一条信息，关注的up主更新后我们就会收到B站的推送，然后就可以在动态看见up主更新的内容。要注意的是，我们不会收到没有关注的up主的消息推送，我们可以关注多个up主，只要他们更新，就一定会收到通知。

#####  MQTT服务器选择

使用MQTT服务器的途径有两种，一种是自己搭建服务器(需要有公网IP)，一种是使用现有平台提供的MQTT服务器，如阿里云、华为云等，使用这些公用服务器需要注册账号、收费等。

当然仅为了学习和测试可以使用公用的、免费的MQTT服务器，本项目使用的MQTT服务器是 **Mosquitto** ，它支持基于Websocket的MQTT协议。

test.mosquitto.org

> This is test.mosquitto.org. It hosts a publicly available Eclipse Mosquitto MQTT server/broker. This server is provided as a service for the community to do testing, but it is also extremely useful for testing the server. This means that it will often be running unreleased or experimental code and may not be as stable as you might hope.

MQTT服务器地址：test.mosquitto.org
TCP 端口：1883
TCP/TLS 端口：8883
WebSockets 端口：8080
Websocket/TLS 端口：8081

####  NodeMCU客户端

#####  温湿度信息读取

DHT11模块

> DHT11数字温湿度传感器是一款含有已校准数字信号输出的温湿度复合传.感器。它应用专用的数字模块采集技术和温湿度传感技术，确保产品具有极高的可靠性与卓越的长期稳定性。

工作电压范围：3.3V-5.5V
工作电流：平均 0.5mA
输出：单总线数字信号
测量范围：湿度 20~90％RH，温度 0~50℃
精度：湿度±5%，温度±2℃
分辨率：湿度 1%，温度 1℃

dht11库

> DHT11 Temperature & Humidity Sensor library for Arduino.

DHT 系列模块有许多非常成熟的库，不同的库使用方法不尽相同。本项目中使用的是一个比较简单的、专门针对DHT11的库，温湿度信息的获取方法如下：

```c++
#include <dht11.h>

//建立dht11传感器对象
dht11 DHT11;
//读取传感器信息，传感器连接引脚(GPIO14 D5)
DHT11.read(14);
//温度信息
String tempMessageString = String(DHT11.temperature);
//湿度信息
String humMessageString = String(DHT11.humidity); 
```

#####  功能实现关键

PubSubClient库

> Arduino Client for MQTT. This library provides a client for doing simple publish/subscribe messaging with a server that supports MQTT.

库结构如下：

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/pubsubclient.png" alt = "pubsubclient" /></div>

各个函数的详细功能及使用方法不再展开描述，下面只说明项目中的具体功能实现。

#####  程序实现

1. 配置客户端连接信息

```c++
//MQTT服务器地址
const char* mqttServer = "test.mosquitto.org";
//建立WiFiClient对象
WiFiClient wifiClient;
//建立mqttClient对象
PubSubClient mqttClient(wifiClient);             

//设置MQTT服务器和端口号(这里使用的是普通的TCP端口)
mqttClient.setServer(mqttServer, 1883);
//设置收到信息后的回调函数
mqttClient.setCallback(receiveCallback);
```

2. 连接MQTT服务器

```c++
//设置客户端ID(尽量独一无二，避免重复)
String clientId = "NodeMCU-" + WiFi.macAddress();
//成功连接MQTT服务器
if (mqttClient.connect(clientId.c_str())) { 
  Serial.println("MQTT Server Connected");
} 
//连接失败
else {
  Serial.print("MQTT server connect failed!");
} 
```

3. 订阅主题

```c++
//订阅LED状态控制主题(尽量独一无二，以免收到使用同一服务器客户端的无用信息)
String ledStateTopicString = WiFi.macAddress() + "/control/ledState";
char ledStateTopic[ledStateTopicString.length() + 1];  
strcpy(ledStateTopic, ledStateTopicString.c_str());
//如果订阅成功，输出订阅信息
if(mqttClient.subscribe(ledStateTopic)){
  Serial.println("Subscrib Topic: " + ledStateTopicString);
} else {
  Serial.print("Subscribe fail!");
} 
```

4. 发布消息

```c++
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
```

5. 接收消息

```c++
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
```

####  微信小程序客户端

#####  功能实现关键

WebSocket

> WebSocket是一种网络传输协议，可在单个TCP连接上进行**全双工通信**，位于OSI模型的应用层。
>
> WebSocket使得客户端和服务器之间的数据交换变得更加简单，允许服务端主动向客户端推送数据。在WebSocket API中，浏览器和服务器只需要完成一次握手，两者之间就可以创建持久性的连接，并进行双向数据传输。

在服务器选择时介绍了选择 **Mosquitto** 作为服务器的原因，有很重要的一点是它支持基于WebSocket的MQTT协议，而浏览器中连接MQTT服务器正是通过WebSocket来实现的。如果让微信小程序来作为MQTT的客户端，接受NodeMCU的消息或者向NodeMCU发送信息，那么MQTT over WebSocket自然成了最合理的途径，这里使用了一个很有名的第三方库 **MQTT.js** 。

MQTT.js

> MQTT.js is a client library for the MQTT protocol, written in JavaScript for node.js and the browser.

MQTT.js 是 JavaScript 编写的，实现了 MQTT 协议客户端功能的模块，可以在浏览器 和 Node.js 环境中使用。由于 JavaScript 单线程特性，MQTT.js 是全异步 MQTT 客户端，MQTT.js 支持 MQTT 与 MQTT over WebSocket，在不同运行环境支持的度如下：

1. 浏览器环境：MQTT over WebSocket（包括微信小程序、支付宝小程序等定制浏览器环境）
2. Node.js 环境：MQTT、MQTT over WebSocket

不同环境里除了少部分连接参数不同，其他 API 均是相同的。将[mqtt.js文件](https://unpkg.com/mqtt@4.1.0/dist/mqtt.js)直接复制到小程序项目中就可以使用了，这里连接使用的版本是4.1.0，2.18.8版本在微信开发者工具模拟器中可以连接成功，但是在真机上无法连接，也有可能是本人测试手机(Android)的问题。

#####  程序实现

1. 配置客户端连接信息

在设置连接服务器地址时，根据不同的协议指定使用的连接方式：

ws：未加密 WebSocket 连接
wss：加密 WebSocket 连接
mqtt：未加密 TCP 连接
mqtts：加密 TCP 连接
wxs：微信小程序连接
alis：支付宝小程序连接

```javascript
//引用mqtt.js库
var mqtt = require('../../utils/mqtt');
//定义客户端对象
var client = null;

//服务器地址
const connectUrl = 'wxs://test.mosquitto.org'
//连接配置
const options = {
      clientId: '573b79e3bae7431b8f2e7fb50a0ed2ee',
      keepalive: 60,
      clean: true,
      port: 8081,	 //端口(这里使用的是Websocket端口)
      protocolVersion: 4 //MQTT v3.1.1
}
```

2. 连接MQTT服务器

```javascript
const client = mqtt.connect(connectUrl, options)

//网络错误
client.on('error', (error) => {
    ...
});
//重连
client.on('reconnect', (error) => {
    ...
});
//连接成功
client.on('connect', (e) => {
    ...
});
```

3. 订阅主题

```javascript
//this.data.subTopic为订阅的主题
client.subscribe(this.data.subTopic, {
	qos: 0 //服务质量等级
}, function(err) {
	if (!err) {
		wx.showToast({
			title: '订阅成功',
			mask: true,
			icon: 'success',
			duration: 1000
		});
	}else{
		wx.showToast({
			title: '订阅失败',
			mask: true,
			icon: 'none',
			duration: 1000
		});
	}
});
```

4. 发布消息

```javascript
//this.data.pubTopic为发布消息的主题，this.data.pubMsg为发布的消息
client.publish(this.data.pubTopic, this.data.pubMsg, function(err) {
	if (!err) {
		wx.showToast({
			title: '发布成功',
			mask: true,
			icon: 'success',
			duration: 1000
		});
	}else{
		wx.showToast({
			title: '发布失败',
			mask: true,
			icon: 'none',
			duration: 1000
		});
	}
});
```

5. 接收消息

```javascript
//topic为消息主题，messsage为消息
client.on('message', function (topic, message) {
    that.setData({
        //更新消息框的内容
        receiveInfo: that.data.receiveInfo + topic.toString() + ": " + message.toString() + "\n",
        //更新竖向滚动条位置到最新的消息(每条消息的高度设为20)
        scrollTop: that.data.scrollTop + 20
    });
});
```

####  MQTT.fx客户端

#####  MQTT.fx介绍

> MQTT.fx 是目前主流的 MQTT 桌面客户端，它支持 Windows、 Mac、Linux 操作系统，可以快速验证是否可与 IoT Cloud 进行连接，并发布或订阅消息。

在NodeMCU客户端和小程序开发过程中，可以使用MQTT.fx对两个客户端的功能进行测试，项目最终并未使用该客户端，其主要功能就是测试。

#####  MQTT.fx使用

1. 配置客户端

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/setclient.png" alt = "setclient" /></div>

2. 发布消息

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/pubbymqttfx.png" alt = "pubbymqttfx" /></div>

3. 订阅主题

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/subbymqttfx.png" alt = "subbymqttfx" /></div>

### 展示

####  NodeMCU串口输出

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/backend_info_1.png" alt = "backend_info_1" /></div>

连接WiFi后连接MQTT服务器，订阅LED状态和亮度控制主题，每隔10s向温湿度主题发布消息。

<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/backend_info_2.png" alt = "backend_info_2" /></div>

接受来自其他客户端对LED状态和亮度控制主题的消息。

####  微信小程序

#####  微信开发者工具模拟器界面


<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/testonwechatdevtools.png" width = "280" height = "500" alt = "testonwechatdevtools" /></div>

#####  真机界面


<div align = center><img src="https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/WAN_microcontroller/illus/testionphone.jpg" width = "280" height = "591" alt = "testionphone" /></div>

####  视频展示

[视频](https://github.com/SingleMoonlight/NodeMCU-projects/tree/main/WAN_microcontroller/Video)

## loading

继续学习更新......

## 参考资料

这些小项目虽然功能简单甚至简陋，但是制作的背后对我来说都有许多新的知识。发现问题、分析问题然后解决问题，在互联网上几乎可以找到任何自己想要的资料。除了这些网站以外，还参考了很多的博客、文章、B站视频等，内容很多不再一一列举，非常感谢这些内容背后的创作者，分享知识、共同进步。

### 工具类网站

> [ArduinoJson官网](https://arduinojson.org/)<br>
> [Element官网](https://element.eleme.cn/#/zh-CN)<br>
> [Axios官网](http://www.axios-js.com/)<br>
> [Eclipse Mosquitto官网](http://www.mosquitto.org/)<br>

### 应用类网站

> [Arduino官网](https://www.arduino.cc/)<br>
> [NodeMCU官网](https://www.nodemcu.com/)<br>
> [Vue.js官网](https://cn.vuejs.org/)<br>
> [MQTT官网](https://mqtt.org/)<br>
> [心知天气官网](https://www.seniverse.com/)<br>

### 知识类网站

> [太极创客官网](http://www.taichi-maker.com/)<br>
> [菜鸟教程官网](https://www.runoob.com/)<br>

### libraries

> [U8g2库](https://github.com/olikraus/u8g2/)<br>
> [Ticker库](https://github.com/sstaub/Ticker/)<br>
> [ArduinoJSON库](https://github.com/bblanchon/ArduinoJson)<br>
> [NTPClient库](https://github.com/arduino-libraries/NTPClient)<br>
> [PubSubClient库](https://github.com/knolleary/pubsubclient)<br>
> [MQTT.js库](https://github.com/mqttjs/MQTT.js)<br>
