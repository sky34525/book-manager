<template>
  <el-form :model="registerForm" ref="formRef" class="register-form" @submit.prevent="onRegister">
    <el-form-item label="用户名" prop="username">
      <el-input v-model="registerForm.username" autocomplete="off" />
    </el-form-item>
    <el-form-item label="密码" prop="password">
      <el-input v-model="registerForm.password" type="password" autocomplete="off" />
    </el-form-item>
    <el-form-item label="确认密码" prop="confirmPassword">
      <el-input v-model="registerForm.confirmPassword" type="password" autocomplete="off" />
    </el-form-item>
    <el-form-item>
      <el-button type="primary" @click="onRegister">注册</el-button>
      <el-button type="text" @click="$router.push('/login')">返回登录</el-button>
    </el-form-item>
  </el-form>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import axios from 'axios'
import { ElMessage } from 'element-plus'

const router = useRouter()
const registerForm = ref({
  username: '',
  password: '',
  confirmPassword: ''
})

const onRegister = async () => {
  if (registerForm.value.password !== registerForm.value.confirmPassword) {
    ElMessage.error('两次输入的密码不一致')
    return
  }
  try {
    await axios.post('http://localhost:18080/api/register', {
      username: registerForm.value.username,
      password: registerForm.value.password
    })
    ElMessage.success('注册成功，请登录')
    router.push('/login')
  } catch (e) {
    ElMessage.error('注册失败，用户名可能已存在')
  }
}
</script>

<style scoped>
.register-form {
  max-width: 400px;
  margin: 100px auto;
}
</style>
