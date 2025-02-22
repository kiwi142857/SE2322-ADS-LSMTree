#pragma once

#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <utility>
#include "../memtable/memTable.h"
#include "../utils/bloomFilter.h"
#include "../vlog/vLog.h"
#include "sstable.h"

#define kb 1024
#define mb 1024 * kb
#define gb 1024 * mb

#define MEMTABLE_THRESHOLD 408

class SSTableHandler
{
    // SSTable objects
    std::vector<SSTable*> sstables;
    int maxLevel;
    uint64_t timeId;
    std::string dir;
    std::string vlog;

  public:
    std::fstream vlogFile;
    // Constructor
    SSTableHandler(const std::string &dir, const std::string &vlog) : maxLevel(0), timeId(0), dir(dir), vlog(vlog)
    {
        vlogFile.open(vlog, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
    };

    ~SSTableHandler()
    {
        vlogFile.close();
    }

    // Function to convert a MemTable to a SSTable
    void convertMemTableToSSTable(const MemTable &memtable);

    // Function to get a value from the SSTables
    std::string get(uint64_t key);

    // Function to get a value's offset from the SSTables
    // return uint64_t max if key not found
    uint64_t getOffset(uint64_t key);

    // Function to scan the SSTables for a range of keys
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list);

    // Function to perform garbage collection on the SSTables
    void gc(uint64_t chunkSize);

    // Function to reset the SSTables
    void reset();

    // Function to initialize the SSTables from disk
    void init();

    // Function to input file
    void input(std::string filename, int level, std::string fileSubName);

    // Function to input file
    SSTable input(std::string filename);

    // Function to perform compaction on a level
    void compact(int level);

    // Function to perform compaction on level 0
    void compactLevel0();

    void printVLog(int offset)
    {
        // Save the current position
        vlogFile.seekg(offset);
        char magic;
        vlogFile.read(&magic, 1);

        vLogEntry entry;
        vlogFile.read((char *)&entry.checksum, sizeof(entry.checksum));
        vlogFile.read((char *)&entry.key, sizeof(entry.key));
        vlogFile.read((char *)&entry.vlen, sizeof(entry.vlen));
        entry.value.resize(entry.vlen);
        vlogFile.read(&entry.value[0], entry.vlen);

        std::vector<unsigned char> data;
        for (size_t i = 0; i < sizeof(entry.key); ++i) {
            data.push_back((entry.key >> (i * 8)) & 0xFF);
        }
        for (size_t i = 0; i < sizeof(entry.vlen); ++i) {
            data.push_back((entry.vlen >> (i * 8)) & 0xFF);
        }
        for (char c : entry.value) {
            data.push_back(c);
        }

        printf("Key: %lu\n", entry.key);
        printf("Value: %s\n", entry.value.c_str());
    }
};