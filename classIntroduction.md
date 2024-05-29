## 面向过程的操作

- PUT(Key, Value) 设置键 Key 的值为 Value。
- GET(Key) 读取键 Key 的值。
- DEL(Key) 删除键 Key 及其值。
- SCAN(Key1,Key2) 读取键 Key 在[Key1,Key2]区间内的键值对。
- GC(Chunck_size) 扫描totalSize>=chunksize的vlogEntry，打洞并将合法内容写到MemTable中。
- reset() 重置整个内容。

## 转为面向对象的处理

### KVStore

1. PUT(Key, Value)
调用MemTable的PUT并查看size，
    if size大于预定值则调用SSTablehandler的转换函数。

2. GET(Key)
调用MemTable的GET
    - If found in memtable: return the value
    - If not found in memtable  
        调用SSTableHandler的GET,return 

3. DEL(Key) 删除键 Key 及其值。
调用SSTbaleHandler的GET(key)
    - if found in sstable: insert a record called "~DELETED~"
    - if not found, find in memtable 
                - if found, delete it
                - else return false.

4. SCAN(Key1,Key2)

调用SSTableHandler的SACN，加入到list<K,V>中
调用MemTable的SCAN,add all got in list<K,V> 如果存在KEY相同，则替代掉

5. GC(Chunck_size)
扫描 vLog 头部 GCSize 大小内的 vLog entry，使用该 vLog entry的 Key 在 LSM Tree 中查找最新的记录，比较其 offset 是否指向该vLog entry
    if the latest, then save in memtable, 调用MemTable的PUT
    else do nothing
pack the memtable into sstable.
do_alloc_file()

6. reset()
在调用 reset() 函数时，其应将所有层的 SSTable文件（以及表示层的目录）、 vLog 文件删除，清除内存中 MemTable和缓存等数据， 将 tail 和 head 的值置 0，使键值存储系统恢复到空的状态。

7. init()
调用SSTABLEHANDLER读取各层SSTable文件，读入内存中。
扫描vlog,恢复tail,head的值。

### MemTable

1. 构造函数
2. 析构函数 ~MemTable

### SSTable

2. GET(Key)
- 逐层查找SSTable：先判断maxKeyvalue, minKeyValue, then bloomFilter, then 二分查找
            - if found, return value
            - else return empty

### SSTableHandler

1. SSTable convertMemTableToSSTable(const MemTable& memTable);