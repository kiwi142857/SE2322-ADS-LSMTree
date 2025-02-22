#include "bloomFilter.h"

void gen_bloom_filter(std::vector<uint64_t> &nums, std::vector<bool> &bloom_filter, int num_hashes, int num_bits) {
    for (int i = 0; i < nums.size(); i++) {
        for (int j = 0; j < num_hashes; j++) {
            uint64_t hash[2];
            MurmurHash3_x64_128(&nums[i], sizeof(uint64_t), j, hash);
            bloom_filter[hash[0] % num_bits] = true;
        }
    }
}

void gen_bloom_filter(std::list<std::pair<uint64_t, std::string> > &nums, std::vector<bool> &bloom_filter, int num_hashes , int num_bits ) {
    for (auto &num : nums) {
        for (int j = 0; j < num_hashes; j++) {
            uint64_t hash[2];
            MurmurHash3_x64_128(&num.first, sizeof(uint64_t), j, hash);
            bloom_filter[hash[0] % num_bits] = true;
        }
    }
}

void gen_bloom_filter(std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> &nums, std::vector<bool> &bloom_filter, int num_hashes, int num_bits) {
    for (auto &num : nums) {
        for (int j = 0; j < num_hashes; j++) {
            uint64_t hash[2];
            MurmurHash3_x64_128(&std::get<0>(num), sizeof(uint64_t), j, hash);
            bloom_filter[hash[0] % num_bits] = true;
        }
    }
}

bool check_bloom_filter(std::vector<uint64_t> &nums, std::vector<bool> &bloom_filter, int num_hashes) {
    for (int i = 0; i < nums.size(); i++) {
        bool found = true;
        for (int j = 0; j < num_hashes; j++) {
            uint64_t hash[2];
            MurmurHash3_x64_128(&nums[i], sizeof(uint64_t), j, hash);
            if (!bloom_filter[hash[0] % bloom_filter.size()]) {
                found = false;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

bool check_bloom_filter(uint64_t num, std::vector<bool> &bloom_filter, int num_hashes) {
    bool found = true;
    for (int j = 0; j < num_hashes; j++) {
        uint64_t hash[2];
        MurmurHash3_x64_128(&num, sizeof(uint64_t), j, hash);
        if (!bloom_filter[hash[0] % bloom_filter.size()]) {
            found = false;
            break;
        }
    }
    return found;
}