var mqtt = require('../../utils/mqtt');
var client = null;
Page({
  /**
   * 页面的初始数据
   */
  data: {
    checkedOnline: false,
    checkedOutline: true,
    receiveInfo: '',
    scrollTop: 0,
    subTopic: '',
    pubTopic: '',
    pubMsg: '',
  },

  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function () {
    wx.showLoading({
      title: '加载中',
      mask: true,
    });
    wx.hideLoading();
  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {
    //断开连接
    if(client != null){
      client.end();
    }
  },

  /**
   * 连接MQTT服务器并订阅
   */
  connectMqtt: function() {
    //this使用时不能超过两层函数，所以要在外层提前定义
    var that = this;
    wx.showLoading({
      title: '连接中',
      mask: true,
    });
    //连接mqtt服务器
    client = mqtt.connect(this.getMqttServerIP(), this.getMqttOptions());
    //网络错误
    client.on('error', (error) => {
      //修改radio状态
      this.setData({    
        checkedOnline: false,
        checkedOutline: true,
      });
      wx.showToast({
        title: '网络错误',
        mask: true,
        icon: 'none',
        duration: 1000
      });
    });
    //重连
    client.on('reconnect', (error) => {
      //修改radio状态
      this.setData({    
        checkedOnline: false,
        checkedOutline: true,
      });
      wx.showLoading({
        title: '尝试重新连接',
        mask: true,
      });
    });
    //连接成功
    client.on('connect', (e) => {
      wx.showToast({
        title: '连接成功',
        mask: true,
        icon: 'success',
        duration: 1000
      });
      //修改radio状态
      this.setData({    
        checkedOnline: true,
        checkedOutline: false,
      });
    });
    //接受消息监听
    client.on('message', function (topic, message) {
      that.setData({
        receiveInfo: that.data.receiveInfo + topic.toString() + ": " + message.toString() + "\n",
        //更新竖向滚动条位置，消息条数乘以每条消息的高度
        scrollTop: that.data.scrollTop + 20
      });
    });
  },

  /**
   * 获取主题输入框内容
   * @param {*} e 
   */
  getTopic: function(e) {
    this.setData({topic: e.detail.value});
  },

  /**
   * 获取消息输入框内容
   * @param {*} e 
   */
  getMessage: function(e) {
    this.setData({message: e.detail.value});
  },

  /**
   * 向指定主题发布消息
   */
  publishMsg: function() {
    if(this.data.pubTopic != '' && this.data.pubMsg != ''){
      if(client != null){
        wx.showLoading({
          title: '发布中',
          mask: true,
        });
        //发布消息
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
      }
      else {
        wx.showToast({
          title: '请先连接服务器',
          mask: true,
          icon: 'none',
          duration: 1000
        });
      }
    }
    else {
      wx.showToast({
        title: '请先输入发布主题及消息',
        mask: true,
        icon: 'none',
        duration: 1000
      });
    }
  },

  /**
   * 获取服务器Ip
   */
  getMqttServerIP: function() {
    return 'wxs://test.mosquitto.org';
  },

  /**
   * 获取客户端连接mqtt服务器的配置信息
   */
  getMqttOptions: function() {
    return {
      clientId: '573b79e3bae7431b8f2e7fb50a0ed2ee',
      keepalive: 60,
      clean: true,
      port: 8081,
      protocolVersion: 4 //MQTT v3.1.1
    };
  },

  /**
   * 订阅主题
   */
  subTopics: function() {
    if(this.data.subTopic != ''){
      if(client != null){
        wx.showLoading({
          title: '订阅中',
          mask: true,
        });
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
      }
      else {
        wx.showToast({
          title: '请先连接服务器',
          mask: true,
          icon: 'none',
          duration: 1000
        });
      }
    }
    else{
      wx.showToast({
        title: '请先输入订阅主题',
        mask: true,
        icon: 'none',
        duration: 1000
      });
    }
  },

  /**
   * 获取订阅主题输入框内容
   * @param {*} e 
   */
  getSubTopic: function(e) {
    this.setData({subTopic: e.detail.value});
  },

  /**
   * 获取发布主题输入框内容
   * @param {*} e 
   */
  getPubTopic: function(e) {
    this.setData({pubTopic: e.detail.value});
  },

  /**
   * 获取发布消息输入框内容
   * @param {*} e 
   */
  getPubMsg: function(e) {
    this.setData({pubMsg: e.detail.value});
  },
});