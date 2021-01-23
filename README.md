#  NodeMCU-projects

NodeMCU是一个开源的物联网平台，拥有开源，交互式，可编程，低成本，简单，智能，WI-FI硬件等特点，可以在Arduino IDE中进行编程开发，对新手十分友好。这是本人使用NodeMCU开发的一些小项目，**仅为自己一边学习一边总结记录**。

+ [weather-clock](#weather-clock)
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

关于U8g2库相关函数的使用，网上有很多详细的资料，U8g2库也提供了丰富的示例程序(运行时需要取消构造函数的注释)，可供研究使用。本项目里绘制所使用的比较的函数有：

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

## loading

继续学习更新......

## 参考资料

这些小项目虽然功能简单甚至简陋，但是制作的背后都有许多新的知识。发现问题、分析问题然后解决问题，在互联网上几乎可以找到任何自己想要的资料。除了这些网站以外，还参考了很多的博客、文章、B站视频等，内容很多不再一一列举，非常感谢这些内容背后的创作者，分享知识、共同进步。

> [Arduino官网](https://www.arduino.cc/)<br/>
> [NodeMCU官网](https://www.nodemcu.com/)<br/>
> [ArduinoJson官网](https://arduinojson.org/)<br/>
> [U8g2库](https://github.com/olikraus/u8g2/)<br/>
> [Ticker库](https://github.com/sstaub/Ticker/)<br/>
> [NTPClient库](https://github.com/arduino-libraries/NTPClient)<br/>
> [心知天气官网](https://www.seniverse.com/)<br/>
> [太极创客官网](http://www.taichi-maker.com/)<br/>
