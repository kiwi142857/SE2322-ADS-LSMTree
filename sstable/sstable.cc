#include "sstable.h"
#include "../bloomFilter/bloomFilter.h"
#include <iostream>
#include <fstream>

SSTable::SSTable(uint64_t timeId, uint64_t pairNum, uint64_t largestKey, uint64_t smallestKey, std::vector<bool> bloomFilter, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> item)
: timeId(timeId), pairNum(pairNum), largestKey(largestKey), smallestKey(smallestKey), bloomFilter(bloomFilter), item(item) {
    level = 0;
}

SSTable::SSTable() {
    level = 0;
    
}

uint64_t SSTable::getTimeId() {
    return timeId;
}

uint64_t SSTable::getPairNum() {
    return pairNum;
}

uint64_t SSTable::getLargestKey() {
    return largestKey;
}

uint64_t SSTable::getSmallestKey() {
    return smallestKey;
}

std::vector<bool> SSTable::getBloomFilter() {
    return bloomFilter;
}

std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> SSTable::getItem() {
    return item;
}

bool SSTable::checkBloomFilter(uint64_t num, int num_hashes) {
    return check_bloom_filter(num, bloomFilter, num_hashes);
}

// TODO：修改为二分查找
std::tuple<uint64_t, uint64_t, uint32_t> SSTable::getOffset(uint64_t key) {
    for (size_t i = 0; i < item.size(); i++) {
        if (std::get<0>(item[i]) == key) {
            return item[i];
        }
    }
    return std::make_tuple(0, 0, 0);
}

void SSTable::output(std::fstream &out) {

    // 写入Header
    out.write((char *)&timeId, sizeof(timeId));
    out.write((char *)&pairNum, sizeof(pairNum));
    out.write((char *)&largestKey, sizeof(largestKey));
    out.write((char *)&smallestKey, sizeof(smallestKey));

    // 写入BloomFilter
    for (int i = 0; i < bloomFilter.size(); i++) {
        auto temp = bloomFilter[i];
        out.write((char *)&temp, sizeof(temp));
    }

    // 写入item
    for (int i = 0; i < item.size(); i++) {
        out.write((char *)&std::get<0>(item[i]), sizeof(std::get<0>(item[i])));
        out.write((char *)&std::get<1>(item[i]), sizeof(std::get<1>(item[i])));
        out.write((char *)&std::get<2>(item[i]), sizeof(std::get<2>(item[i])));
    }
    out.close();
}
// Path: sstable/sstable_test.cc
