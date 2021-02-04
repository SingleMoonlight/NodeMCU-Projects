<template>
    <div class="background">
        <!-- 第1行 -->
        <el-row>
            <div class="topPart">
            <el-col :span="22">
                <h3>Node MCU LAN Micro Server Control Panel</h3>
            </el-col>
            <el-col :span="2">
                <router-link to="/">
                    <el-tooltip class="item" effect="dark" content="回到首页" placement="bottom">
                        <i style="margin-top: 24px; margin-left: 40px; color: aliceblue" class="el-icon-close"></i>                    </el-tooltip>
                </router-link>
            </el-col>
            </div>
        </el-row>
        <br>
        <!-- 第2行，分成3列，列间隔12 -->
        <el-row :gutter="12">
            <div class="bottomPart">
                <!-- 第1列两张卡片 -->
                <el-col :span="8">
                    <!-- 板载LED控制卡片 -->
                    <el-card shadow="hover">
                        <h3>板载LED控制</h3>
                        <el-divider></el-divider>
                        <span>开关</span>
                        <br><br>
                        <el-switch
                                v-model="LedState"
                                active-color="#409EFF"
                                inactive-color="#797979"
                                @change="changeLedState()">
                        </el-switch>
                        <br><br>
                        <span>亮度调节</span>
                        <br><br>
                        <el-slider v-model="LedBrightness" @change="changeLedBrightness()"></el-slider>
                        <br>
                        <span>其他控制</span>
                        <br><br>
                        <el-radio v-model="LedMode" label="1" @change="selectLedMode()">呼吸灯模式</el-radio>
                        <el-radio disabled v-model="LedMode" label="2">期待...</el-radio>
                    </el-card>
                    <br><br>
                    <!-- 信息交互卡片 -->
                    <el-card shadow="hover">
                        <el-dropdown split-button style="float: right; padding: 13px 0" type="text" @click="openMassage()">
                            点我
                            <el-dropdown-menu slot="dropdown">
                                <el-dropdown-item>闪存信息</el-dropdown-item>
                                <el-dropdown-item>服务器IP</el-dropdown-item>
                                <el-dropdown-item>项目地址</el-dropdown-item>
                                <el-dropdown-item>......</el-dropdown-item>
                            </el-dropdown-menu>
                        </el-dropdown>
                        <h3>信息交互</h3>
                        <el-divider></el-divider>
                        <el-input
                                type="textarea"
                                :rows="5"
                                placeholder="您想发送的信息"
                                v-model="InteractInfo">
                        </el-input>
                        <br><br>
                        <el-button type="primary" plain @click="interactInfo()">发送</el-button>
                    </el-card>
                </el-col>
                <!-- 第2列一张卡片 -->
                <el-col :span="8">
                    <!-- 文件管理卡片 -->
                    <el-card shadow="hover">
                        <el-button style="float: right; padding: 23px 0" type="text" @click="openMassage2()">点我</el-button>
                        <h3>文件管理</h3>
                        <el-divider></el-divider>
                        <span>文件下载</span>
                        <br><br>
                        <el-autocomplete
                                style="width: 87%"
                                v-model="SelectFile"
                                :fetch-suggestions="querySearchAsync"
                                placeholder="请选择文件"
                                @select="handleSelect"
                                @focus="loadFiles"
                        ></el-autocomplete>
                        <br><br>
                        <el-button type="primary" plain @click="downloadFile()">下载</el-button>
                        <br><br>
                        <span>文件上传</span>
                        <br><br>
                        <el-upload
                                action=""
                                ref="upload"
                                :http-request="requestFile"
                                :auto-upload="false"
                                :drag="true"
                                multiple>
                            <i class="el-icon-upload"></i>
                            <div class="el-upload__text">将文件拖到此处，或<em>点击上传</em></div>
                            <div class="el-upload__tip" slot="tip">NodeMCU闪存空间较小(只有4M)，除去代码及系统文件所占空间，闪存空间所剩无几，只能上传一些简单的文件，建议(一共)不要超过500kb</div>
                        </el-upload>
                        <br>
                        <el-button type="primary" plain @click="uploadFile()">上传</el-button>
                    </el-card>
                </el-col>
                <!-- 第3列两张卡片 -->
                <el-col :span="8">
                    <!-- 舵机控制卡片 -->
                    <el-card shadow="hover">
                        <h3>舵机控制</h3>
                        <el-divider></el-divider>
                        <span>角度调节</span>
                        <br><br>
                        <div class="block">
                            <el-slider
                                    v-model="ServoAngle"
                                    @change="changeServoAngle()"
                                    :max="180"
                                    show-input>
                            </el-slider>
                        </div>
                    </el-card>
                    <br><br>
                    <!-- 模拟引脚读数卡片 -->
                    <el-card shadow="hover">
                        <h3>模拟引脚读数</h3>
                        <el-divider></el-divider>
                        <span>读数：{{this.AnalogPinValue}}</span>
                        <br><br>
                        <el-slider v-model="AnalogPinValue" disabled :max="20"></el-slider>
                        <br><br>
                        <el-button type="primary" plain @click="getAnalogPinValue()">获取</el-button>
                    </el-card>
                </el-col>
            </div>
        </el-row>
    </div>
</template>

<script>
    export default {
        name: "Control",
        data() {
            return {
                LedState: true,                 //LED灯状态
                LedBrightness: 0,               //LED灯亮度
                LedMode: '',                    //LED灯模式
                InteractInfo: '',               //交互信息
                ServoAngle: 0,                  //舵机角度
                AnalogPinValue: 0,              //模拟引脚读数
                SelectFile: '',                 //选择下载的文件
                Files: [],                      //可供下载的文件列表
                timeout: null,
            }
        },
        methods: {
            /********************************************
            *方 法 名：changeLedState
            *功    能：改变LED灯的状态
            *触    发：点击开关时触发
            *参    数：无
            *返 回 值：无
             *******************************************/
            changeLedState() {
                this.$axios.get('/changeLedState',
                    {
                        params: {
                            LedState: this.LedState
                        }
                    }
                ).then(response => {
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
                }).catch(err => {
                    console.log(err);
                });
                //当点击开关或滑动滑块时，让单选框处于未选中状态
                this.LedMode = '';
            },
            /********************************************
             *方 法 名：changeLedBrightness
             *功    能：改变LED灯的亮度
             *触    发：滑动滑块停止时触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            changeLedBrightness() {
                this.$axios.get('/changeLedBrightness',
                    {
                        params: {
                            LedBrightness: this.LedBrightness
                        }
                    }
                ).then(response => {
                    if (response.data === "Success") {
                        this.$message({
                            showClose: false,
                            message: '调节成功！',
                            type: 'success',
                            center: true
                        });
                    } else {
                        this.$message({
                            showClose: false,
                            message: '调节失败！',
                            type: 'error',
                            center: true
                        });
                    }
                }).catch(err => {
                    console.log(err);
                });
                //当点击开关或滑动滑块时，让单选框处于未选中状态
                this.LedMode = '';
            },
            /********************************************
             *方 法 名：selectLedMode
             *功    能：选择LED灯的模式
             *触    发：点击单选框时触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            selectLedMode() {
                this.$axios.get('/selectLedMode',
                    {
                        params: {
                            LedMode: this.LedMode
                        }
                    }
                ).then(response => {
                    if (response.data === "Success") {
                        this.$message({
                            showClose: false,
                            message: '选择成功！',
                            type: 'success',
                            center: true
                        });
                    } else {
                        this.$message({
                            showClose: false,
                            message: '选择失败！',
                            type: 'error',
                            center: true
                        });
                    }
                }).catch(err => {
                    console.log(err);
                });
            },
            /********************************************
             *方 法 名：interactInfo
             *功    能：前后端信息交互
             *触    发：在文本框输入内容，点击发送按钮触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            interactInfo() {
                this.$axios.get('/interactInfo',
                    {
                        params: {
                            InteractInfo: this.InteractInfo
                        }
                    }
                ).then(response => {
                    //在输入框里显示后端服务器返回的信息
                    this.InteractInfo = response.data;
                }).catch(err => {
                    console.log(err);
                });
            },
            /********************************************
             *方 法 名：getAnalogPinValue
             *功    能：获取模拟引脚读数
             *触    发：点击获取按钮后，每隔2s触发一次
             *参    数：无
             *返 回 值：无
             *******************************************/
            getAnalogPinValue() {
                setInterval(()=>{
                    this.$axios.get('/getAnalogPinValue').then(response => {
                        //将后端服务器返回的模拟引脚读数转换为整型，显示在界面上
                        this.AnalogPinValue = parseInt(response.data);
                    }).catch(err => {
                        console.log(err);
                    });
                }, 2000);
            },
            /********************************************
             *方 法 名：uploadFile
             *功    能：调用自定义文件上传函数
             *触    发：选择文件后点击上传按钮触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            uploadFile(){
                this.$refs.upload.submit();
            },
            /********************************************
             *方 法 名：requestFile
             *功    能：自定义的上传方法，用于上传文件
             *触    发：点击上传按钮被调用触发
             *参    数：param
             *返 回 值：无
             *******************************************/
            requestFile(param) {
                //文件对象
                let fileObj = param.file;
                //FormData对象
                let fd = new FormData();
                fd.append('UploadFile', fileObj);
                let url = '/uploadFile';
                let config = {
                    headers: {
                        'Content-Type': 'multipart/form-data'
                    }
                };
                this.$axios.post(url, fd, config).then(response=>{
                    if(response.data === "Success"){
                        this.$message({
                            showClose: false,
                            message: '上传成功！',
                            type: 'success',
                            center: true
                        });
                    }else {
                        this.$message({
                            showClose: false,
                            message: '上传失败！',
                            type: 'error',
                            center: true
                        });
                    }
                }).catch(err=>{
                    console.log(err);
                    this.$message({
                        showClose: false,
                        message: '网络错误！',
                        type: 'error',
                        center: true
                    });
                });
            },
            /********************************************
             *方 法 名：loadFiles
             *功    能：加载服务器可供下载的文件
             *触    发：文件下载输入框获得焦点时触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            loadFiles() {
                //存储后端服务器返回的文件信息
                const arrayFile = [];
                this.$axios.get('/loadFiles',).then(response => {
                    //console.log(response.data);
                    let i  = response.data.length - 1;
                    while(i >= 0){
                        //单个文件信息
                        const file = {};
                        //取Json数据的filename字段，赋给file的value属性
                        //这个value属性是建议输入框条目的属性
                        file.value = response.data[i].filename;
                        //将单个文件信息加入arrayFile
                        arrayFile.push(file);
                        i--;
                    }
                }).catch(err=>{
                    console.log(err);
                    this.$message({
                        showClose: false,
                        message: '网络错误！',
                        type: 'error',
                        center: true
                    });
                });
                //获得arrayFile，赋给Files
                this.Files = arrayFile;
            },
            /* 说明：以下三个方法均移植于element官方示例，详细信息参见 https://element.eleme.cn/#/zh-CN/component/input */
            /********************************************
             *方 法 名：querySearchAsync
             *功    能：返回输入建议的方法，仅当你的输入建议数
             * 据 resolve 时，通过调用 callback(data:[]) 来
             * 返回它
             *触    发：在输入框变化时会触发该方法，获取到当前
             * 输入的字段，然后调用 createFilter 方法筛选数据
             *参    数：queryString cb
             *返 回 值：无
             *******************************************/
            querySearchAsync(queryString, cb) {
                const Files = this.Files;
                const results = queryString ? Files.filter(this.createStateFilter(queryString)) : Files;

                clearTimeout(this.timeout);
                this.timeout = setTimeout(() => {
                    cb(results);
                }, 1000 * Math.random());
            },
            /********************************************
             *方 法 名：createStateFilter
             *功    能：筛选数据
             *触    发：querySearchAsync执行后被调用触发
             *参    数：queryString
             *返 回 值：无
             *******************************************/
            createStateFilter(queryString) {
                return (state) => {
                    return (state.value.toLowerCase().indexOf(queryString.toLowerCase()) === 0);
                };
            },
            /********************************************
             *方 法 名：handleSelect
             *功    能：打印选中的项
             *触    发：仅点击选中建议项时触发
             *参    数：item
             *返 回 值：无
             *******************************************/
            handleSelect(item) {
                console.log(item);
            },
            /********************************************
             *方 法 名：downloadFile
             *功    能：下载选择的文件到本地
             *触    发：选择文件后点击下载触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            downloadFile() {
                this.$axios.get('/downloadFile',
                    {
                        params: {
                            SelectFile: this.SelectFile,
                        },
                        //响应类型
                        responseType: 'arraybuffer'
                    }
                ).then(response => {
                    //console.log(response);
                    //建立一个blob对象
                    let blob = new Blob([response.data]);
                    //自定义文件名为输入框里选择的文件(截去了/updownload/字符串)
                    let filename = this.SelectFile.substring(12, this.SelectFile.length);
                    if (typeof window.navigator.msSaveBlob !== 'undefined') {
                        window.navigator.msSaveBlob(blob, filename);
                    } else {
                        // 将blob对象转为一个URL
                        const blobURL = window.URL.createObjectURL(blob);
                        // 创建一个a标签
                        const tempLink = document.createElement('a');
                        tempLink.style.display = 'none';
                        tempLink.href = blobURL;
                        // 给a标签添加下载属性
                        tempLink.setAttribute('download', filename);
                        if (typeof tempLink.download === 'undefined') {
                            tempLink.setAttribute('target', '_blank');
                        }
                        // 将a标签添加到body当中
                        document.body.appendChild(tempLink);
                        // 启动下载
                        tempLink.click();
                        // 下载完毕删除a标签
                        document.body.removeChild(tempLink);
                        window.URL.revokeObjectURL(blobURL);
                    }
                }).catch(err => {
                    console.log(err);
                    this.$message({
                        showClose: false,
                        message: '网络错误！',
                        type: 'error',
                        center: true
                    });
                });
            },
            /********************************************
             *方 法 名：changeServoAngle
             *功    能：改变舵机的角度
             *触    发：滑动滑块停止时触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            changeServoAngle() {
                this.$axios.get('/changeServoAngle',
                    {
                        params: {
                            ServoAngle: this.ServoAngle
                        }
                    }
                ).then(response => {
                    if (response.data === "Success") {
                        this.$message({
                            showClose: false,
                            message: '调节成功！',
                            type: 'success',
                            center: true
                        });
                    } else {
                        this.$message({
                            showClose: false,
                            message: '调节失败！',
                            type: 'error',
                            center: true
                        });
                    }
                }).catch(err => {
                    console.log(err);
                });
            },
            /********************************************
             *方 法 名：openMassage
             *功    能：提示信息
             *触    发：点击信息交互卡片的 “点我” 触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            openMassage() {
                const h = this.$createElement;
                this.$notify({
                    title: 'Tips',
                    message: h('i', { style: 'color: teal'},
                        "一般来说，服务器是用来响应客户端请求的，服务器不主动和客户端交流，这里提供一个信息交互的demo，服务器根据客户端的要求做出对应的操作。您可以尝试发送下拉菜单里提供的命令，观察响应信息。"
                    )
                });
            },
            /********************************************
             *方 法 名：openMassage2
             *功    能：提示信息
             *触    发：点击文件管理卡片的“点我”触发
             *参    数：无
             *返 回 值：无
             *******************************************/
            openMassage2() {
                const h = this.$createElement;
                this.$notify({
                    title: 'Tips',
                    message: h('i', { style: 'color: teal'},
                        "本项目使用的NodeMCU闪存空间为4M，空间很小，使用需要注意。因此为了便于管理文件的上传和下载，将所有用于上传和下载的文件都放在/updownload/目录下。"
                    )
                });
            },
        }
    }
</script>

<style scoped>
    .background {
        background-size: cover;
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
        height: 60px;
        background-color: #797979;
        color: aliceblue;
        box-shadow: 0px 2px 5px #505050;
    }
    .topPart h3{
        margin-left: 20px;
    }
    .bottomPart {
        margin-left: 20px;
        margin-right: 20px;
    }
</style>