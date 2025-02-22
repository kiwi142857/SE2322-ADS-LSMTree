# LSM-Tree Key-Value Store

基于 LSM-Tree (Log-Structured Merge Tree) 实现的高性能键值存储系统。

## 项目特性

- 基于 LSM-Tree 的存储引擎实现
- 使用 SkipList 作为内存表（MemTable）数据结构
- 实现 SSTable 进行数据持久化
- 支持 Bloom Filter 优化读取性能
- 实现 WAL (Write-Ahead Log) 保证数据可靠性
- 支持 Compaction 操作优化存储空间

## 项目结构

```
.
├── src/          # 源代码目录
│   ├── core/     # 核心实现
│   ├── memtable/ # 内存表实现
│   ├── sstable/  # SSTable 实现
│   ├── utils/    # 工具类
│   └── vlog/     # WAL 实现
├── tests/        # 测试文件
└── tools/        # 工具脚本
```

## 主要组件

1. MemTable
   - 基于 SkipList 实现的内存数据结构
   - 支持快速的查询和插入操作

2. SSTable
   - 数据持久化存储格式
   - 包含 BloomFilter、索引块和数据块

3. Compaction
   - 多层 LSM-Tree 结构
   - 支持 Level Compaction

4. WAL (Write-Ahead Log)
   - 保证数据可靠性
   - 系统崩溃恢复支持

## 构建和运行

### 依赖要求

- C++11 或更高版本
- Make 或 CMake

### 构建步骤

```bash
make clean  # 清理旧的构建文件
make        # 构建项目
```

### 运行测试

```bash
./correctness    # 运行正确性测试
./persistence    # 运行持久化测试
./performance    # 运行性能测试
```

## API 接口

主要支持以下操作：

- `put(key, value)`: 插入或更新键值对
- `get(key)`: 获取指定 key 的值
- `del(key)`: 删除指定的键值对
- `scan(key1, key2)`: 范围查询
- `reset()`: 重置存储系统
- `gc(chunk_size)`: 垃圾回收

## 性能优化

1. BloomFilter 减少不必要的磁盘访问
2. 使用 SkipList 提供高效的内存操作
3. 实现分层压缩策略优化空间使用
4. 异步 Compaction 减少对正常操作的影响

## 许可证

MIT License 