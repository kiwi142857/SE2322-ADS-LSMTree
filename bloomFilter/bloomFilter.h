#pragma once

#include "../MurmurHash3.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <list>
#include <stdint.h>
#include <tuple>

#define kb 1024
void gen_bloom_filter(std::vector<uint64_t> &nums, std::vector<bool> &bloom_filter, int num_hashes = 4, int num_bits = 64 * kb);
void gen_bloom_filter(std::list<std::pair<uint64_t, std::string> > &nums, std::vector<bool> &bloom_filter, int num_hashes = 4, int num_bits = 64 * kb);
void gen_bloom_filter(std::vector<std::tuple<uint64_t, uint64_t, uint32_t>> &nums, std::vector<bool> &bloom_filter, int num_hashes = 4, int num_bits = 64 * kb);
bool check_bloom_filter(std::vector<uint64_t> &nums, std::vector<bool> &bloom_filter, int num_hashes = 4);
bool check_bloom_filter(uint64_t num, std::vector<bool> &bloom_filter, int num_hashes = 4);