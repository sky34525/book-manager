<template>
  <div style="margin-bottom: 16px;">
    <el-button type="primary" @click="fetchHistory">刷新</el-button>
  </div>
  <el-table :data="historyList" style="width: 100%">
    <el-table-column prop="bookTitle" label="书名" />
    <el-table-column prop="borrowTime" label="借阅时间" />
    <el-table-column prop="returnTime" label="归还时间" />
    <el-table-column label="是否逾期">
      <template #default="scope">
        <span v-if="scope.row.returnTime && isOverdue(scope.row)">是</span>
        <span v-else>否</span>
      </template>
    </el-table-column>
  </el-table>
</template>

<script setup>
import { ref, onMounted, watch } from 'vue'
import { useRoute } from 'vue-router'
import axios from 'axios'

const isMock = false // mock 预览模式
const historyList = ref([])
const route = useRoute()

const isOverdue = (row) => {
  if (!row.borrowTime || !row.returnTime) return false
  const borrow = new Date(row.borrowTime)
  const returnT = new Date(row.returnTime)
  // 借阅时间+7天 < 归还时间，则逾期
  return returnT.getTime() > borrow.getTime() + 7 * 24 * 60 * 60 * 1000
}

const fetchHistory = async () => {
  if (isMock) {
    historyList.value = [
      { bookTitle: '三体', borrowTime: '2024-05-01T10:00:00Z', returnTime: '2024-05-10T10:00:00Z' },
      { bookTitle: '活着', borrowTime: '2024-05-01T10:00:00Z', returnTime: '2024-05-20T10:00:00Z' } // 逾期
    ]
    return
  }
  const res = await axios.get('http://localhost:18080/api/history', {
    params: { user: localStorage.getItem('username') }
  })
  historyList.value = res.data
}

onMounted(fetchHistory)
watch(() => route.fullPath, fetchHistory)
</script> 