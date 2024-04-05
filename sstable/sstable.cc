#include "sstable.h"
#include "../bloomFilter/bloomFilter.h"
#include <iostream>
#include <fstream>

Sstable::Sstable(uint64_t timeId, uint64_t pairNum, uint64_t largestKey, uint64_t smallestKey, std::vector<bool> bloomFilter, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> keyOffsetTable)
: timeId(timeId), pairNum(pairNum), largestKey(largestKey), smallestKey(smallestKey), bloomFilter(bloomFilter), keyOffsetTable(keyOffsetTable) {
    level = 0;
    order = 0;
}

Sstable::Sstable() {
    level = 0;
    order = 0;
}

uint64_t Sstable::getTimeId() {
    return timeId;
}

uint64_t Sstable::getPairNum() {
    return pairNum;
}

uint64_t Sstable::getLargestKey() {
    return largestKey;
}

uint64_t Sstable::getSmallestKey() {
    return smallestKey;
}

std::vector<bool> Sstable::getBloomFilter() {
    return bloomFilter;
}

std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> Sstable::getKeyOffsetTable() {
    return keyOffsetTable;
}

bool Sstable::checkBloomFilter(uint64_t num, int num_hashes) {
    return check_bloom_filter(num, bloomFilter, num_hashes);
}

std::tuple<uint64_t, uint32_t> Sstable::getOffset(uint64_t key) {
    for (int i = 0; i < keyOffsetTable.size(); i++) {
        if (std::get<0>(keyOffsetTable[i]) == key) {
            return std::make_tuple(std::get<1>(keyOffsetTable[i]), std::get<2>(keyOffsetTable[i]));
        }
    }
    return std::make_tuple(0, 0);
}

void Sstable::output(std::string filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write((char *)&timeId, sizeof(timeId));
    out.write((char *)&pairNum, sizeof(pairNum));
    out.write((char *)&largestKey, sizeof(largestKey));
    out.write((char *)&smallestKey, sizeof(smallestKey));
    for (int i = 0; i < bloomFilter.size(); i++) {
        auto temp = bloomFilter[i];
        out.write((char *)&temp, sizeof(temp));
    }
    for (int i = 0; i < keyOffsetTable.size(); i++) {
        out.write((char *)&std::get<0>(keyOffsetTable[i]), sizeof(std::get<0>(keyOffsetTable[i])));
        out.write((char *)&std::get<1>(keyOffsetTable[i]), sizeof(std::get<1>(keyOffsetTable[i])));
        out.write((char *)&std::get<2>(keyOffsetTable[i]), sizeof(std::get<2>(keyOffsetTable[i])));
    }
    out.close();
}
// Path: sstable/sstable_test.cc
