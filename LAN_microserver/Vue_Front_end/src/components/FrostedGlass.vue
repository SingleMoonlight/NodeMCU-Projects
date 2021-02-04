<template>
    <div>
        <main>
            <blockquote>
                这是一个运行在 NodeMCU 上的微服务器，它可以在局域网 (LAN) 里工作。
                <br><br>
                不同于传统意义上专于提供计算和应用服务、执行大量任务的服务器，它的算力十分有限，不过它可以执行一些简单的任务。
                <br><br>
                比如您现在看到的这个 Web 网页，正在运行在 NodeMCU 上。除此之外，它还可以利用局域网进行 TCP 通信，在网页上向 NodeMCU 发送控制指令 (HTTP请求)，让 NodeMCU 执行相应的任务。
                <br><br>
                现在去控制面板试试一吧！
            </blockquote>
        </main>
    </div>
</template>

<script>
    export default {
        name: "FrostedGlass"
    }
</script>

<style scoped>
    body, main::before{
        background: -webkit-radial-gradient(45px 45px, rgba(0, 35, 106, 0.96), rgba(0, 34, 79, 0.96) 15%, black 80%) 0 / cover fixed;
    }
    main{
        position: absolute;
        top: 60px; bottom: 0;
        left: 0; right: 0;
        width: 600px;
        height: 385px;
        line-height: 2;
        margin-left: 20px;
        margin-bottom: 20px;
        border-radius: 5px;
        background: rgba(255, 255, 255, 0.25);
        box-shadow: 3px 3px 6px 3px rgba(0, 0, 0, .3);
        overflow: hidden;
    }
    main::before{
        content: '';
        position: absolute;
        top: 0; bottom: 0;
        left: 0; right: 0;
        filter: blur(20px);
        z-index: -1;
        margin: -30px;
    }
</style>