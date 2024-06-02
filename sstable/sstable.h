#pragma once

#include <cstdint>
#include <vector>
#include <tuple>
#include <list>

class SSTable {

    // SSTabel header
    uint64_t timeId;
    uint64_t pairNum;
    uint64_t largestKey;
    uint64_t smallestKey;

    // Bloom filter
    std::vector<bool> bloomFilter;
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> item;

    uint32_t level;

    // 对应硬盘上文件的名称
    std::string fileName;

    public:
        SSTable(uint64_t timeId, uint64_t pairNum, uint64_t largestKey, uint64_t smallestKey, std::vector<bool> bloomFilter, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable);
        SSTable();

        // get / set functions
        uint64_t getTimeId() const;
        uint64_t getPairNum() const;
        uint64_t getLargestKey() const;
        uint64_t getSmallestKey() const;
        std::vector<bool> getBloomFilter() const;
        std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>& getItem();

        // check if the key is in the bloom filter
        bool checkBloomFilter(uint64_t num, int num_hashes = 4);

        // 通过key获取offset
        std::tuple<uint64_t, uint64_t, uint32_t> getOffset(uint64_t key);

        // output the sstable to a file
        void output(std::fstream &file);

        void scanOffset(uint64_t start, uint64_t end, std::list<std::tuple<uint64_t, uint64_t, uint32_t>> &offsetList);

        void setFileName(std::string name){
            fileName = name;
        }

        std::string getFileName(){
            return fileName;
        }
};