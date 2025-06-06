import { createApp } from 'vue'
import App from './App.vue'
import router from './router' // 引入刚才配置的路由

// ✅ 引入 Element Plus 及其样式
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'

// ✅ 如果你想使用中文语言包（可选）
import zhCn from 'element-plus/es/locale/lang/zh-cn'
import 'dayjs/locale/zh-cn'

const app = createApp(App)

app.use(ElementPlus, {
  locale: zhCn,
})

app.use(router) // 挂载路由

app.mount('#app')
