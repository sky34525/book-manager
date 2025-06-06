<template>
  <el-form :model="loginForm" ref="formRef" class="login-form" @submit.prevent="onLogin">
    <el-form-item label="用户名" prop="username">
      <el-input v-model="loginForm.username" autocomplete="off" />
    </el-form-item>
    <el-form-item label="密码" prop="password">
      <el-input v-model="loginForm.password" type="password" autocomplete="off" />
    </el-form-item>
    <el-form-item>
      <el-button type="primary" @click="onLogin">登录</el-button>
      <el-button type="text" @click="$router.push('/register')">注册</el-button>
    </el-form-item>
  </el-form>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import axios from 'axios'
import { ElMessage } from 'element-plus'

const isMock = false // mock 预览模式
const router = useRouter()
const loginForm = ref({
  username: '',
  password: ''
})

const onLogin = async () => {
  if (isMock) {
    localStorage.setItem('token', 'mock-token')
    localStorage.setItem('username', loginForm.value.username)
    localStorage.setItem('role', loginForm.value.username === 'admin' ? 'admin' : 'reader')
    ElMessage.success('登录成功（mock）')
    router.push('/books')
    return
  }
  try {
    const res = await axios.post('http://localhost:18080/api/login', loginForm.value)
    localStorage.setItem('token', res.data.token)
    localStorage.setItem('username', res.data.username)
    localStorage.setItem('role', res.data.role)
    ElMessage.success('登录成功')
    if (res.data.role === 'admin') {
      router.push('/admin/books')
    } else {
      router.push('/books')
    }
  } catch (e) {
    ElMessage.error('用户名或密码错误')
  }
}
</script>

<style scoped>
.login-form {
  max-width: 400px;
  margin: 100px auto;
}
</style>
