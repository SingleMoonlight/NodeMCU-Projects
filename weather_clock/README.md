##  文件说明

+ ***Pic文件夹*** 下是开始界面和天气的图标，可以直接进行取模。
+ ***illus文件夹*** 下是[README](https://github.com/SingleMoonlight/NodeMCU-projects/blob/main/README.md)文档引用的插图。
+ ***界面样式设计.rp*** 是使用Axure RP画的LED界面。
+ ***weather_clock.ino*** 是程序源文件，但还无法直接运行，需要稍作修改。
  修改的地方有：
  1. 第15行。请求天气信息所用的私钥是个人的，当在心知天气官网注册账号后会自动生成。
  2. 第16行。待查询天气的城市可以自行选择，填写城市的拼音即可。
  3. 第490~492行。添加的WiFi名称和密码，NodeMCU在运行时会连接这几个WiFi中信号最强的。