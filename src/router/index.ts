import { createRouter, createWebHistory } from 'vue-router'
import type { RouteRecordRaw } from 'vue-router'
import Login from '@/components/Login.vue'
import Register from '@/components/Register.vue'
import BookList from '@/components/BookList.vue'
import MyBooks from '@/components/MyBooks.vue'
import BookAdmin from '@/components/BookAdmin.vue'
import History from '@/components/History.vue'

const routes: Array<RouteRecordRaw> = [
  { path: '/', redirect: '/login' },
  { path: '/login', component: Login },
  { path: '/register', component: Register },
  { path: '/books', component: BookList },
  { path: '/my', component: MyBooks },
  { path: '/admin/books', component: BookAdmin },
  { path: '/history', component: History }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

export default router
