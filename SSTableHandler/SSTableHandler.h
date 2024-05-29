#pragma once
#include "../MemTable/memTable.h"
#include "../sstable/sstable.h"
#include <optional>
#include <vector>

#define kb 1024
#define mb 1024 * kb
#define gb 1024 * mb

class SSTableHandler
{
    // SSTable objects
    std::vector<std::vector<SSTable>> sstables;
    int maxLevel;
    uint64_t timeId;
    std::fstream vlogFile;

  public:
    // Constructor
    SSTableHandler() : maxLevel(0), timeId(0)
    {
        vlogFile.open("./data/vlog", std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
    };

    ~SSTableHandler()
    {
        vlogFile.close();
    }

    // Function to convert a MemTable to a SSTable
    void convertMemTableToSSTable(MemTable &memTable);

    // Function to get a value from the SSTables
    std::string get(uint64_t key);

    // Function to scan the SSTables for a range of keys
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list);

    // Function to perform garbage collection on the SSTables
    void gc(uint64_t chunkSize);

    // Function to reset the SSTables
    void reset();

    // Function to initialize the SSTables from disk
    void init();

    // Function to perform compaction on a level
    void compact(int level);

    

};