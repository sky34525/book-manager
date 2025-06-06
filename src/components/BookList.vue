<template>
  <div style="margin-bottom: 16px;">
    <el-button type="info" @click="$router.push('/history')" style="margin-right: 8px;">借阅历史</el-button>
    <el-input v-model="search" placeholder="输入书名或作者" style="width: 200px; margin-right: 8px;" @keyup.enter="fetchBooks" />
    <el-button type="primary" @click="fetchBooks">搜索</el-button>
  </div>
  <el-table :data="bookList" style="width: 100%">
    <el-table-column prop="id" label="ID" width="80" />
    <el-table-column prop="title" label="书名" />
    <el-table-column prop="author" label="作者" />
    <el-table-column prop="stock" label="库存" />
    <el-table-column label="操作">
      <template #default="scope">
        <el-button
          v-if="role === 'reader'"
          :disabled="scope.row.stock === 0"
          @click="borrowBook(scope.row)"
          size="small"
        >借阅</el-button>
        <span v-if="role === 'reader' && scope.row.stock === 0" style="color: red; margin-left: 8px;">库存为零</span>
        <el-button
          v-if="role === 'admin'"
          @click="editBook(scope.row)"
          size="small"
        >编辑</el-button>
        <el-button
          v-if="role === 'admin'"
          type="danger"
          @click="deleteBook(scope.row)"
          size="small"
        >删除</el-button>
      </template>
    </el-table-column>
  </el-table>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'
import { ElMessage } from 'element-plus'

const isMock = false // mock 预览模式
const bookList = ref([])
const search = ref('')
const role = ref(localStorage.getItem('role') || 'reader')

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

const borrowBook = async (book) => {
  if (isMock) {
    ElMessage.success('借阅成功（mock）')
    bookList.value = bookList.value.map(b => b.id === book.id ? { ...b, stock: b.stock - 1 } : b)
    return
  }
  try {
    const res = await axios.post('http://localhost:18080/api/borrow', {
      bookId: book.id,
      token: localStorage.getItem('token'),
      username: localStorage.getItem('username')
    })
    if (res.data && res.data.error) {
      ElMessage.error(res.data.error)
    } else {
      ElMessage.success('借阅成功')
      fetchBooks()
    }
  } catch (e) {
    ElMessage.error('借阅失败')
  }
}

const editBook = (book) => {
  // 跳转到编辑页面或弹窗
  ElMessage.info('编辑功能待实现')
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
</script>