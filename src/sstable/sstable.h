#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include "../utils/bloomFilter.h"

struct IndexEntry {
    uint64_t key;
    uint32_t offset;
    IndexEntry(uint64_t k, uint32_t off) : key(k), offset(off) {}
};

class SSTable {
private:
    std::string filename;
    BloomFilter bloomFilter;
    std::vector<IndexEntry> index;
    std::fstream file;
    uint64_t timestamp;
    uint32_t dataOffset;

public:
    SSTable(const std::string &filename, uint64_t timestamp);
    ~SSTable();

    void writeEntry(uint64_t key, const std::string &value, uint32_t &offset);
    std::string readValue(uint32_t offset, uint32_t length) const;
    bool mayContain(uint64_t key) const;
    std::string get(uint64_t key) const;
    uint64_t getTimestamp() const { return timestamp; }
    const std::vector<IndexEntry>& getIndex() const { return index; }
};