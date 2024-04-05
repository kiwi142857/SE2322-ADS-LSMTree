#pragma once

#include "../MurmurHash3.h"
#include <iostream>
#include <vector>
#include <fstream>

void gen_bloom_filter(std::vector<uint64_t> &nums, std::vector<bool> &bloom_filter, int num_hashes = 4, int num_bits = 8192);
bool check_bloom_filter(std::vector<uint64_t> &nums, std::vector<bool> &bloom_filter, int num_hashes = 4);
bool check_bloom_filter(uint64_t num, std::vector<bool> &bloom_filter, int num_hashes = 4);