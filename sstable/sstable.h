#pragma once

#include <cstdint>
#include <vector>
#include <tuple>

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

    public:
        SSTable(uint64_t timeId, uint64_t pairNum, uint64_t largestKey, uint64_t smallestKey, std::vector<bool> bloomFilter, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable);
        SSTable();

        // get / set functions
        uint64_t getTimeId();
        uint64_t getPairNum();
        uint64_t getLargestKey();
        uint64_t getSmallestKey();
        std::vector<bool> getBloomFilter();
        std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> getItem();

        // check if the key is in the bloom filter
        bool checkBloomFilter(uint64_t num, int num_hashes = 4);

        // 通过key获取offset
        std::tuple<uint64_t, uint64_t, uint32_t> getOffset(uint64_t key);

        // output the sstable to a file
        void output(std::fstream &file);

};