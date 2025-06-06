<template>
  <div style="margin-bottom: 16px;">
    <span>用户名：{{ username }}</span>
    <span style="margin-left: 24px;">最大可借书数：{{ maxBooks }}</span>
  </div>
  <el-table :data="borrowList" style="width: 100%">
    <el-table-column prop="bookTitle" label="书名" />
    <el-table-column prop="borrowTime" label="借阅时间" />
    <el-table-column label="是否逾期">
      <template #default="scope">
        <span v-if="isOverdue(scope.row)" style="color: red;">逾期</span>
        <span v-else>正常</span>
      </template>
    </el-table-column>
    <el-table-column label="操作">
      <template #default="scope">
        <el-button type="primary" @click="returnBook(scope.row)" size="small">归还</el-button>
      </template>
    </el-table-column>
  </el-table>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'
import { ElMessage } from 'element-plus'

const isMock = false // mock 预览模式
const username = ref(localStorage.getItem('username') || '测试用户')
const maxBooks = ref(5)
const borrowList = ref([])

const isOverdue = (row) => {
  if (!row.borrowTime) return false
  const borrow = new Date(row.borrowTime)
  const now = new Date()
  return now.getTime() > borrow.getTime() + 7 * 24 * 60 * 60 * 1000
}

const fetchBorrowList = async () => {
  if (isMock) {
    borrowList.value = [
      { bookId: 1, bookTitle: '三体', borrowTime: '2024-06-01T10:00:00Z' },
      { bookId: 2, bookTitle: '活着', borrowTime: '2024-05-20T10:00:00Z' } // 逾期
    ]
    return
  }
  const res = await axios.get('http://localhost:18080/api/mybooks', {
    params: { user: username.value }
  })
  borrowList.value = res.data
}

onMounted(fetchBorrowList)

const returnBook = async (row) => {
  if (isMock) {
    ElMessage.success('归还成功（mock）')
    borrowList.value = borrowList.value.filter(b => b.bookId !== row.bookId)
    return
  }
  try {
    await axios.post('http://localhost:18080/api/return', {
      bookId: row.bookId,
      user: localStorage.getItem('username')
    })
    ElMessage.success('归还成功')
    fetchBorrowList()
  } catch (e) {
    ElMessage.error('归还失败')
  }
}
</script> 