<template>
  <div class="background">
    <el-row>
      <div class="topPart">
        <h1>NODE MCU</h1>
        <el-divider></el-divider>
        <h4>LAN Micro Server</h4>
      </div>
    </el-row>
    <el-row>
      <el-col :span="12">
        <div class="leftPart">
          <FrostedGlass></FrostedGlass>
          <router-link to="/Control">
            <el-button icon="el-icon-right" circle></el-button>
          </router-link>
          <h6>Go to the control panel</h6>
          <br><br>
          <el-link>Copyright (c) 2021, winston.wyx@foxmail.com All rights reserved.</el-link>
        </div>
      </el-col>
      <el-col :span="12">
        <div class="rightPart">
          <span>
            参考链接
            <br><br>
            <el-link href="http://www.nodemcu.com/index_cn.html" target="_blank">NodeMCU官网</el-link>
            <br><br>
            <el-link href="https://www.arduino.cc/" target="_blank">Arduino官网</el-link>
            <br><br>
            <el-link href="https://cn.vuejs.org/" target="_blank">Vue官网</el-link>
            <br><br>
            <el-link href="http://www.taichi-maker.com/" target="_blank">太极创客官网</el-link>
            <br><br>
            <el-link href="https://github.com/SingleMoonlight/NodeMCU-projects" target="_blank">GitHub地址</el-link>
          </span>
          <img src="../assets/nodemcu.png">
        </div>
      </el-col>
    </el-row>
  </div>
</template>

<script>
  //毛玻璃组件
  import FrostedGlass from "@/components/FrostedGlass";

export default {
  name: 'HelloWorld',
  components: {
    FrostedGlass,
  },
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
  .background {
    background-size: cover;
    background-image: -webkit-radial-gradient(45px 45px, rgba(0, 35, 106, 0.96), rgba(0, 34, 79, 0.96) 15%, black 80%);
    color: aliceblue;
    height: 100%;
    width: 100%;
    top: 0;
    left: 0;
    position: fixed;
    overflow-y:scroll;
    overflow-x:hidden;
  }
  ::-webkit-scrollbar {
    /*隐藏滚动条(对谷歌和Safari浏览器有效)*/
    display: none;
  }
  .topPart {
    margin-left: 20px;
    margin-right: 20px;
  }
  .leftPart {
    margin-left: 60px;
    margin-top: 500px;
  }
  .rightPart {
    margin-left: 220px;
    margin-top: 40px;
  }
</style>
