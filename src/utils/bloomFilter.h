#pragma once

#include <vector>
#include <cstdint>
#include "MurmurHash3.h"

class BloomFilter {
private:
    std::vector<bool> bits;
    size_t size;
    size_t numHashes;

    size_t hash(uint64_t key, size_t seed) const;

public:
    BloomFilter(size_t size = 10000, size_t numHashes = 4);
    
    void add(uint64_t key);
    bool mayContain(uint64_t key) const;
    void clear();
    
    const std::vector<bool>& getBits() const { return bits; }
    size_t getSize() const { return size; }
    size_t getNumHashes() const { return numHashes; }
};