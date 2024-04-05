#pragma once

#include <cstdint>
#include <vector>
#include <tuple>

class Sstable {

    uint64_t timeId;
    uint64_t pairNum;
    uint64_t largestKey;
    uint64_t smallestKey;
    std::vector<bool> bloomFilter;
    std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable;
    uint32_t level;
    uint32_t order;

    public:
        Sstable(uint64_t timeId, uint64_t pairNum, uint64_t largestKey, uint64_t smallestKey, std::vector<bool> bloomFilter, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable);
        Sstable();
        uint64_t getTimeId();
        uint64_t getPairNum();
        uint64_t getLargestKey();
        uint64_t getSmallestKey();
        std::vector<bool> getBloomFilter();
        std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> getKeyOffsetTable();
        bool checkBloomFilter(uint64_t num, int num_hashes = 4);
        std::tuple<uint64_t, uint32_t> getOffset(uint64_t key);
        void output(std::string filename);
};