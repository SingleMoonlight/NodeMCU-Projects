import VueRouter from 'vue-router'
import HelloWorld from '@/components/HelloWorld'
import Control from "@/components/Control";

//创建路由对象
const router = new VueRouter({
    // 去掉路由地址的#
    //mode: 'history',
    routes: [
        {
            path: '/',
            name: 'HelloWorld',
            component: HelloWorld,
        },
        {
            path: '/Control',
            name: 'Control',
            component: Control,
        },
    ]
});

//把路由对象暴露出去
export default router