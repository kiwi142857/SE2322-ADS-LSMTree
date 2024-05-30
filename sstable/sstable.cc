#include "sstable.h"
#include "../bloomFilter/bloomFilter.h"
#include <iostream>
#include <fstream>
#include <sstream>

SSTable::SSTable(uint64_t timeId, uint64_t pairNum, uint64_t largestKey, uint64_t smallestKey, std::vector<bool> bloomFilter, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> item)
: timeId(timeId), pairNum(pairNum), largestKey(largestKey), smallestKey(smallestKey), bloomFilter(bloomFilter), item(item) {
    level = 0;
}

SSTable::SSTable() {
    level = 0;
    
}

uint64_t SSTable::getTimeId() const {
    return timeId;
}

uint64_t SSTable::getPairNum() const{
    return pairNum;
}

uint64_t SSTable::getLargestKey() const{
    return largestKey;
}

uint64_t SSTable::getSmallestKey() const{
    return smallestKey;
}

std::vector<bool> SSTable::getBloomFilter() const{
    return bloomFilter;
}

std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>& SSTable::getItem() {
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
    return std::make_tuple(INT64_MAX, 0, 0);
}

void SSTable::output(std::fstream &out) {
    std::stringstream ss;

    // 写入Header
    ss.write((char *)&timeId, sizeof(timeId));
    ss.write((char *)&pairNum, sizeof(pairNum));
    ss.write((char *)&largestKey, sizeof(largestKey));
    ss.write((char *)&smallestKey, sizeof(smallestKey));

    // 写入BloomFilter, 8个bool占用一个字节
    for (int i = 0; i < bloomFilter.size(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8; j++) {
            if (i + j < bloomFilter.size()) {
                byte |= (bloomFilter[i + j] << j);
            }
        }
        ss.write(&byte, sizeof(byte));
    }

    // 写入item
    for (int i = 0; i < item.size(); i++) {
        ss.write((char *)&std::get<0>(item[i]), sizeof(std::get<0>(item[i])));
        ss.write((char *)&std::get<1>(item[i]), sizeof(std::get<1>(item[i])));
        ss.write((char *)&std::get<2>(item[i]), sizeof(std::get<2>(item[i])));
    }

    // 将字符串流的内容写入到文件中
    out << ss.rdbuf();
}

void SSTable::scanOffset(uint64_t start, uint64_t end, std::list<std::tuple<uint64_t, uint64_t, uint32_t>> &offsetList) {
    if(start > largestKey || end < smallestKey) {
        return;
    }
    // since the item is sorted, we can use binary search to find the start and end
    int left = 0;
    int right = item.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (std::get<0>(item[mid]) < start) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    int startIdx = left;

    left = 0;
    right = item.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (std::get<0>(item[mid]) > end) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    int endIdx = right;

    for (int i = startIdx; i <= endIdx; i++) {
        offsetList.push_back(item[i]);
    }
}
// Path: sstable/sstable_test.cc
