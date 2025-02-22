#pragma once

#include "skiplist.h"
#include <string>
#include <list>
#include <utility>

#define MEMTABLE_THRESHOLD 2097152 // 2MB

class MemTable
{
  public:
    MemTable();
    ~MemTable();
    void put(uint64_t key, const std::string &s);
    std::string get(uint64_t key);
    bool del(uint64_t key);
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list);
    void print();
    void clean();
    void getList(std::list<std::pair<uint64_t, std::string>> &list);
    uint64_t getSize();
    void setSize(uint64_t size);
    void addDeletedKey(uint64_t key);
    SkipList& getSkipList();
  private:
    SkipList skiplist;
    uint64_t size;
};