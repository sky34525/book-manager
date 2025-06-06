<template>
  <div style="margin-bottom: 16px;">
    <el-input v-model="search" placeholder="输入书名或作者" style="width: 200px; margin-right: 8px;" @keyup.enter="fetchBooks" />
    <el-button type="primary" @click="fetchBooks">搜索</el-button>
    <el-button type="success" @click="showAddDialog = true" style="margin-left: 8px;">新增图书</el-button>
  </div>
  <el-table :data="bookList" style="width: 100%">
    <el-table-column prop="id" label="ID" width="80" />
    <el-table-column prop="title" label="书名" />
    <el-table-column prop="author" label="作者" />
    <el-table-column prop="stock" label="库存" />
    <el-table-column label="操作">
      <template #default="scope">
        <el-button @click="editBook(scope.row)" size="small">编辑</el-button>
        <el-button type="danger" @click="deleteBook(scope.row)" size="small">删除</el-button>
      </template>
    </el-table-column>
  </el-table>

  <!-- 新增/编辑图书弹窗 -->
  <el-dialog v-model="showDialog" :title="dialogTitle">
    <el-form :model="form">
      <el-form-item label="书名">
        <el-input v-model="form.title" />
      </el-form-item>
      <el-form-item label="作者">
        <el-input v-model="form.author" />
      </el-form-item>
      <el-form-item label="库存">
        <el-input-number v-model="form.stock" :min="0" />
      </el-form-item>
    </el-form>
    <template #footer>
      <el-button @click="showDialog = false">取消</el-button>
      <el-button type="primary" @click="submitForm">确定</el-button>
    </template>
  </el-dialog>
</template>

<script setup>
import { ref, reactive, onMounted, watchEffect } from 'vue'
import axios from 'axios'
import { ElMessage } from 'element-plus'

const isMock = false // mock 预览模式
const bookList = ref([])
const search = ref('')
const showDialog = ref(false)
const dialogTitle = ref('')
const form = reactive({ id: null, title: '', author: '', stock: 1 })
let isEdit = false

const fetchBooks = async () => {
  if (isMock) {
    bookList.value = [
      { id: 1, title: '三体', author: '刘慈欣', stock: 3 },
      { id: 2, title: '活着', author: '余华', stock: 0 }
    ]
    return
  }
  const res = await axios.get('http://localhost:18080/api/books', {
    params: { q: search.value }
  })
  bookList.value = res.data
}

onMounted(fetchBooks)

const showAddDialog = ref(false)

const resetForm = () => {
  form.id = null
  form.title = ''
  form.author = ''
  form.stock = 1
}

const submitForm = async () => {
  if (isMock) {
    if (isEdit) {
      ElMessage.success('修改成功（mock）')
      bookList.value = bookList.value.map(b => b.id === form.id ? { ...form } : b)
    } else {
      ElMessage.success('新增成功（mock）')
      bookList.value.push({ ...form, id: Date.now() })
    }
    showDialog.value = false
    return
  }
  try {
    if (isEdit) {
      await axios.post('http://localhost:18080/api/books/update', form)
      ElMessage.success('修改成功')
    } else {
      await axios.post('http://localhost:18080/api/books/add', form)
      ElMessage.success('新增成功')
    }
    showDialog.value = false
    fetchBooks()
  } catch (e) {
    ElMessage.error('操作失败')
  }
}

const editBook = (book) => {
  isEdit = true
  dialogTitle.value = '编辑图书'
  form.id = book.id
  form.title = book.title
  form.author = book.author
  form.stock = book.stock
  showDialog.value = true
}

const deleteBook = async (book) => {
  if (isMock) {
    ElMessage.success('删除成功（mock）')
    bookList.value = bookList.value.filter(b => b.id !== book.id)
    return
  }
  try {
    await axios.post('http://localhost:18080/api/books/delete', { id: book.id })
    ElMessage.success('删除成功')
    fetchBooks()
  } catch (e) {
    ElMessage.error('删除失败')
  }
}

watchEffect(() => {
  if (showAddDialog.value) {
    isEdit = false
    dialogTitle.value = '新增图书'
    resetForm()
    showDialog.value = true
    showAddDialog.value = false
  }
})
</script> 