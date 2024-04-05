#pragma once

#include "skiplist.h"
#include "../bloomFilter/bloomFilter.h"
#include "../sstable/sstable.h"

class MemTable
{
  public:
    MemTable();
    ~MemTable();
    void put(uint64_t key, const std::string &val);
    std::string get(uint64_t key) const;
    bool del(uint64_t key);
    void print();
    void scan(uint64_t start, uint64_t end, std::list<std::pair<uint64_t, std::string>> &list);
    void clean();
  private:
    skiplist::skiplist_type *skiplist;
};